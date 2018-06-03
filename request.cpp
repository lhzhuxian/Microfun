
#include "request.hpp"


void http_request::set_url(const char * m) {
  method = m;
}
void http_request::set_url(const char * data, int len){
  url += string(data, len);
}

void http_request::set_headers(const char * data, int len){
  int size = headers.size();
  if(size > values.size()) {
    headers[size - 1] += string(data, len);
  } else { 
    headers.push_back(string(data, len));
  }
}
void http_request::set_values(const char * data, int len){
  int size = values.size();
  if (size < headers.size()) {
    values.push_back(string(data, len));
  } else {
    valuse[size - 1] += string(data, len);
  }
}

void http_request::set_data(const char *data, int len) {
  for(int i = 0; i < len; i++) {
    data.push_back(data[len]);
  }
}

http_response::http_response(int i): request_id(i), len(0){}

