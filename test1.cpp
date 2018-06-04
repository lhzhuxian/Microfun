#include <iostream>
#include <cstdint>
using namespace std;

int main(void) {
  int i1 = 0;
  int i2 = 1;
  int i3 = 2;
  int i4 = 4;
  cout << i1 % 3 << endl;
  cout << i2 % 3 << endl;
  cout << (i3 & 2) << endl;
  cout << (i4 & 2) << endl;
  cout << i4 % 3 << endl;
}
