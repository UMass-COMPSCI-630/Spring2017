#ifndef TPRINTF_H
#define TPRINTF_H

#include <unistd.h>
#include <cmath>

using namespace std;

const int MAXBUF = 65536;

template <typename T>
void writeval(T v);

int itoa(char * buf, long n) {
  auto startbuf = buf;
  if (n < 0) {
    *buf++ = '-';
    n = -n;
  }
  if (n == 0) {
    *buf++ = '0';
    return (int) (buf - startbuf);
  }
  long tens = 1L;
  while (n / (10 * tens)) {
    tens *= 10;
  }
  while (tens) {
    *buf++ = '0' + n / tens;
    n = n - (n / tens) * tens;
    tens /= 10;
  }
  return (int) (buf - startbuf);
}

int ftoa(char * buf, double n, int decimalPlaces = 4) {
  // Extract integer part
  auto ipart = (long) n;
  
  // Extract floating part
  auto fpart = n - (float) ipart;
  if (fpart < 0.0) {
    fpart = -fpart;
  }
  
  // convert integer part to string
  int i = itoa(buf, ipart);
  
  if (decimalPlaces > 0) {
    buf[i] = '.';
    fpart = fpart * pow(10, decimalPlaces);
    i = i + itoa(buf + i + 1, (long) fpart) + 1;
  }
  return i;
}

void writeval(int n) {
  char buf[MAXBUF];
  int len = itoa(buf, n);
  write(1, buf, len);
}

void writeval(long n) {
  char buf[MAXBUF];
  int len = itoa(buf, n);
  write(1, buf, len);
}

void writeval(unsigned long n) {
  char buf[MAXBUF];
  int len = itoa(buf, n);
  write(1, buf, len);
}

void writeval(double n) {
  char buf[MAXBUF];
  int len = ftoa(buf, n);
  write(1, buf, len);
}
  
void writeval(const char * str) {
  write(1, str, strlen(str));
}

void writeval(const char c) {
  char buf[1];
  buf[0] = c;
  write(1, buf, 1);
}

void tprintf(const char* format) // base function
{
  writeval(format);
}

template<typename T, typename... Targs>
void tprintf(const char * format, T value, Targs... Fargs)
{
  for ( ; *format != '\0'; format++ ) {
    if ( *format == '@' ) {
      if ( *(format + 1) == '@') {
	writeval("@");
	format = format+2;
      } else {
	writeval(value);
	tprintf(format+1, Fargs...);
	return;
      }
    }
    writeval(*format);
  }
}

#endif
