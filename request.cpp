
#include "request.hpp"


void http_request::set_method(const char *data) {
  method = data;
}

void http_request::set_url(const char * data, unsigned int len){
  url = url + string(data, len);
}

void http_request::set_headers(const char * data, unsigned int len){
  int size = headers.size();
  if(size > values.size()) {
    headers[size - 1] = headers[size - 1] + string(data, len);
  } else { 
    headers.push_back(string(data, len));
  }
}
void http_request::set_values(const char * data, unsigned int len){
  int size = values.size();
  if (size < headers.size()) {
    values.push_back(string(data, len));
  } else {
    values[size - 1] = values[size - 1] + string(data, len);
  }
}

void http_request::set_data(const char *d, unsigned int len) {
  data = data + string(d, len);
}




