#include "common.hpp"

int main(void) {
  // set up listener
  int status;
  addrinfo hints;
  addrinfo *servinfo;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(NULL, "9999", &hints, &servinfo)) != 0) {
    cerr << "getaddrinfo error : " << gai_strerror(status) << endl;
  }

  int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
  string msg;
  cin >> msg;
  /*  
      char * msg ="GET http://g-ec2.images-amazon.com/images/G/01/social/api-share/amazon_logo_500500._V323939215_.png HTTP/1.1 \r\n"
    "Host: g-ec2.images-amazon.com\r\n"					\
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:59.0) Gecko/20100101 Firefox59.0\r\n" \
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/ /*;q=0.8\r\n" \
    "Accept-Language: en-US,en;q=0.5\r\n"					\
    "Accept-Encoding: gzip, deflate\r\n"					\
    "Connection: keep-alive\r\n"						\
    "Upgrade-Insecure-Requests: 1\r\n"					\
    "Pragma: no-cache\r\n"						\
    "Cache-Control: no-cache\r\n";
*/
  int len = msg.size();
  int bytes_sent = send(sockfd, msg.data(), len, 0);
  cout << "send: " << bytes_sent << endl;
  void * buf = malloc(len + 1);
  recv(sockfd, buf, len, 0);
  cout << static_cast<char *> (buf);


}
