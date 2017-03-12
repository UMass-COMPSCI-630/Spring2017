#ifndef OS_SPECIFIC_H
#define OS_SPECIFIC_H

#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(__APPLE__)
#include <mach-o/getsect.h>
#endif

#   pragma weak __data_start
#   pragma weak data_start

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
public:

  static void getGlobals(void *& start, void *& end) {
#if defined(__APPLE__)
    // NOP for now.
    start = end = 0;
    return;
    
    // Note this is actually deprecated.
    start = (void *) get_etext();
    end   = (void *) get_end();
    if ((uintptr_t) &GCMallocGlobal < (uintptr_t) start) {
      start = &GCMallocGlobal;
    } else if ((uintptr_t) &GCMallocGlobal > (uintptr_t) end) {
      end = &GCMallocGlobal;
    }
#else
    start = (void *) __data_start;
    end = (void *) _end;
#endif
    if (start > end) {
      auto tmp = start;
      start = end;
      end = tmp;
    }
  }
   
  static void getStack(void *& start, void *& end) {
#if !defined(__APPLE__)
    unsigned long kstkesp, startstack;
    readStat(kstkesp, startstack);
    start = (void *) kstkesp;
    end   = (void *) startstack;
    tprintf("stack: start=@, end=@\n", (size_t) start, (size_t) end);
#else
    static pthread_t self = pthread_self();
    auto addr = pthread_get_stackaddr_np(self);
    auto size = pthread_get_stacksize_np(self);
    // Assumes stack grows down.
    start = (void *) ((uintptr_t) addr - size);
    end = addr;
#endif
  }

private:

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
};

#endif
