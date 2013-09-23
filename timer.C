/************************************************
Name: timer.C
Source: timer.C 11/09/07
Author: Zhuo Li
************************************************/

//----------------------------------------------
// Standard Include Library
//----------------------------------------------
#include <stdio.h>

//----------------------------------------------
// Local Include Library
//----------------------------------------------
#include "timer.h"

//----------------------------------------------
// STL Include Library
//----------------------------------------------
#include <string>
using namespace std;

bool Timer::draw_xgraph = false;
bool Timer::draw_tcl = false;
bool Timer::satisfy_flow = false;
enum DebugLevel Timer::DEBUGLEVEL = MINIMUM;

//----------------------------------------------
// Timer class routines
//----------------------------------------------

  // start timer
  void Timer::starttimer() 
  {
    struct rusage timestuff;
    getrusage(RUSAGE_SELF, &timestuff);
    _sTime = timestuff.ru_utime.tv_sec;
    _uTime = timestuff.ru_utime.tv_usec;
    return;
  }

  // stop timer
  void Timer::stoptimer() 
  {
    struct rusage timestuff;
    getrusage(RUSAGE_SELF, &timestuff);
    _dtime = (double)(timestuff.ru_utime.tv_sec - _sTime)
      + 1.0e-6*(double)(timestuff.ru_utime.tv_usec - _uTime);
    return;
  }
