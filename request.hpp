#ifndef _REQUEST_H
#define _REQUEST_H

#include "common.hpp"

class http_request {
  string method;
  string url;
  unordered_map<string, string> headers;
public:
  http_request();
  ~http_request();
  
}
