
#include "request.hpp"

void http_request::set_method(const char *data, int len) {
  method = move(new string(data, len));
}
void http_request::set_url(const char * data, int len){
  url = move(new string(data, len));
}

void http_request::set_headers(const char * data, int len){
  headers.push_back(new string(data, len));
}
void http_request::set_values(const char * data, int len){
  values.push_back(new string(data, len));
  next = data + len + 5;
}
