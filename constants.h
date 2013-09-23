#ifndef CONSTANTS_H
#define CONSTANTS_H

//----------------------------------------------
// Standard Include Library
//----------------------------------------------
#include <utility>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <math.h>

//----------------------------------------------
// Local Include Library
//----------------------------------------------

//----------------------------------------------
// STL Include Library
//----------------------------------------------
#include <list>
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <numeric>  //std::accumulate

#define NUM_OF_REGION_TEAM 16
#define NUM_OF_TOTAL_TEAM 64
#define NUM_OF_REGIONS 4
#define NUM_ROUNDS 6
/* 
   M midwest, W west, S south, E east
   M plays with W in the semi-finals
   S plays with E in the semi-finals
*/
typedef enum {M=0,W,S,E}REGION_SYMBOL;
/*
  MUST_WIN means player 1 team won, but player 2 team did not. 
  MUST_LOSE means player 2 team won, but player 1 team did not.
  WIN means player 1 chose this team, but player 2 did not.
  LOSE means player 2 chose that team, but player 1 did not. 
  ZERO means same picks or does not matter.
*/
typedef enum {MUST_WIN=2, MUST_LOSE=-2, WIN=1, LOSE=-1, ZERO=0}ELEMENT;

enum DebugLevel {
  MINIMUM,
  DETAIL,
  DUMP
};

//log2(NUM_OF_TOTAL_TEAM)
const int POINTS[] = {10, 20, 40, 80, 160, 320};
const int POW2[] = {1, 2, 4, 8, 16, 32, 64};
const int SEED[] = {1, 16, 8, 9, 5, 12, 4, 13, 6, 11, 3, 14, 7, 10, 2, 15};
const int REV_SEED[] = {1, 15, 11, 7, 5, 9, 13, 3, 4, 14, 10, 6, 8, 12, 16, 2};
//round pointtypedef enum {};
#endif
