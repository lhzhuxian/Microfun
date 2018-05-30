#ifndef _REQUEST_H
#define _REQUEST_H

#include "common.hpp"

class http_request {
  string method;
  string url;
  vector<string> headers;
  vector<string> values;
  void * next;
  void * data;
public:
  void set_method(const char * data, size_t length);
  int set_url(const char * data, size_t length);
  int set_headers(const char * data, size_t length);
  int set_values(const char * data, size_t length);
  int set_data(const char * data, size_t length);
}
