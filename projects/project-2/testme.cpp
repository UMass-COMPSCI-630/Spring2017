#include <iostream>
using namespace std;

//#include "gcmalloc.hh"
extern "C"
{
  //  void * xxmalloc(size_t);
  //  void xxfree(void *);
}

int main()
{
  cout << "-----------" << endl;
  int ** p1 = (int **) malloc(8);
  int * p2 = (int *) malloc(8);
  *p1 = p2;
  *p2 = 12;
  char * p = nullptr;
  for (int i = 0; i < 200000; i++) {
    char * ptr = (char *) malloc(256);
    ptr[0] = 'X';
    if (i == 10000) {
      p = ptr;
      ptr[0] = 'Y';
    }
    cout << i << " -- " << (size_t) ptr << endl;
  }
  cout << **p1 << endl; // should be 12
  cout << p[0] << endl; // should be Y
  return 0;
}
