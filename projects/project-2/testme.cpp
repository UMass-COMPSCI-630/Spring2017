#include <iostream>
using namespace std;

//#include "gcmalloc.hh"
extern "C"
{
  //  void * xxmalloc(size_t);
  //  void xxfree(void *);
}

int testme;

int main()
{
  cout << (size_t) &testme << endl;
  cout << "-----------" << endl;
  int ** p1 = (int **) malloc(8);
  int * p2 = (int *) malloc(8);
  char * p = nullptr;
  cout << "p1 address = " << (size_t) &p1 << ", p2 address = " << (size_t) &p2 << endl;
  cout << "p address = " << (size_t) &p << endl;
  cout << "p1 should be reachable: " << (size_t) p1 << endl;
  cout << "p2 should be reachable: " << (size_t) p2 << endl;
  *p1 = p2;
  *p2 = 12;
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
