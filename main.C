/************************************************
Name: main.C
Source: main.C 04/27/10
Author: Zhuo Li
 ************************************************/

//----------------------------------------------
// Standard Include Library
//----------------------------------------------
#include <stdio.h>

//----------------------------------------------
// Local Include Library
//----------------------------------------------
#include "logic_simulate.h"
#include "subclass.h"
#include "timer.h"

//----------------------------------------------
// STL Include Library
//----------------------------------------------
#include <iostream>
using namespace std;


/***********************************************
Usage:
logic_simulation.exe configure
will read the input file and find power plant locations to minimize the total cost
 ************************************************/

int main(int argc, char ** argv) 
{
  Timer timer1, timer2;
  timer1.starttimer();
  timer2.starttimer();

  //read configure file
  LogicSimulate logic_simulate;
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " configure_file" << endl;
    exit(1);
  }
  logic_simulate.ReadConfigure(argv[1]);
  //according to configure file, read in input files
  logic_simulate.ReadInput();
  timer2.stoptimer();
  cout << "Reading input runtime is " << timer2.dtime() << endl;
  logic_simulate.Run();
  
  timer1.stoptimer();
  cout << "Total runtime is " << timer1.dtime() << endl;
  return EXIT_SUCCESS;
}
