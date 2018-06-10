#include <iostream>
#include <cstdint>
#include <memory>
using namespace std;

int main(void) {
  char * a = new char[50];
  unique_ptr<char> up1(a);
}
