#ifndef OS_SPECIFIC_H
#define OS_SPECIFIC_H

#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <utility>
#include <array>
#include <vector>
#include <sstream>
#include <cstring>

#if defined(__APPLE__)
#define _XOPEN_SOURCE 1
#endif

#include <ucontext.h>

#if !defined(__APPLE__)

#pragma weak __data_start
#pragma weak data_start

#endif

extern "C" {
#if !defined(__APPLE__)
  extern int _end[];
  extern char **__environ;
  
  extern char etext;
  extern char edata;
  extern char end;

  extern int __data_start[], data_start[];
#endif
}

extern void * GCMallocGlobal;

class OSSpecific {
private:
  
  // The maximum number of global regions we will manage.
  enum { MAX_GLOBALS = 1024 };

  // All ranges of globals (start, end).
  array<pair<void *, void *>, MAX_GLOBALS> globals;

  // The number of global regions.
  int numGlobals;

  // True iff we have initialized everything.
  bool initialized;
  
#if defined(__APPLE__)
  // A buffer containing the contents of vmmap's output;
  // we later parse this to get the globals.
  char result[131072];
#endif
  
public:

  OSSpecific()
    : numGlobals (0),
      initialized (false)     
  {
  }

  void initialize() {
    if (initialized) {
      return;
    }
    //    tprintf("initializing.\n");
#if defined(__APPLE__)
    pread(getpid(), result, sizeof(result));
#endif
    // Read in list of global regions.
#if !defined(__APPLE__)
    void * start, * end;
    start = (void *) __data_start;
    if (start == 0) {
      start = (void *) data_start;
    }
    end = (void *) _end;
    if (start == 0) {
      end = 0;
    }
    start = (void *) (((uintptr_t) start + 7) & ~7);
    globals[numGlobals++]= pair<void *, void *>(start, end);
#else
    // Need to iterate over a bunch of sections.
    // Now parse out __DATA addresses.
    char strbuf[65536];
    char * str = strtok_r(result, "\n", (char **) &strbuf);
    while (true) {
      if (strstr(str, "__DATA") == str) {
	char * ptr = str + 6;
	while (*ptr == ' ') {
	  ptr++;
	}
	char * startStr = ptr;
	while (((*ptr >= '0') && (*ptr <= '9')) ||
	       ((*ptr >= 'a') && (*ptr <= 'f'))) {
	  ptr++;
	}
	if (*ptr == '-') {
	  *ptr = '\0';
	  auto start = strtoul(startStr, nullptr, 16);
	  ptr++;
	  char * endStr = ptr;
	  while (*ptr != ' ') {
	    ptr++;
	  }
	  *ptr = '\0';
	  auto end = strtoul(endStr, nullptr, 16);
	  void * startptr = (void *) (((uintptr_t) start + 7) & ~7);
	  void * endptr = (void *) end;
	  if (end > start) {
	    if (numGlobals < MAX_GLOBALS) {
	      globals[numGlobals++]= pair<void *, void *>(startptr, endptr);
	    }
	  }
	}
      }
      str = strtok_r(nullptr, "\n", (char **) &strbuf);
      if (str == nullptr) {
	break;
      }
    }
#endif
    initialized = true;
  }

  // Execute a function on every word in the registers.
  void walkRegisters(const std::function< void(void *) >& f) {
#if defined(__APPLE__)
    // Give up. getcontext just doesn't work in this context :(.
#else // linux
    ucontext_t ucp;
    getcontext(&ucp);
    void * bufStart;
    size_t bufSize;
    bufStart = &ucp.uc_mcontext.gregs;
    bufSize = sizeof(ucp.uc_mcontext.gregs);
    const auto startVal = (uintptr_t) bufStart; // + sizeof(void*) - 1) & ~(sizeof(void *) - 1);
    const auto endVal   = (uintptr_t) bufStart + bufSize;
    for (auto ptr = startVal; ptr < endVal; ptr += sizeof(void *)) {
      void ** p = (void **) ptr;
      f((void *) *p);
    }
#endif
  }
  
  // Execute a function on every word on the stack.
  void walkStack(const std::function< void(void *) >& f) {
    void * start, * end;
    getStack(start, end);
    const auto startVal = (uintptr_t) start;
    const auto endVal   = (((uintptr_t) end + 4095) & ~4095) - 1; // Round up to next page.
    for (auto ptr = startVal; ptr < endVal; ptr += sizeof(void *)) {
      void ** p = (void **) ptr;
      f((void *) *p);
    }
  }

  // Execute a function on every word in the global space.
  void walkGlobals(const std::function< void(void *) >& f) {
    initialize();
    for (int i = 0; i < numGlobals; i++) {
      const auto start = (uintptr_t) globals[i].first;
      const auto end   = (uintptr_t) globals[i].second;
      for (auto ptr = start; ptr < end; ptr += sizeof(void *)) {
	void ** p = (void **) ptr;
	f((void *) *p);
      }
    }
  }

private:

  // Get the range of the stack.
  static void getStack(void *& start, void *& end) {
#if !defined(__APPLE__)
    unsigned long kstkesp, startstack;
    readStat(kstkesp, startstack);
    start = (void *) kstkesp;
    end   = (void *) startstack;
#else
    static pthread_t self = pthread_self();
    auto addr = pthread_get_stackaddr_np(self);
    auto size = pthread_get_stacksize_np(self);
    // Assumes stack grows down.
    start = (void *) ((uintptr_t) addr - size);
    end = addr;
#endif
    start = (void *) (((uintptr_t) start + 7) & ~7);
  }

  // Parse every value out of stat, but keep only the stack pointer and start of the stack.
  static void readStat(unsigned long& kstkesp, unsigned long& startstack)
  {
    int fd = open("/proc/self/stat", O_RDONLY);
    char buf[4096];
    size_t sz = read(fd, buf, 4096);
    //  void * addr = mmap(0, 4096, PROT_READ, MAP_PRIVATE, fd, 0);
    int pid;
    char comm[255];
    char state;
    int ppid;
    int pgrp;
    int session;
    int tty_nr;
    int tpgid;
    unsigned flags;
    unsigned long minflt, cminflt, majflt, cmajflt, utime, stime;
    long cutime, cstime, priority, nice, num_threads, itrealvalue;
    unsigned long long starttime;
    unsigned long vsize;
    long rss;
    unsigned long rsslim, startcode, endcode;
    /*    unsigned long startstack; */
    /*    unsigned long kstkesp; */
    unsigned long kstkeip, signal, blocked, sigignore, sigcatch, wchan, nswap, cnswap;
    int exit_signal, processor;
    unsigned int rt_priority, policy;
    unsigned long long delayacct_blkio_ticks;
    unsigned long guest_time;
    long cguest_time;
    unsigned long start_data, end_data, start_brk, arg_start, arg_end, env_start, env_end;
    int exit_code;
    sscanf((char *) buf, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %d",
	   &pid,
	   (char *) &comm,
	   &state,
	   &ppid,
	   &pgrp,
	   &session,
	   &tty_nr,
	   &tpgid,
	   &flags,
	   &minflt,
	   &cminflt,
	   &majflt,
	   &cmajflt,
	   &utime,
	   &stime,
	   &cutime,
	   &cstime,
	   &priority,
	   &nice,
	   &num_threads,
	   &itrealvalue,
	   &starttime,
	   &vsize,
	   &rss,
	   &rsslim,
	   &startcode,
	   &endcode,
	   &startstack,
	   &kstkesp,
	   &kstkeip,
	   &signal,
	   &blocked,
	   &sigignore,
	   &sigcatch,
	   &wchan,
	   &nswap,
	   &cnswap,
	   &exit_signal,
	   &processor,
	   &rt_priority,
	   &policy,
	   &delayacct_blkio_ticks,
	   &guest_time,
	   &cguest_time,
	   &start_data,
	   &end_data,
	   &start_brk,
	   &arg_start,
	   &arg_end,
	   &env_start,
	   &env_end,
	   &exit_code);
  }
  
  int pread(int pid, char * buf, int buflen) {
    // Build up the command.
    // Nasty hackery here to redirect vmmap into a temporary file.
    char cmd[255];
    const auto progName = "/usr/bin/vmap";
    char fileName[] = "tempo     ";
    strcpy(cmd, "/usr/bin/vmmap        > tempo     ");
    int pidpos = itoa(&cmd[strlen(progName)+2], pid);
    itoa(&cmd[29], pid);
    system(cmd);
    int v = itoa(&fileName[5], pid);
    fileName[5+v] = '\0';
    int f = open(fileName, O_RDONLY);
    read(f, buf, buflen-1);
    close(f);
    unlink(fileName);
    return 0;
  }
  
};

#endif
