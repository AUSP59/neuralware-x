// SPDX-License-Identifier: Apache-2.0
#include "nwx/bundle.hpp"
#include "nwx/version.hpp"
#include "nwx/cpu_features.hpp"
#include "nwx/quant_runtime.hpp"
#include "nwx/serialize.hpp"
#include "nwx/tensor.hpp"
#include <string>
#include <vector>
#include <thread>
#include <sstream>
#include <random>
#include <cstring>
#include <iostream>
#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <ctime>
#include <map>
#include <queue>
#include <future>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <fstream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#ifdef NWX_HAVE_OPENSSL
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

using namespace nwx;
using clock_t = std::chrono::steady_clock;
#ifdef NWX_HAVE_ZLIB
#include <zlib.h>
static bool inflate_gzip(const std::string& in, std::string& out){
  z_stream zs{}; if (inflateInit2(&zs, 16+MAX_WBITS)!=Z_OK) return false;
  zs.next_in = (Bytef*)in.data(); zs.avail_in = (uInt)in.size();
  char buf[1<<14]; int ret=Z_OK;
  while (ret==Z_OK){
    zs.next_out = (Bytef*)buf; zs.avail_out = sizeof(buf);
    ret = inflate(&zs, Z_NO_FLUSH);
    if (ret==Z_OK || ret==Z_STREAM_END){ out.append(buf, sizeof(buf)-zs.avail_out); }
  }
  inflateEnd(&zs);
  return ret==Z_STREAM_END;
}
#endif
// Rate limiting state
static std::mutex rl_mtx;
static std::unordered_map<std::string, std::pair<long long,long long>> rl_counts; // token -> (count, reset_epoch)
static long long RL_LIMIT = (std::getenv("NWX_RATELIMIT")? atoll(std::getenv("NWX_RATELIMIT")) : 0);
static long long RL_WINDOW = 60;
static void parse_rl_env(){ const char* e = std::getenv("NWX_RATELIMIT"); if(!e) return; std::string v(e); auto c=v.find(','); if(c!=std::string::npos){ RL_LIMIT = atoll(v.substr(0,c).c_str()); RL_WINDOW = atoll(v.substr(c+1).c_str()); } }

static std::atomic<bool> SHUTDOWN(false);
static void nwx_handle_sig(int){ SHUTDOWN.store(true); }


static void otlp_export_metrics_http(long long total_requests){
  const char* url = std::getenv("NWX_OTEL_METRICS_URL");
  if (!url) return;
  std::string u(url); if (u.rfind("http://",0) != 0) return;
  std::string hostport_path = u.substr(7); std::string hostport, path;
  auto slash = hostport_path.find('/'); if (slash==std::string::npos){ hostport=hostport_path; path="/v1/metrics"; } else { hostport=hostport_path.substr(0,slash); path=hostport_path.substr(slash); }
  std::string host = hostport; int port = 80; auto colon = hostport.find(':'); if (colon!=std::string::npos){ host = hostport.substr(0,colon); port = std::atoi(hostport.substr(colon+1).c_str()); }
#ifdef _WIN32
  WSADATA wsaData; WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
  struct addrinfo hints{}; hints.ai_family=AF_UNSPEC; hints.ai_socktype=SOCK_STREAM;
  struct addrinfo* res=nullptr; char portstr[16]; snprintf(portstr,sizeof(portstr), "%d", port);
  if (getaddrinfo(host.c_str(), portstr, &hints, &res)!=0 || !res) return;
  int fd = (int)socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (fd<0){ freeaddrinfo(res); return; }
  if (connect(fd, res->ai_addr, (int)res->ai_addrlen)!=0){ freeaddrinfo(res); return; }
  freeaddrinfo(res);
  // Very small OTLP/HTTP JSON for sum (counter)
  std::ostringstream body;
  body << "{\"resourceMetrics\":[{\"resource\":{\"attributes\":[{\"key\":\"service.name\",\"value\":{\"stringValue\":\"neuralwarex\"}}]},\"scopeMetrics\":[{\"metrics\":[{\"name\":\"nwx.requests.total\",\"sum\":{\"isMonotonic\":true,\"aggregationTemporality\":2,\"dataPoints\":[{\"asDouble\":"<< total_requests <<"}]}}]}]}]}";
  std::string b = body.str();
  std::ostringstream req;
  req << "POST " << path << " HTTP/1.1\r\nHost: " << host << "\r\nContent-Type: application/json\r\nContent-Length: " << b.size() << "\r\nConnection: close\r\n\r\n" << b;
  std::string r = req.str(); send(fd, r.c_str(), (int)r.size(), 0);
#ifdef _WIN32
  closesocket(fd); WSACleanup();
#else
  close(fd);
#endif
}

struct BatchItem {
  nwx::Tensor2D X;
  std::promise<nwx::Tensor2D> prom;
};
static std::mutex batch_mtx;
static std::condition_variable batch_cv;
static std::queue<std::shared_ptr<BatchItem>> batch_q;
static bool batching_stop=false;
static int BATCH_SIZE = std::getenv("NWX_BATCH_SIZE")? std::max(0, std::atoi(std::getenv("NWX_BATCH_SIZE"))):0;
static int BATCH_TIMEOUT_MS = std::getenv("NWX_BATCH_TIMEOUT_MS")? std::max(1, std::atoi(std::getenv("NWX_BATCH_TIMEOUT_MS"))):5;
static int NWX_THREADS = std::getenv("NWX_THREADS")? std::max(1, std::atoi(std::getenv("NWX_THREADS"))):1;
static bool MODEL_WARM = false;


static void otlp_export_http(const std::string& kind, const std::string& name, long long dur_ms, const std::string& trace_id, const std::string& span_id){
  const char* url = std::getenv("NWX_OTEL_EXPORTER_URL");
  if (!url) return;
  std::string u(url);
  if (u.rfind("http://",0) != 0) return; // only http for simplicity
  // parse http://host:port[/path]
  std::string hostport_path = u.substr(7);
  std::string hostport, path;
  auto slash = hostport_path.find('/');
  if (slash==std::string::npos){ hostport = hostport_path; path = "/v1/traces"; } else { hostport = hostport_path.substr(0,slash); path = hostport_path.substr(slash); }
  std::string host = hostport; int port = 80;
  auto colon = hostport.find(':');
  if (colon!=std::string::npos){ host = hostport.substr(0,colon); port = std::atoi(hostport.substr(colon+1).c_str()); }
#ifdef _WIN32
  WSADATA wsaData; WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
  struct addrinfo hints{}; hints.ai_family=AF_UNSPEC; hints.ai_socktype=SOCK_STREAM;
  struct addrinfo* res=nullptr; char portstr[16]; snprintf(portstr,sizeof(portstr), "%d", port);
  if (getaddrinfo(host.c_str(), portstr, &hints, &res)!=0 || !res) return;
  int fd = (int)socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (fd<0){ freeaddrinfo(res); return; }
  if (connect(fd, res->ai_addr, (int)res->ai_addrlen)!=0){ freeaddrinfo(res); return; }
  freeaddrinfo(res);
  // very small OTLP/HTTP JSON body
  std::ostringstream body;
  body << "{\"resourceSpans\":[{\"resource\":{\"attributes\":[{\"key\":\"service.name\",\"value\":{\"stringValue\":\"neuralwarex\"}}]},"
       << "\"scopeSpans\":[{\"spans\":[{\"name\":\""<< name <<"\",\"kind\":1,"
       << "\"traceId\":\""<< trace_id <<"\",\"spanId\":\""<< span_id <<"\","
       << "\"endTimeUnixNano\":0,\"startTimeUnixNano\":0,"
       << "\"attributes\":[{\"key\":\"http.route\",\"value\":{\"stringValue\":\""<< name <<"\"}},{\"key\":\"nwx.kind\",\"value\":{\"stringValue\":\""<< kind <<"\"}},{\"key\":\"duration.ms\",\"value\":{\"doubleValue\":"<< dur_ms << "}}]}]}]}]}";
  std::string b = body.str();
  std::ostringstream req;
  req << "POST " << path << " HTTP/1.1\r\nHost: " << host << "\r\nContent-Type: application/json\r\nContent-Length: " << b.size() << "\r\nConnection: close\r\n\r\n" << b;
  std::string r = req.str();
  send(fd, r.c_str(), (int)r.size(), 0);
#ifdef _WIN32
  closesocket(fd); WSACleanup();
#else
  close(fd);
#endif
}

static std::string gen_request_id(){ std::mt19937_64 rng{std::random_device{}()}; std::ostringstream o; for(int i=0;i<4;++i){ auto v=rng(); o<<std::hex<<std::nouppercase; o.width(16); o.fill('0'); o<<v; } return o.str(); }
static std::string weak_etag_from(const std::string& s){ uint64_t h=1469598103934665603ull; for(unsigned char c: s){ h^=c; h*=1099511628211ull; } std::ostringstream o; o << "W/\"" << std::hex << h << "-" << s.size() << "\""; return o.str(); }
static bool verify_jwt_claims(const std::string& jwt){
  const char* iss = std::getenv("NWX_JWT_ISS");
  const char* aud = std::getenv("NWX_JWT_AUD");
  long leeway = 0; if (const char* l = std::getenv("NWX_JWT_LEEWAY_S")) leeway = std::strtol(l,nullptr,10);
  auto dot1 = jwt.find('.'); if (dot1==std::string::npos) return false;
  auto dot2 = jwt.find('.', dot1+1); if (dot2==std::string::npos) return false;
  std::string payload_b64 = jwt.substr(dot1+1, dot2-dot1-1);
  auto decode = [](const std::string& b)->std::string{
    static const std::string t="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string in=b; for(char& c:in){ if(c=='-') c='+'; else if(c=='_') c='/'; }
    auto problem_json = [&](int code, const std::string& title, const std::string& detail){ std::ostringstream pj; pj << "{\"type\":\"about:blank\",\"title\":\"" << title << "\",\"status\":"<<code<<",\"detail\":\"" << detail << "\"}"; respond(code, pj.str(), "application/problem+json"); }
    while (in.size()%4!=0) in.push_back('=');
    std::string out; out.reserve(in.size()*3/4);
    int val=0, valb=-8; for(unsigned char c: in){ if(c=='=') break; int idx=t.find(c); if(idx==std::string::npos) continue; val=(val<<6)+idx; valb+=6; if(valb>=0){ out.push_back(char((val>>valb)&0xFF)); valb-=8; } }
    return out;
  };
  std::string p = decode(payload_b64);
  auto has_kv = [&](const std::string& k, const std::string& v){ auto pos = p.find("\""+k+"\""); if(pos==std::string::npos) return false; auto c = p.find(':', pos); if(c==std::string::npos) return false; auto q = p.find('"', c+1); if(q==std::string::npos) return false; auto r = p.find('"', q+1); if(r==std::string::npos) return false; return p.substr(q+1, r-q-1) == v; };
  if (iss && !has_kv("iss", iss)) return false;
  if (aud && !has_kv("aud", aud)) return false;
  auto find_num = [&](const std::string& k)->long long { auto pos=p.find("\""+k+"\""); if(pos==std::string::npos) return 0; auto c=p.find(':',pos); if(c==std::string::npos) return 0; long long v=0; size_t i=c+1; while(i<p.size() && (p[i]==' '||p[i]=='\t')) i++; bool neg=false; if(i<p.size() && p[i]=='-'){neg=true;i++;} while(i<p.size() && isdigit((unsigned char)p[i])){ v=v*10 + (p[i]-'0'); i++; } return neg? -v: v; };
  long long now = (long long)time(nullptr);
  long long exp = find_num("exp"); if (exp && now - leeway > exp) return false;
  long long nbf = find_num("nbf"); if (nbf && now + leeway < nbf) return false;
  return true;
}

#ifdef NWX_HAVE_OPENSSL
static std::string b64url_pad(std::string s){ while(s.size()%4!=0) s.push_back('='); return s; }
static bool verify_jwt_hs256(const std::string& jwt, const std::string& secret){
  auto dot1 = jwt.find('.'); if (dot1==std::string::npos) return false;
  auto dot2 = jwt.find('.', dot1+1); if (dot2==std::string::npos) return false;
  std::string header = jwt.substr(0, dot1);
  std::string payload = jwt.substr(dot1+1, dot2-dot1-1);
  std::string sig_b64 = jwt.substr(dot2+1);
  auto b64url = [](const std::string& in)->std::string{
    std::string s=in; for(char& c:s){ if(c=='+') c='-'; else if(c=='/') c='_'; }
#ifdef NWX_HAVE_OPENSSL

#ifdef NWX_HAVE_OPENSSL
static std::string b64url_to_bin(const std::string& in){
  std::string s=in; for(char& c: s){ if(c=='-') c='+'; else if(c=='_') c='/'; }
  while (s.size()%4!=0) s.push_back('=');
  static const std::string t="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out; out.reserve(s.size()*3/4);
  int val=0, valb=-8;
  for (unsigned char c: s){
    if (c=='=') break; int idx = t.find(c); if(idx==(int)std::string::npos) continue;
    val = (val<<6) + idx; valb += 6; if (valb>=0){ out.push_back(char((val>>valb)&0xFF)); valb-=8; }
  }
  return out;
}
// very small JWKS parser: expects {"keys":[{"kid":"...","kty":"RSA","n":"...","e":"..."}...]}
static bool jwks_find_key(const std::string& jwks_json, const std::string& kid, std::string& n_b64, std::string& e_b64){
  auto pos = jwks_json.find("\"kid\":\""+kid+"\""); if (pos==std::string::npos) return false;
  auto npos = jwks_json.find("\"n\":\"", pos); if (npos==std::string::npos) return false; npos += 5; auto nend = jwks_json.find("\"", npos); n_b64 = jwks_json.substr(npos, nend-npos);
  auto epos = jwks_json.find("\"e\":\"", pos); if (epos==std::string::npos) return false; epos += 5; auto eend = jwks_json.find("\"", epos); e_b64 = jwks_json.substr(epos, eend-epos);
  return true;
}
static bool verify_jwt_rs256_jwks(const std::string& jwt, const std::string& jwks_json){
  auto dot1 = jwt.find('.'); if (dot1==std::string::npos) return false;
  auto dot2 = jwt.find('.', dot1+1); if (dot2==std::string::npos) return false;
  std::string header_b64 = jwt.substr(0, dot1);
  std::string payload = jwt.substr(dot1+1, dot2-dot1-1);
  std::string sig_b64 = jwt.substr(dot2+1);
  // extract kid from header
  auto decode_json = [](const std::string& b){ return b64url_to_bin(b); };
  std::string hdr = decode_json(header_b64);
  auto kpos = hdr.find("\"kid\":\""); if (kpos==std::string::npos) return false; kpos+=7; auto kend = hdr.find("\"", kpos); std::string kid = hdr.substr(kpos, kend-kpos);
  std::string n_b64,e_b64; if(!jwks_find_key(jwks_json, kid, n_b64, e_b64)) return false;
  std::string n_bin = b64url_to_bin(n_b64), e_bin = b64url_to_bin(e_b64);
  const unsigned char* n_ptr = (const unsigned char*)n_bin.data();
  const unsigned char* e_ptr = (const unsigned char*)e_bin.data();
  BIGNUM* n = BN_bin2bn(n_ptr, (int)n_bin.size(), nullptr);
  BIGNUM* e = BN_bin2bn(e_ptr, (int)e_bin.size(), nullptr);
  RSA* rsa = RSA_new(); RSA_set0_key(rsa, n, e, nullptr);
  EVP_PKEY* pkey = EVP_PKEY_new(); EVP_PKEY_assign_RSA(pkey, rsa);
  std::string data = header_b64 + "." + payload;
  std::string sig = b64url_to_bin(sig_b64);
  EVP_MD_CTX* ctx = EVP_MD_CTX_new(); bool ok=false;
  if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) == 1 &&
      EVP_DigestVerifyUpdate(ctx, data.data(), data.size()) == 1 &&
      EVP_DigestVerifyFinal(ctx, (const unsigned char*)sig.data(), (size_t)sig.size()) == 1) ok = true;
  EVP_MD_CTX_free(ctx); EVP_PKEY_free(pkey); // frees rsa too
  return ok;
}
#endif

static bool verify_jwt_rs256(const std::string& jwt, const std::string& pub_pem){
  auto dot1 = jwt.find('.'); if (dot1==std::string::npos) return false;
  auto dot2 = jwt.find('.', dot1+1); if (dot2==std::string::npos) return false;
  std::string header = jwt.substr(0, dot1);
  std::string payload = jwt.substr(dot1+1, dot2-dot1-1);
  std::string sig_b64 = jwt.substr(dot2+1);
  auto decode = [](const std::string& b)->std::string{
    static const std::string t="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string in=b; std::string out; out.reserve(in.size()*3/4);
    for(char& c: in){ if(c=='-') c='+'; else if(c=='_') c='/'; }
    int val=0, valb=-8;
    for (unsigned char c : in){
      if (c=='=') break;
      int idx = t.find(c);
      if (idx==std::string::npos) continue;
      val = (val<<6) + idx; valb += 6;
      if (valb>=0){ out.push_back(char((val>>valb)&0xFF)); valb-=8; }
    }
    return out;
  };
  std::string data = header + "." + payload;
  std::string sig = decode(sig_b64);
  BIO* bio = BIO_new_mem_buf(pub_pem.data(), (int)pub_pem.size());
  EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
  BIO_free(bio);
  if (!pkey) return false;
  EVP_MD_CTX* ctx = EVP_MD_CTX_new();
  bool ok = false;
  if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) == 1 &&
      EVP_DigestVerifyUpdate(ctx, data.data(), data.size()) == 1 &&
      EVP_DigestVerifyFinal(ctx, (const unsigned char*)sig.data(), sig.size()) == 1) ok = true;
  EVP_MD_CTX_free(ctx);
  EVP_PKEY_free(pkey);
  return ok;
}
#endif
 while(s.size()%4!=0)s.push_back('=');
    return s;
  };
  auto decode = [](const std::string& b)->std::string{
    static const std::string t="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string in=b; for(char& c:in){ if(c=='-') c='+'; else if(c=='_') c='/'; }
    std::string out; out.reserve(in.size()*3/4);
    int val=0, valb=-8;
    for (unsigned char c : in){
      if (c=='=') break;
      int idx = t.find(c);
      if (idx==std::string::npos) continue;
      val = (val<<6) + idx; valb += 6;
      if (valb>=0){ out.push_back(char((val>>valb)&0xFF)); valb-=8; }
    }
    return out;
  };
  std::string data = header + "." + payload;
  unsigned int len=0;
  unsigned char mac[EVP_MAX_MD_SIZE];
  HMAC(EVP_sha256(), secret.data(), (int)secret.size(), (const unsigned char*)data.data(), (int)data.size(), mac, &len);
  std::string sig = decode(sig_b64);
  if (sig.size()!=len) return false;
  return CRYPTO_memcmp(mac, (const unsigned char*)sig.data(), len)==0;
}
#endif

static bool LOG_JSON = false;
static std::string CORS_ALLOW = std::getenv("NWX_CORS_ALLOW")? std::string(std::getenv("NWX_CORS_ALLOW")) : std::string("");
static void logj(const char* level, const std::string& msg){
  if(LOG_JSON){ std::cerr << "{\"level\":\""<<level<<"\",\"msg\":\""<<msg<<"\"}\n"; }
  else { std::cerr << "["<<level<<"] "<<msg<<"\n"; }
}

static std::string http_response(int code, const std::string& body, const std::string& ctype="application/json") {
  std::ostringstream oss;
  std::string msg = (code==200)?"OK":(code==401)?"Unauthorized":(code==429)?"Too Many Requests":"Error";
  oss << "HTTP/1.1 " << code << " " << msg << "\r\n"
      << "Content-Type: " << ctype << "\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Connection: close\r\n\r\n"
      << body;
  return oss.str();
}

static std::atomic<bool> g_stop{false}; static std::atomic<bool> g_reload{false};
static void handle_term(int){ g_stop.store(true); }
static void handle_hup(int){ g_reload.store(true); }

struct TokenBucket {
  double tokens{0.0};
  long long last_ms{0};
};

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "Usage: nwx_serve <model.bundle|model.bin> <port> [--token SECRET] [--rate RPS] [--burst N] [--tls_cert cert.pem --tls_key key.pem] [--max_body 1048576] [--rcv_timeout_ms 5000]\n";
    return 2;
  }
  std::string model_path = argv[1];
  int port = std::stoi(argv[2]);
  std::string token=""; int rate=50; int burst=100; size_t max_body=1048576; int rcv_timeout_ms=5000; int max_conc=64; bool log_json=false;
  std::string tls_cert, tls_key; std::string jwt_secret=""; std::string jwt_rs256_pub=""; std::string jwt_jwks=""; bool require_mtls=false; std::string client_ca="";
  for (int i=3;i<argc;++i){
    std::string a=argv[i];
    if(a=="--token" && i+1<argc) token=argv[++i];
    else if(a=="--rate" && i+1<argc) rate=std::stoi(argv[++i]);
    else if(a=="--burst" && i+1<argc) burst=std::stoi(argv[++i]);
    else if(a=="--max_body" && i+1<argc) max_body=(size_t)std::stoll(argv[++i]);
    else if(a=="--rcv_timeout_ms" && i+1<argc) rcv_timeout_ms=std::stoi(argv[++i]);
    else if(a=="--max_concurrency" && i+1<argc) max_conc=std::stoi(argv[++i]);
    else if(a=="--log_json") log_json=true;
    else if(a=="--tls_cert" && i+1<argc) tls_cert=argv[++i];
    else if(a=="--tls_key" && i+1<argc) tls_key=argv[++i];
    else if(a=="--jwt_hs256" && i+1<argc) jwt_secret=argv[++i];
    else if(a=="--jwt_rs256_pub" && i+1<argc) jwt_rs256_pub=argv[++i];
    else if(a=="--jwt_jwks" && i+1<argc) jwt_jwks=argv[++i];
    else if(a=="--client_ca" && i+1<argc) client_ca=argv[++i];
  }
  if(token.empty()){ const char* t=getenv("NWX_TOKEN"); if(t) token=t; } const char* lj=getenv("NWX_LOG_JSON"); if(lj&&std::string(lj)=="1") log_json=true; LOG_JSON=log_json; const char* js=getenv("NWX_JWT_HS256"); if(js) jwt_secret=js; const char* mt=getenv("NWX_REQUIRE_CLIENT_CERT"); if(mt && std::string(mt)=="1") require_mtls=true;

  ModelBundle b; std::mutex bmtx; std::string model_on_disk = model_path; bool decrypted_temp=false; bool quantize_env = (std::getenv("NWX_QUANTIZE") && std::string(std::getenv("NWX_QUANTIZE"))=="1");
  
std::string path_to_load = model_path;
const char* k = std::getenv("NWX_BUNDLE_KEY");
if (k && model_path.size()>4 && model_path.substr(model_path.size()-4)==".enc"){
#ifdef NWX_HAVE_OPENSSL
  std::ifstream fi(model_path, std::ios::binary); std::vector<unsigned char> buf((std::istreambuf_iterator<char>(fi)), {});
  if (buf.size()>=28){
    std::vector<unsigned char> iv(buf.begin(), buf.begin()+12);
    std::vector<unsigned char> tag(buf.end()-16, buf.end());
    std::vector<unsigned char> ct(buf.begin()+12, buf.end()-16);
    std::vector<unsigned char> out(ct.size());
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    std::vector<unsigned char> key(32,0);
    for (int i=0;i<32;i++){ unsigned v=0; sscanf(k+i*2, "%2x", &v); key[i]=(unsigned char)v; }
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data());
    int len=0, olen=0; EVP_DecryptUpdate(ctx, out.data(), &len, ct.data(), (int)ct.size()); olen=len;
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag.data());
    int ok = EVP_DecryptFinal_ex(ctx, out.data()+olen, &len); EVP_CIPHER_CTX_free(ctx);
    if (ok==1){ std::string tmp="/tmp/nwx_model.bundle"; std::ofstream fo(tmp, std::ios::binary); fo.write((char*)out.data(), olen); fo.close(); path_to_load=tmp; decrypted_temp=true; }
  }
#endif
}
if (!load_bundle(b, path_to_load)) {
    if (!load_model(b.model, model_path)) {
      std::cerr << "Failed to load model/bundle: " << model_path << "\n";
      return 3;
    }
    b.has_scaler = false;
    b.model.temperature = 1.0;
  // Warmup: single forward pass to prime caches
  try { nwx::Tensor2D Wx(1, b.model.W1.cols); for(int j=0;j<b.model.W1.cols;++j) Wx(0,j)=0.0; (void)nwx::softmax(b.model.forward(Wx)); MODEL_WARM = true; } catch(...) { MODEL_WARM = false; }
  }

#ifdef _WIN32
  WSADATA wsaData; WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

#ifdef NWX_HAVE_OPENSSL
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
  SSL_CTX* ssl_ctx = nullptr;
  bool use_tls = (!tls_cert.empty() && !tls_key.empty());
  if (use_tls) {
    if (!client_ca.empty()) { if (!SSL_CTX_load_verify_locations(ssl_ctx, client_ca.c_str(), nullptr)) { std::cerr<<"OpenSSL: failed to load client CA\n"; return 4; } require_mtls=true; }
    if (require_mtls) SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
    if (require_mtls) SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ssl_ctx = SSL_CTX_new(TLS_server_method());
    SSL_CTX_set_min_proto_version(ssl_ctx, TLS1_2_VERSION);
    SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
    SSL_CTX_set_cipher_list(ssl_ctx, "HIGH:!aNULL:!MD5");
    if (!ssl_ctx) { std::cerr << "OpenSSL: SSL_CTX_new failed\n"; return 4; }
    if (SSL_CTX_use_certificate_file(ssl_ctx, tls_cert.c_str(), SSL_FILETYPE_PEM) <= 0) { std::cerr << "OpenSSL: cert load failed\n"; return 4; }
    if (SSL_CTX_use_PrivateKey_file(ssl_ctx, tls_key.c_str(), SSL_FILETYPE_PEM) <= 0) { std::cerr << "OpenSSL: key load failed\n"; return 4; }
  }
#else
  bool use_tls = false;
  if (!tls_cert.empty() || !tls_key.empty()) {
    std::cerr << "TLS requested but OpenSSL disabled at build time.\n";
    return 4;
  }
#endif

  // Create socket
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) { std::cerr << "Failed to create socket\n"; return 5; }
  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

#ifndef _WIN32
  // Set close-on-exec for hygiene
  fcntl(server_fd, F_SETFD, FD_CLOEXEC);
#endif

  sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_addr.s_addr = htonl(INADDR_ANY); addr.sin_port = htons((uint16_t)port);
  if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) { std::cerr << "bind failed\n"; return 6; }
  listen(server_fd, 64);
  logj("INFO", std::string("nwx_serve listening on port ")+std::to_string(port)+(use_tls?" (TLS)":""));

  // Receive timeout
#ifdef _WIN32
  int tv = rcv_timeout_ms;
  setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#else
  timeval tv{ rcv_timeout_ms/1000, (rcv_timeout_ms%1000)*1000 };
  setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

  std::signal(SIGINT, handle_term);
#ifndef _WIN32
  std::signal(SIGTERM, handle_term);
  std::signal(SIGHUP, handle_hup);
#endif

  static std::atomic<long long> requests{0}, pred_requests{0}; static std::atomic<int> active_conns{0};
  std::unordered_map<std::string, TokenBucket> buckets;
  std::atomic<long long> total_requests{0}; std::atomic<long long> total_errors{0}; std::map<double,long long> lat_buckets{{0.005,0},{0.01,0},{0.025,0},{0.05,0},{0.1,0},{0.25,0},{0.5,0},{1.0,0},{2.5,0},{5.0,0},{10.0,0}}; std::map<long long,long long> size_buckets{{128,0},{256,0},{512,0},{1024,0},{2048,0},{4096,0},{8192,0},{16384,0},{32768,0},{65536,0}};

  while (!g_stop.load()) {
    if (g_reload.exchange(false)) { std::lock_guard<std::mutex> lk(bmtx); ModelBundle nb; if (load_bundle(nb, model_on_disk)) b = nb; }
    sockaddr_in peer{}; socklen_t plen = sizeof(peer);
    int client = accept(server_fd, (sockaddr*)&peer, &plen);
    if (client < 0) continue;
    if (active_conns.load() >= max_conc) { std::string resp=http_response(503, "{\"error\":\"busy\"}"); send(client, resp.c_str(), (int)resp.size(), 0); #ifdef _WIN32
closesocket(client);
#else
close(client);
#endif
 continue; }
    active_conns++;
    std::thread([&,client,peer]{

    char ipbuf[64]; std::string ip = "unknown";
#ifdef _WIN32
    inet_ntop(AF_INET, &peer.sin_addr, ipbuf, sizeof(ipbuf));
#else
    inet_ntop(AF_INET, &peer.sin_addr, ipbuf, sizeof(ipbuf));
#endif
    ip = ipbuf;

    auto read_request_plain = [&](std::string& req)->bool{
      req.resize(8192);
#ifdef _WIN32
      int n = recv(client, req.data(), (int)req.size(), 0);
#else
      int n = (int)recv(client, req.data(), req.size(), 0);
#endif
      if (n <= 0) return false;
      req.resize(n);
      return true;
    };

#ifdef NWX_HAVE_OPENSSL
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
    auto read_request_tls = [&](SSL* ssl, std::string& req)->bool{
      req.resize(8192);
      int n = SSL_read(ssl, req.data(), (int)req.size());
      if (n <= 0) return false;
      req.resize(n);
      return true;
    };
#endif

    auto write_all_plain = [&](const std::string& resp){
#ifdef _WIN32
      send(client, resp.c_str(), (int)resp.size(), 0);
      closesocket(client);
#else
      send(client, resp.c_str(), resp.size(), 0);
      close(client);
#endif
    };

#ifdef NWX_HAVE_OPENSSL
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
    auto write_all_tls = [&](SSL* ssl, const std::string& resp){
      SSL_write(ssl, resp.c_str(), (int)resp.size());
      SSL_shutdown(ssl);
      SSL_free(ssl);
#ifdef _WIN32
      closesocket(client);
#else
      close(client);
#endif
    };
#endif

    requests++;
#ifdef NWX_HAVE_OPENSSL
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
    SSL* ssl = nullptr;
    if (use_tls) {
    if (!client_ca.empty()) { if (!SSL_CTX_load_verify_locations(ssl_ctx, client_ca.c_str(), nullptr)) { std::cerr<<"OpenSSL: failed to load client CA\n"; return 4; } require_mtls=true; }
    if (require_mtls) SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
    if (require_mtls) SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
      ssl = SSL_new(ssl_ctx);
      SSL_set_fd(ssl, client);
      if (SSL_accept(ssl) <= 0) { write_all_plain(http_response(400, "{\"error\":\"tls_handshake\"}")); continue; }
    }
#endif

    std::string req; auto tstart = clock_t::now(); std::string traceparent=""; total_requests.fetch_add(1, std::memory_order_relaxed);
      std::string request_id="";
    bool ok =
#ifdef NWX_HAVE_OPENSSL
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
      (use_tls ? read_request_tls(ssl, req) : read_request_plain(req));
#else
      read_request_plain(req);
#endif
    if (!ok) {
#ifdef NWX_HAVE_OPENSSL
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
      if (use_tls) {
    if (!client_ca.empty()) { if (!SSL_CTX_load_verify_locations(ssl_ctx, client_ca.c_str(), nullptr)) { std::cerr<<"OpenSSL: failed to load client CA\n"; return 4; } require_mtls=true; }
    if (require_mtls) SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
    if (require_mtls) SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr); write_all_tls(ssl, http_response(400, "{\"error\":\"bad_request\"}")); }
      else { write_all_plain(http_response(400, "{\"error\":\"bad_request\"}")); }
#else
      write_all_plain(http_response(400, "{\"error\":\"bad_request\"}"));
#endif
      continue;
    }

    thread_local int last_status_code = 200;
    thread_local std::string last_path = "";
    auto respond = [&](int code, const std::string& body, const std::string& ctype="application/json"){ last_status_code = code;
      thread_local std::string extra_headers="";
#ifdef NWX_HAVE_ZLIB
      bool do_gzip = (std::getenv("NWX_GZIP") && std::string(std::getenv("NWX_GZIP"))=="1" && head.find("Accept-Encoding: gzip")!=std::string::npos);
      std::string out_body = body; std::string hdr;
      if (do_gzip) {
        std::string comp; comp.resize(body.size()+64);
        z_stream zs{}; deflateInit2(&zs, Z_BEST_SPEED, Z_DEFLATED, 15+16, 8, Z_DEFAULT_STRATEGY);
        zs.next_in = (Bytef*)body.data(); zs.avail_in = (uInt)body.size();
        zs.next_out = (Bytef*)comp.data(); zs.avail_out = (uInt)comp.size();
        int r = deflate(&zs, Z_FINISH); if (r==Z_STREAM_END){ comp.resize(zs.total_out); out_body.swap(comp);} deflateEnd(&zs);
        hdr = std::string("HTTP/1.1 ") + std::to_string(code) + " OK\r\nContent-Type: " + ctype + "\r\nContent-Encoding: gzip\r\nX-Content-Type-Options: nosniff\r\n";
      } else {
        hdr = std::string("HTTP/1.1 ") + std::to_string(code) + " OK\r\nContent-Type: " + ctype + "\r\nX-Content-Type-Options: nosniff\r\n";
      }
#else
      std::string out_body = body; std::string hdr = std::string("HTTP/1.1 ") + std::to_string(code) + " OK\r\nContent-Type: " + ctype + "\r\nX-Content-Type-Options: nosniff\r\n";
#endif
      if(!CORS_ALLOW.empty()) hdr += std::string("Access-Control-Allow-Origin: ")+CORS_ALLOW+"\r\n";
      if (use_tls) hdr += "Strict-Transport-Security: max-age=31536000; includeSubDomains\r\n";
      hdr += "Permissions-Policy: interest-cohort=()\r\nCross-Origin-Opener-Policy: same-origin\r\nCross-Origin-Resource-Policy: cross-origin\r\n";
      hdr += "X-Request-Id: " + request_id + "\r\n";
      hdr += extra_headers; extra_headers.clear();
      hdr += "Content-Length: " + std::to_string(out_body.size()) + "\r\n\r\n";
      std::string out = hdr + out_body; send(client, out.c_str(), (int)out.size(), 0);
    };
      if (code>=400) total_errors.fetch_add(1, std::memory_order_relaxed);
      auto hdr = std::string("HTTP/1.1 ") + std::to_string(code) + " OK\r\nContent-Type: " + ctype + "\r\nX-Content-Type-Options: nosniff\r\nContent-Security-Policy: default-src 'none'; frame-ancestors 'none'; base-uri 'none'\r\nReferrer-Policy: no-referrer\r\n";
      if(!CORS_ALLOW.empty()) hdr += std::string("Access-Control-Allow-Origin: ")+CORS_ALLOW+"\r\n";
      hdr += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
      std::string out = hdr + body; send(client, out.c_str(), (int)out.size(), 0);
    };
      if (!traceparent.empty()) logj("INFO", std::string("severity=INFO " "traceparent=")+traceparent);

      auto resp = http_response(code, body, ctype);
#ifdef NWX_HAVE_OPENSSL
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
      if (use_tls) write_all_tls(ssl, resp); else write_all_plain(resp);
#else
      write_all_plain(resp);
#endif
    };

    // Routing & small parser
    
    // KServe v2: POST /v2/models/{name}/infer  OR TF Serving v1: POST /v1/models/{name}:predict
    if (req.rfind("POST /v2/models/", 0) == 0 && req.find("/infer") != std::string::npos) {
      // We reuse the same JSON body format; return JSON with predictions
      // by mapping to /predict handler path expectations.
      // Simply fall through to prediction branch below by rewriting request line.
      size_t sp = req.find(' '); size_t eol = req.find('\r\n');
      req.replace(0, eol, "POST /predict HTTP/1.1");
    } else if (req.rfind("POST /v1/models/", 0) == 0 && req.find(":predict") != std::string::npos) {
      size_t eol = req.find('\r\n'); req.replace(0, eol, "POST /predict HTTP/1.1");
    }

    auto header_end = req.find("\r\n\r\n");
    auto get_header = [&](const std::string& key)->std::string{ auto p = head.find(key); if (p==std::string::npos) return ""; auto st = p + key.size(); auto en = head.find("\r\n", st); return head.substr(st, en-st); };
    static std::vector<std::string> allowed_tokens;
    if (allowed_tokens.empty()){
      const char* t = std::getenv("NWX_TOKEN"); if (t) allowed_tokens.push_back(std::string("Bearer ")+t);
      const char* ts = std::getenv("NWX_TOKENS"); if (ts){ std::string v(ts); size_t pos=0; while(true){ size_t c=v.find(',',pos); std::string tok=v.substr(pos, c==std::string::npos? std::string::npos : c-pos); if(!tok.empty()) allowed_tokens.push_back(std::string("Bearer ")+tok); if (c==std::string::npos) break; pos=c+1; } }
    }
    auto authv = get_header("Authorization: ");
    auto is_authorized = [&](){ if (allowed_tokens.empty()) return true; for (auto& a: allowed_tokens) if (authv==a) return true; return false; };
    parse_rl_env();

    const char* mb = std::getenv("NWX_MAX_BODY"); if (mb){ long long maxb = atoll(mb); auto clp = head.find("Content-Length: "); if (clp!=std::string::npos){ auto st=clp+16; auto en=head.find("\r\n", st); long long cl = atoll(head.substr(st,en-st).c_str()); if (cl>maxb){ problem_json(413, "payload_too_large", "Content-Length exceeds NWX_MAX_BODY"); continue; } } }
    // Request ID: echo or generate
    auto ridpos = req.find("X-Request-Id: "); if (ridpos!=std::string::npos){ auto st=ridpos+14; auto en=req.find("\r\n",st); request_id = req.substr(st, en-st); } else { request_id = gen_request_id(); }
    auto low = req; for (auto& c: low) c = (char)tolower((unsigned char)c);
    auto tpp = low.find("traceparent: "); if (tpp!=std::string::npos){ auto e = low.find("\r\n", tpp); traceparent = req.substr(tpp+12, e-(tpp+12)); }
    std::string head = header_end==std::string::npos? req : req.substr(0, header_end+4);
    auto has = [&](const std::string& s){ return head.find(s)!=std::string::npos; };

    if (req.find("GET /healthz") == 0) { respond(200, "{\"status\":\"ok\"}"); continue; }
    if (req.find("GET /readyz") == 0) { respond(200, "{\"ready\":true}"); continue; }
    if (req.find("OPTIONS ") == 0) {
      std::string body="";
      std::string hdr = "HTTP/1.1 204 No Content\r\n";
      if(!CORS_ALLOW.empty()) hdr += std::string("Access-Control-Allow-Origin: ")+CORS_ALLOW+"\r\n";
      hdr += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\nAccess-Control-Allow-Headers: Authorization, Content-Type, Traceparent\r\nAccess-Control-Max-Age: 600\r\nContent-Length: 0\r\n\r\n";
      std::string out = hdr; send(client, out.c_str(), (int)out.size(), 0); continue;
    } else if (req.find("GET /stats") == 0) {
      std::ostringstream st; st<<"{\"requests\":"<< total_requests.load() <<",\"errors\":"<< total_errors.load() <<"}"; respond(200, st.str()); continue;
    } else if (req.find("GET /signature") == 0) {
      ModelBundle lb; { std::lock_guard<std::mutex> lk(bmtx); lb = b; }
      std::ostringstream js; js << "{\"input_dim\":"<< lb.model.W1.cols <<",\"hidden\":"<< lb.model.W1.rows <<",\"output_dim\":"<< lb.model.W2.cols <<",\"activation\":\"" << lb.model.activation << "\",\"temperature\":"<< lb.model.temperature <<"}";
      respond(200, js.str()); continue;
    } else if (req.find("GET /modelcard") == 0) {
      std::ifstream f("MODEL_CARD.json"); if(!f.good()){ respond(404, "{\"error\":\"no_model_card\"}"); continue; }
      std::stringstream buf; buf<<f.rdbuf(); respond(200, buf.str()); continue;
    } else if (req.find("GET /buildinfo") == 0) {
      auto feats = nwx::detect_cpu();
      std::ostringstream bi; bi<<"{\"version\":\""<< NWX_VERSION <<"\",\"cpu\":{\"avx2\":"
      <<(feats.avx2?"true":"false")<<",\"fma\":"<<(feats.fma?"true":"false")<<"},\"tls\":"<<(use_tls?"true":"false")<<"}";
      respond(200, bi.str()); continue;
    } else if (req.find("GET /openapi.yaml") == 0) {
      std::ifstream f("openapi/openapi.yaml"); if(!f.good()){ problem_json(404, "not_found", "openapi spec not found"); continue; }
      std::stringstream buf; buf<<f.rdbuf(); std::string body = buf.str(); std::string etag = weak_etag_from(body);
      auto inmpos = head.find("If-None-Match: "); if (inmpos!=std::string::npos){ auto st=inmpos+15; auto en=head.find("\r\n", st); std::string inm=head.substr(st,en-st); if (inm==etag){ std::string hdr = "HTTP/1.1 304 Not Modified\r\nETag: "+etag+"\r\nX-Request-Id: "+request_id+"\r\n\r\n"; std::string out = hdr; send(client, out.c_str(), (int)out.size(), 0); continue; } }
      std::string hdr = "HTTP/1.1 200 OK\r\nContent-Type: text/yaml\r\nETag: "+etag+"\r\nX-Request-Id: "+request_id+"\r\n"; if(!CORS_ALLOW.empty()) hdr += std::string("Access-Control-Allow-Origin: ")+CORS_ALLOW+"\r\n"; hdr += "X-Content-Type-Options: nosniff\r\n"; if (use_tls) hdr += "Strict-Transport-Security: max-age=31536000; includeSubDomains\r\n"; hdr += "Content-Length: "+std::to_string(body.size())+"\r\n\r\n"; std::string out = hdr + body; send(client, out.c_str(), (int)out.size(), 0); continue;
      std::ifstream f("openapi/openapi.yaml");
      std::stringstream buf; buf << f.rdbuf();
      respond(200, buf.str(), "application/yaml"); continue;
    } else if (req.find("GET /metrics") == 0) { {
      std::ostringstream h;
      h << "# HELP nwx_request_latency_seconds Request latency\n# TYPE nwx_request_latency_seconds histogram\n";
      double sum=0.0; long long count=0; for(auto &kv: lat_buckets){ h << "nwx_request_latency_seconds_bucket{le=\"" << kv.first << "\"} " << kv.second << "\n"; count += kv.second; sum += kv.first * kv.second; } h << "nwx_request_latency_seconds_bucket{le=\"+inf\"} " << count << "\n" << "nwx_request_latency_seconds_sum " << sum << "\n" << "nwx_request_latency_seconds_count " << count << "\n";
      h << "# HELP nwx_request_size_bytes Request size\n# TYPE nwx_request_size_bytes histogram\n";
      long long scount=0; long long ssum=0; for(auto &kv: size_buckets){ h << "nwx_request_size_bytes_bucket{le=\"" << kv.first << "\"} " << kv.second << "\n"; scount += kv.second; ssum += kv.first * kv.second; } h << "nwx_request_size_bytes_bucket{le=\"+inf\"} " << scount << "\n" << "nwx_request_size_bytes_sum " << ssum << "\n" << "nwx_request_size_bytes_count " << scount << "\n";

      std::ostringstream m;
      m << "# HELP nwx_requests_total Total HTTP requests\n# TYPE nwx_requests_total counter\n";
      m << "nwx_requests_total " << requests.load() << "\n";
      m << "# HELP nwx_predict_requests_total Total prediction requests\n# TYPE nwx_predict_requests_total counter\n";
      m << "nwx_predict_requests_total " << pred_requests.load() << "\n";
      respond(200, m.str(), "text/plain; version=0.0.4"); continue;
    }

    if (req.find("POST /reload") == 0) {
      if (!token.empty()) { if (req.find("Authorization: Bearer ") == std::string::npos || req.find("Authorization: Bearer " + token) == std::string::npos) { if(!is_authorized()){ problem_json(401, "unauthorized", "missing or invalid credentials"); continue; } goto end; } }
      {
        std::lock_guard<std::mutex> lk(bmtx);
        ModelBundle nb; if (!load_bundle(nb, model_on_disk)) { respond(500, "{\"error\":\"reload_failed\"}"); goto end; }
        b = nb;
      }
      respond(200, "{\"reloaded\":true}"); goto end;
    } else 

if (req.find("POST /validate") == 0) {
      size_t bpos = req.find("\r\n\r\n");
      std::string body = (bpos==std::string::npos)? std::string() : req.substr(bpos+4);
#ifdef NWX_HAVE_ZLIB
      if (head.find("Content-Encoding: gzip")!=std::string::npos){ std::string out; if (inflate_gzip(body, out)) body.swap(out); }
#endif
      if (body.find("instances") == std::string::npos){ problem_json(400, "bad_request", "missing instances"); continue; }
      int rows=0, cols=0, cur=0; bool innum=false, fracm=false; int fcnt=0; double val=0, frac=0; int sign=1;
      auto flush=[&](){ innum=false; fracm=false; fcnt=0; val=0; frac=0; sign=1; };
      for (char c: body){
        if ((c>='0'&&c<='9')||c=='-'||c=='.'){
          if(!innum){ innum=true; val=0; frac=0; fcnt=0; fracm=false; sign=1; if(c=='-'){sign=-1; continue;} }
          if (c=='.'){ fracm=true; continue; }
          if (c>='0'&&c<='9'){ if(!fracm) val = val*10 + (c-'0'); else { frac = frac*10 + (c-'0'); fcnt++; } }
        } else {
          if (innum) flush();
          if (c==']'){ if (cols==0 && cur>0) cols=cur; if (cur>0){ rows++; cur=0; } }
          if (c==',') cur++;
        }
      }
      if (cols<=0 || rows<=0){ problem_json(400, "bad_request", "could not infer shape"); continue; }
      respond(204, "");
      continue;
    } else if (req.find("POST /predict/stream") == 0) {
      if(!is_authorized()){ problem_json(401, "unauthorized", "missing or invalid credentials"); continue; }
      // Rate limit
      if (RL_LIMIT>0){
        std::string token = authv;
        auto now = (long long)time(nullptr);
        long long remaining=0, reset=0;
        {
          std::lock_guard<std::mutex> lk(rl_mtx);
          auto& ent = rl_counts[token];
          if (ent.second < now){ ent = {0, now + RL_WINDOW}; }
          ent.first += 1;
          remaining = (ent.first<=RL_LIMIT)? (RL_LIMIT - ent.first) : 0;
          reset = ent.second - now;
        }
        if (remaining==0){
          extra_headers += "Retry-After: " + std::to_string(reset) + "\\r\\nRateLimit-Limit: " + std::to_string(RL_LIMIT) + "\\r\\nRateLimit-Remaining: 0\\r\\nRateLimit-Reset: " + std::to_string(reset) + "\\r\\n";
          problem_json(429, "too_many_requests", "rate limit exceeded"); continue;
        } else {
          extra_headers += "RateLimit-Limit: " + std::to_string(RL_LIMIT) + "\\r\\nRateLimit-Remaining: " + std::to_string(remaining) + "\\r\\nRateLimit-Reset: " + std::to_string(reset) + "\\r\\n";
        }
      }

      std::string hdr = "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nCache-Control: no-cache\r\nConnection: keep-alive\r\n";
      if(!CORS_ALLOW.empty()) hdr += std::string("Access-Control-Allow-Origin: ")+CORS_ALLOW+"\r\n";
      hdr += "X-Content-Type-Options: nosniff\r\n";
      if (use_tls) hdr += "Strict-Transport-Security: max-age=31536000; includeSubDomains\r\n";
      hdr += "\r\n"; send(client, hdr.c_str(), (int)hdr.size(), 0);
      size_t bpos = req.find("\r\n\r\n"); std::string body = (bpos==std::string::npos)? std::string() : req.substr(bpos+4);
#ifdef NWX_HAVE_ZLIB
      if (head.find("Content-Encoding: gzip")!=std::string::npos){ std::string out; if (inflate_gzip(body, out)) body.swap(out); }
#endif
      if (body.find("instances") == std::string::npos){ problem_json(400, "bad_request", "missing instances"); continue; }
      std::vector<double> nums; int rows=0, cols=0; int cur=0; bool innum=false, fracm=false; int fcnt=0; double val=0, frac=0; int sign=1;
      auto flushnum = [&](){ double v = sign * (val + (fcnt? frac/std::pow(10,fcnt) : 0)); nums.push_back(v); innum=false; fracm=false; fcnt=0; val=0; frac=0; sign=1; };
      for (char c: body){
        if ((c>='0'&&c<='9') || c=='-' || c=='.'){
          if (!innum){ innum=true; val=0; frac=0; fcnt=0; fracm=false; sign=1; if(c=='-'){ sign=-1; continue; } }
          if (c=='.'){ fracm=true; continue; }
          if (c>='0'&&c<='9'){ int d=c-'0'; if(!fracm) val = val*10 + d; else { frac = frac*10 + d; fcnt++; } }
        } else {
          if (innum) flushnum();
          if (c==']'){ if (cols==0 && cur>0) cols=cur; if (cur>0){ rows++; cur=0; } }
          if (c==',') cur++;
        }
      }
      if (cols==0){ problem_json(400, "bad_request", "could not infer shape"); continue; }
      ModelBundle lb; { std::lock_guard<std::mutex> lk(bmtx); lb = b; if (quantize_env) enable_runtime_quantization(lb.model); }
      nwx::Tensor2D X(rows, cols);
      for (int r=0;r<rows;++r) for (int c=0;c<cols;++c) X(r,c)=nums[r*cols+c];
      if (lb.has_scaler) X = lb.scaler.transform(X);
      auto P = nwx::softmax(lb.model.forward(X));
      for (int r=0;r<P.rows;++r){
        std::ostringstream line; line << "data: {\\\"row\\\":"<<r<<",\\\"probs\\\":[";
        for (int c=0;c<P.cols;++c){ if(c) line<<","; line<<P(r,c); }
        line << "]}\n\n"; auto sline=line.str(); send(client, sline.c_str(), (int)sline.size(), 0);
      }
      continue;
    } else if (req.find("POST /predict") == 0) {
      if(!is_authorized()){ problem_json(401, "unauthorized", "missing or invalid credentials"); continue; }
      if (RL_LIMIT>0){
        std::string token = authv; auto now=(long long)time(nullptr); long long remaining=0, reset=0;
        { std::lock_guard<std::mutex> lk(rl_mtx); auto& ent = rl_counts[token]; if (ent.second < now){ ent={0, now+RL_WINDOW}; } ent.first += 1; remaining = (ent.first<=RL_LIMIT)? (RL_LIMIT-ent.first):0; reset = ent.second - now; }
        if (remaining==0){ extra_headers += "Retry-After: " + std::to_string(reset) + "\\r\\nRateLimit-Limit: " + std::to_string(RL_LIMIT) + "\\r\\nRateLimit-Remaining: 0\\r\\nRateLimit-Reset: " + std::to_string(reset) + "\\r\\n"; problem_json(429, "too_many_requests", "rate limit exceeded"); continue; }
        else { extra_headers += "RateLimit-Limit: " + std::to_string(RL_LIMIT) + "\\r\\nRateLimit-Remaining: " + std::to_string(remaining) + "\\r\\nRateLimit-Reset: " + std::to_string(reset) + "\\r\\n"; }
      }

      /* Batching hook start */
      bool use_batch = (BATCH_SIZE>0);
      /* Batching hook end */

      bool want_ndjson = (head.find("Accept: application/x-ndjson")!=std::string::npos) || (req.find("POST /predict?format=ndjson")==0);
      // Auth
      if (!token.empty()) {
        if (!has("Authorization: Bearer ") || head.find("Authorization: Bearer " + token) == std::string::npos) {
          if(!is_authorized()){ problem_json(401, "unauthorized", "missing or invalid credentials"); continue; } continue;
        }
      }
      // Per-IP token bucket
      long long now_ms = (long long)(std::chrono::duration_cast<std::chrono::milliseconds>(clock_t::now().time_since_epoch()).count());
      std::string bucket_key = ip; if (!token.empty()) { auto p = head.find("Authorization: Bearer "); if (p!=std::string::npos){ auto st = p+22; auto en = head.find("\r\n", st); bucket_key = head.substr(st, en-st); } }
      auto& bkt = buckets[bucket_key];
      if (bkt.last_ms == 0) bkt.last_ms = now_ms;
      double elapsed = (now_ms - bkt.last_ms)/1000.0; bkt.last_ms = now_ms;
      bkt.tokens = std::min((double)burst, bkt.tokens + rate * elapsed);
      if (bkt.tokens < 1.0) { respond(429, "{\"error\":\"rate_limited\"}"); continue; }
      bkt.tokens -= 1.0;

      // Content-Length limit
      size_t clen = 0;
      auto clpos = head.find("Content-Length:");
      if (clpos != std::string::npos) {
        clen = (size_t)std::stoll(head.substr(clpos+15));
        if (clen > max_body) { respond(400, "{\"error\":\"body_too_large\"}"); continue; }
      }
      std::string body = (header_end==std::string::npos) ? "" : req.substr(header_end+4);
      size_t body_len = body.size(); for (auto &kv: size_buckets) if (body_len <= (size_t)kv.first) { kv.second++; break; }
      if (clen && body.size() < clen) { /* would read more, omitted for brevity */ }

      // Parse body (CSV or JSON {"instances":[[...] ]})
      std::vector<std::vector<double>> rows;
      std::string body_type = "csv";
      if (!body.empty() && body[0] == '{') body_type = "json";
      if (body_type == "json") {
        size_t pos = body.find("["); size_t end = body.rfind("]");
        if (pos!=std::string::npos && end!=std::string::npos && end>pos) {
          std::string arr = body.substr(pos, end-pos+1);
          std::vector<double> cur; std::vector<std::vector<double>> currow; std::string num;
          for (char c: arr) {
            if ((c>='0' && c<='9') || c=='-' || c=='.' || c=='e' || c=='E') num.push_back(c);
            else if (c==',' || c==']') { if(!num.empty()){ try{ cur.push_back(std::stod(num)); }catch(...){cur.push_back(0.0);} num.clear(); } if (c==']') { if(!cur.empty()){ rows.push_back(cur); cur.clear(); } }
          }
        }
      } else {
        std::stringstream ss(body);
        std::string line;
        while (std::getline(ss, line)) {
          if (line.empty()) continue;
          if (line.size() > 4096) { respond(400, "{\"error\":\"line_too_long\"}"); continue; }
          std::stringstream ls(line);
          std::string cell; std::vector<double> vals;
          while (std::getline(ls, cell, ',')) { try { vals.push_back(std::stod(cell)); } catch(...) { vals.push_back(0.0);} }
          if (!vals.empty()) rows.push_back(std::move(vals));
        }
      }

      std::vector<std::vector<double>> rows;
      std::stringstream ss(body);
      std::string line;
      while (std::getline(ss, line)) {
        if (line.empty()) continue;
        if (line.size() > 4096) { respond(400, "{\"error\":\"line_too_long\"}"); continue; }
        std::stringstream ls(line);
        std::string cell; std::vector<double> vals;
        while (std::getline(ls, cell, ',')) { try { vals.push_back(std::stod(cell)); } catch(...) { vals.push_back(0.0);} }
        if (!vals.empty()) rows.push_back(std::move(vals));
      }
      if (rows.empty()) { respond(400, "{\"error\":\"empty body\"}"); continue; }
      int in_dim = (int)rows[0].size();
      Tensor2D X((int)rows.size(), in_dim);
      for (int i=0;i<(int)rows.size();++i) for (int j=0;j<in_dim;++j) X(i,j)=rows[i][j];
      ModelBundle lb; { std::lock_guard<std::mutex> lk(bmtx); lb = b; if (quantize_env) enable_runtime_quantization(lb.model); }
      if (lb.has_scaler) X = lb.scaler.transform(X);
      auto L = lb.model.forward(X);
      for (double &u : L.data) u /= lb.model.temperature;
      auto probs = softmax(L);
      std::ostringstream out; out << "[";
      for (int i=0;i<probs.rows;++i) {
        if (i) out << ",";
        out << "{\"label\":"; int best=0; double bv=probs(i,0);
        for (int j=1;j<probs.cols;++j) if (probs(i,j)>bv){bv=probs(i,j); best=j;}
        out << best << ",\"probs\":[";
        for (int j=0;j<probs.cols;++j) { if (j) out << ","; out << probs(i,j); }
        out << "]}";
      }
      out << "]";
      pred_requests++;
      auto tend = clock_t::now(); double sec = std::chrono::duration<double>(tend - tstart).count(); for (auto &kv: lat_buckets) if (sec <= kv.first) { kv.second++; break; } respond(200, out.str());
      continue;
    }

    respond(200, "{\"ok\":true}");
end: ;
  }

#ifndef _WIN32
  close(server_fd);
#else
  closesocket(server_fd);
#endif

#ifdef NWX_HAVE_OPENSSL
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
  if (ssl_ctx) SSL_CTX_free(ssl_ctx);
#endif

  std::cerr << "nwx_serve stopped\n";
  return 0;
}
