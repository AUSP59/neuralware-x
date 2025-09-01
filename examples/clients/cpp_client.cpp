// SPDX-License-Identifier: Apache-2.0
#include <iostream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif
int main(){
  std::string host="127.0.0.1"; int port=8080;
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr{}; addr.sin_family=AF_INET; addr.sin_port=htons(port); inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
  connect(sock, (sockaddr*)&addr, sizeof(addr));
  std::string body = "{\"instances\":[[0,1],[1,0]]}";
  std::string req = "POST /predict HTTP/1.1\r\nHost: localhost\r\nAuthorization: Bearer SECRET\r\nContent-Type: application/json\r\nContent-Length: "+std::to_string(body.size())+"\r\n\r\n"+body;
#ifdef _WIN32
  send(sock, req.c_str(), (int)req.size(), 0);
  char buf[4096]; int n = recv(sock, buf, sizeof(buf), 0);
  closesocket(sock);
#else
  send(sock, req.c_str(), req.size(), 0);
  char buf[4096]; int n = (int)recv(sock, buf, sizeof(buf), 0);
  close(sock);
#endif
  if (n>0) std::cout.write(buf, n);
  return 0;
}
