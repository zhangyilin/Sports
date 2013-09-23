#ifndef TIMER_H
#define TIMER_H
/************************************************
Name: timer.h
Source: timer.h 03/27/10
Author: Zhuo Li
************************************************/

//----------------------------------------------
// Standard Include Library
//----------------------------------------------
#include <sys/resource.h>

//----------------------------------------------
// Local Include Library
//----------------------------------------------
#include "constants.h"
using namespace std;

//----------------------------------------------
// STL Include Library
//----------------------------------------------


  enum NodeType {
    PLANT,
    CITY,
    TRANSFORMER,
    STEINER
  };

  enum EdgeType {
    VHT,
    UGHT,
    HT
  };

  class Timer {
  public:
    // Constructor
    Timer() {};

    // Accessor
    double dtime() const { return _dtime; }

    // Function
    void starttimer();
    void stoptimer();

    static bool draw_xgraph;
    static bool draw_tcl;
    static bool onlyShowGraph;
    static bool satisfy_flow;
    static enum DebugLevel DEBUGLEVEL;

  private:
    double _dtime;
    long _sTime, _uTime;
  };


#endif
