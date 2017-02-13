#include <iostream>
using namespace std;

#include "statsalloc.h"

int main()
{
  reportStats();
  cout << "-----------" << endl;
  for (int i = 0; i < 10; i++) {
    char * ptr = (char *) malloc(171);
    *ptr = 'a' + i;
    *(ptr + 1) = '\0';
    //    cout << ptr << endl;
  }
  reportStats();
  walk([&](Header * h){ cout << "Object address = " << (void *) (h + 1) << ", requested size = " << h->requestedSize << ", allocated size = " << h->allocatedSize << endl; });
  return 0;
}
