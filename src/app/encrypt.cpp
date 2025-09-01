// SPDX-License-Identifier: Apache-2.0
#include <iostream>
#include <fstream>
#include <vector>
#ifdef NWX_HAVE_OPENSSL
#include <openssl/evp.h>
#include <openssl/rand.h>
#endif
int main(int argc, char** argv){
  if (argc<5){ std::cerr<<"Usage: nwx_encrypt --in <file> --out <file> (--encrypt|--decrypt) --key <hex64>\n"; return 2; }
  std::string in, out, mode, keyhex;
  for (int i=1;i<argc;++i){ std::string a=argv[i];
    if(a=="--in"&&i+1<argc) in=argv[++i];
    else if(a=="--out"&&i+1<argc) out=argv[++i];
    else if((a=="--encrypt"||a=="--decrypt")&&mode.empty()) mode=a;
    else if(a=="--key"&&i+1<argc) keyhex=argv[++i];
  }
#ifndef NWX_HAVE_OPENSSL
  std::cerr<<"OpenSSL not available\n"; return 3;
#else
  std::ifstream fi(in, std::ios::binary); if(!fi.good()){ std::cerr<<"input not found\n"; return 4; }
  std::vector<unsigned char> src((std::istreambuf_iterator<char>(fi)),{});
  std::vector<unsigned char> key(32,0);
  if (keyhex.size()<64){ std::cerr<<"key must be 64 hex chars\n"; return 5; }
  auto hex2=[&](const std::string& h){ for(int i=0;i<32;i++){ unsigned v=0; sscanf(h.c_str()+i*2, "%2x", &v); key[i]=(unsigned char)v; } };
  hex2(keyhex);
  if (mode=="--encrypt"){
    std::vector<unsigned char> iv(12); RAND_bytes(iv.data(), (int)iv.size());
    std::vector<unsigned char> outbuf(src.size()+16);
    EVP_CIPHER_CTX* ctx=EVP_CIPHER_CTX_new(); int len=0, olen=0;
    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data());
    EVP_EncryptUpdate(ctx, outbuf.data(), &len, src.data(), (int)src.size()); olen=len;
    std::vector<unsigned char> tag(16);
    EVP_EncryptFinal_ex(ctx, outbuf.data()+olen, &len); olen+=len;
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data());
    EVP_CIPHER_CTX_free(ctx);
    std::ofstream fo(out, std::ios::binary); fo.write((char*)iv.data(), iv.size()); fo.write((char*)outbuf.data(), olen); fo.write((char*)tag.data(), tag.size()); fo.close();
    std::cout<<"wrote "<<out<<"\n"; return 0;
  } else if (mode=="--decrypt"){
    if (src.size()<28){ std::cerr<<"ciphertext too small\n"; return 6; }
    std::vector<unsigned char> iv(src.begin(), src.begin()+12);
    std::vector<unsigned char> tag(src.end()-16, src.end());
    std::vector<unsigned char> ct(src.begin()+12, src.end()-16);
    std::vector<unsigned char> pt(ct.size());
    EVP_CIPHER_CTX* ctx=EVP_CIPHER_CTX_new(); int len=0, olen=0;
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data());
    EVP_DecryptUpdate(ctx, pt.data(), &len, ct.data(), (int)ct.size()); olen=len;
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag.data());
    int ok=EVP_DecryptFinal_ex(ctx, pt.data()+olen, &len); EVP_CIPHER_CTX_free(ctx);
    if (ok!=1){ std::cerr<<"decrypt failed\n"; return 7; }
    std::ofstream fo(out, std::ios::binary); fo.write((char*)pt.data(), olen); fo.close(); std::cout<<"wrote "<<out<<"\n"; return 0;
  } else { std::cerr<<"unknown mode\n"; return 8; }
#endif
}
