#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_parser.h"
#define alloc_cpy(dest, src, len) \
    dest = malloc(len + 1);\
    memcpy(dest, src, len);\
    dest[len] = '\0';
void print_range(const char *buf, size_t len) {
  char *temp_buf = (char *)malloc(len);
  memcpy(temp_buf, buf, len);
  temp_buf[len] = '\0';
  printf("%s\n", temp_buf);
  free(temp_buf);
}

struct http_header {
    char *name, *value;
    struct http_header *next;
};

static inline struct http_header *new_http_header() {
    struct http_header *header = malloc(sizeof(struct http_header));
    header->name = NULL;
    header->value = NULL;
    header->next = NULL;
    return header;
}

static inline void delete_http_header(struct http_header *header) {
    if (header->name != NULL) free(header->name);
    if (header->value != NULL) free(header->value);
    free(header);
}

struct http_request {
    char *method, *url, *body;
    unsigned int flags;
    unsigned short http_major, http_minor;
    struct http_header *headers;
};
#define F_HREQ_KEEPALIVE 0x01

static inline struct http_request *new_http_request() {
    struct http_request *request = malloc(sizeof(struct http_request));
    request->headers = NULL;
    request->url = NULL;
    request->body = NULL;
    request->flags = 0;
    request->http_major = 0;
    request->http_minor = 0;
    return request;
}

static inline void delete_http_request(struct http_request *request) {
    if (request->url != NULL) free(request->url);
    if (request->body != NULL) free(request->body);
    struct http_header *header = request->headers;
    while (header != NULL) {
        struct http_header *to_delete = header;
        header = header->next;
        delete_http_header(to_delete);
    }
    free(request);
}

static inline struct http_header *add_http_header(struct http_request *request) {
    struct http_header *header = request->headers;

    while (header != NULL) {

        if (header->next == NULL) {
            header->next = new_http_header();
            return header->next;
        }
        header = header->next;
    }

    request->headers = new_http_header();
    return request->headers;
}
int null_cb(http_parser *parser) { return 0; }

int url_cb(http_parser *parser, const char *buf, size_t len) {
    printf("on url_cb: \n");
    print_range(buf, len);

    struct http_request *request = (struct http_request *) parser->data;

    request->method = (char *)http_method_str(parser->method);
    request->http_major = parser->http_major;
    request->http_minor = parser->http_minor;

    alloc_cpy(request->url, buf, len)
    printf("\n");
    return 0;
}

int header_field_cb(http_parser *parser, const char *buf, size_t len) {
    printf("on header_field_cb: \n");
    print_range(buf, len);
    printf("\n");
    printf("len = %d\n", len);
    struct http_request *request = (struct http_request *) parser->data;

    struct http_header *header = add_http_header(request);

    alloc_cpy(header->name, buf, len)
    return 0;
}

int header_value_cb(http_parser *parser, const char *buf, size_t len) {
    printf("on header_value_cb: \n");
    print_range(buf, len);
    printf("\n");
    struct http_request *request = (struct http_request *) parser->data;

    struct http_header *header = request->headers;

    while (header->next != NULL) {
        header = header->next;
    }

    alloc_cpy(header->value, buf, len)
    return 0;
}

int body_cb(http_parser *parser, const char *buf, size_t len) {
    printf("on body_cb: \n");
    print_range(buf, len);
    printf("\n");
    struct http_request *request = (struct http_request *) parser->data;
    alloc_cpy(request->body, buf, len)
    return 0;
}

int on_message_begin(http_parser *parser) {
  printf("on_message_begin\n");
  return 0;
}

int on_message_complete(http_parser *parser) {
  printf("on_message_complete\n");
  return 0;
}

int on_headers_complete(http_parser *parser) {
  printf("on_headers_complete\n");
  return 0;
}

static http_parser_settings parser_settings =
{
     .on_message_begin    = on_message_begin
    ,.on_message_complete = on_message_complete
    ,.on_headers_complete = on_headers_complete
    ,.on_header_field     = header_field_cb
    ,.on_header_value     = header_value_cb
    ,.on_url              = url_cb
    ,.on_body             = body_cb
};

const char * http_get_raw = "GET /favicon.ico HTTP/1.1\r\n"
         "Host: 0.0.0.0=5000\r\n"
         "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0\r\n"
         "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
         "Accept-Language: en-us,en;q=0.5\r\n"
         "Accept-Encoding: gzip,deflate\r\n"
         "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
         "Keep-Alive: 300\r\n"
         "Connection: keep-alive\r\n"
         "\r\n";
const char * http_post_raw =  "POST /post_identity_body_world?q=search#hey HTTP/1.1\r\n"
         "Accept: */*\r\n"
         "Transfer-Encoding: identity\r\n"
         "Content-Length: 5\r\n"
         "\r\n"
  "World";
const char * http_trunk_part_1 ="POST /post_identity_body_world?q=search#hey HTTP/1.1\r\n"
  "Accept";
const char * http_trunk_part_2 =": */*\r\n"
  "Transfer-Encoding: identity\r\n"
  "Content-Length: 5\r\n"
  "\r\n"
  "W"
  "orldfdafds";



const char * http_trunk_head =  "POST /chunked_w_trailing_headers HTTP/1.1\r\n"
         "Transfer-Encoding: chunked\r\n"
         "\r\n"
         "5\r\nhello\r\n"
         "6\r\n world\r\n"
         "0\r\n"
         "Vary: *\r\n"
         "Content-Type: text/plain\r\n"
         "\r\n";
const char *http_trunk_head1 = "POST /two_chunks_mult_zero_end HTTP/1.1\r\n"
         "Transfer-Encoding: chunked\r\n"
         "\r\n"
         "5\r\nhello\r\n"
         "6\r\n world\r\n"
         "000\r\n"
         "\r\n";

/*
const char * http_trunk_part_1 =  "POST /chunked_w_trailing_headers HTTP/1.1\r\n"
         "Transfer-Encoding: chunked\r\n"
         "\r\n"
         "5\r\nhel";
const char * http_trunk_part_2 = "lo\r\n6\r\n world\r\n"
         "0\r\n"
         "Vary: *\r\n"
         "Content-Type: text/plain\r\n"
         "\r\n";
*/
// 测试HTTP GET报文
void test_http_get() {
  printf("Test HTTP GET\n");
  http_parser *parser = (http_parser *)malloc(sizeof(http_parser));
  http_parser_init(parser, HTTP_REQUEST);
  struct http_request *requst = new_http_request();
  parser->data = requst;
  int res = http_parser_execute(parser, &parser_settings, http_get_raw, strlen(http_get_raw));
  printf("method: %s, version: %u.%u\n",
     requst->method, requst->http_major, requst->http_minor);
  delete_http_request(requst);
  free(parser);
  printf("END Test HTTP GET\n\n\n");
}
// 测试HTTP POST报文
void test_http_post() {
  printf("Test HTTP POST\n");
  http_parser *parser = (http_parser *)malloc(sizeof(http_parser));
  http_parser_init(parser, HTTP_REQUEST);
  struct http_request *requst = new_http_request();
  parser->data = requst;
  int res = http_parser_execute(parser, &parser_settings, http_post_raw, strlen(http_post_raw));
  printf("method: %s, version: %u.%u\n",
     requst->method, requst->http_major, requst->http_minor);
  delete_http_request(requst);
  free(parser);
  printf("END Test HTTP POST\n\n\n");
}
// 测试含有托挂字段的HTTP trunk编码的报文
void test_http_chunk() {
  printf("Test HTTP CHUNK\n");
  http_parser *parser = (http_parser *)malloc(sizeof(http_parser));
  http_parser_init(parser, HTTP_REQUEST);
  struct http_request *requst = new_http_request();
  parser->data = requst;
  int res = http_parser_execute(parser, &parser_settings, http_trunk_head, strlen(http_trunk_head));
  printf("method: %s, version: %u.%u\n",
     requst->method, requst->http_major, requst->http_minor);
  delete_http_request(requst);
  free(parser);
  printf("END Test HTTP TRUNK\n\n\n");
}
// 测试HTTP trunk编码的报文: trunk编码以多个0表明结束
void test_http_chunk1() {
  printf("Test HTTP CHUNK 1\n");
  http_parser *parser = (http_parser *)malloc(sizeof(http_parser));
  http_parser_init(parser, HTTP_REQUEST);
  struct http_request *requst = new_http_request();
  parser->data = requst;
  int res = http_parser_execute(parser, &parser_settings, http_trunk_head1, strlen(http_trunk_head1));
  printf("method: %s, version: %u.%u\n",
     requst->method, requst->http_major, requst->http_minor);
  delete_http_request(requst);
  free(parser);
  printf("END Test HTTP TRUNK 1\n\n\n");
}
// 测试HTTP trunk编码的报文: 将一个完整的报文分两次解析
void test_http_chunk_part() {
  printf("Test HTTP CHUNK PART\n");
  http_parser *parser = (http_parser *)malloc(sizeof(http_parser));
  http_parser_init(parser, HTTP_REQUEST);
  struct http_request *requst = new_http_request();
  parser->data = requst;
  int res = http_parser_execute(parser, &parser_settings, http_trunk_part_1, strlen(http_trunk_part_1));
  //printf("method: %s, version: %u.%u\n",
     //requst->method, requst->http_major, requst->http_minor);

  res = http_parser_execute(parser, &parser_settings, http_trunk_part_2, strlen(http_trunk_part_2));
   printf("method: %s, version: %u.%u\n",
     requst->method, requst->http_major, requst->http_minor);
  delete_http_request(requst);
  free(parser);
  printf("END Test HTTP TRUNK PART\n\n\n");
}
void main(int argc, char **argv) {
  //test_http_get();
  //test_http_post();
  //test_http_chunk();
  //test_http_chunk1();
  test_http_chunk_part();
}
