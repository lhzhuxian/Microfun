#ifndef _REQUEST_H
#define _REQUEST_H


#include <string>
#include <vector>
using namespace std;

struct http_request {
  string method;
  string url;
  vector<string> headers;
  vector<string> values;
  //void * next;
  string data;
  int id;
  int fd;
  //int status; // -1 ini, 0 start, 1 finished

  void set_method(const char * data);
  void set_url(const char * data, unsigned int length);
  void set_headers(const char * data, unsigned int length);
  void set_values(const char * data, unsigned int length);
  void set_data(const char * data, unsigned int length);
};
struct http_response{
  int id;
  int len;
  int offset;
  string data;
};

#endif
