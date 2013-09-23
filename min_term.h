#ifndef MIN_TERM_H
#define MIN_TERM_H
/************************************************
Name: min_term.h
Source: min_term.h 09/06/13
Purpose: min term rule application
Author: Zhuo Li, Yilin Zhang, Chuck Alpert
************************************************/

//----------------------------------------------
// Standard Include Library
//----------------------------------------------
#include <utility>
#include <algorithm>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <fstream>

//----------------------------------------------
// Local Include Library
//----------------------------------------------
#include "timer.h"
#include "constants.h"
#include "subclass.h"
#include "logic_simulate.h"

//----------------------------------------------
// STL Include Library
//----------------------------------------------
#include <list>
#include <vector>
#include <queue>
#include <string>
using namespace std;

class MintermHelper{
  const LogicSimulate* const _p_logic_simulate;
  public:
  MintermHelper(const LogicSimulate* const p_logic_simulate):_p_logic_simulate(p_logic_simulate){}
  //CalculateSubcallBound shared by both FindMinterm and FindMintermOpt
  void CalculateSubcallBound(const int& x, const int& y, const int& y_end, const int& end_round, int& not_take_start_round, int&not_take_start_team)const;

  void ApplyExclusiveRule(const int& x, const int& y, Matrix<ELEMENT>& take, vector<int>& take_max_points, vector<int>& take_min_points)const;
  void ApplyReverseRegressiveRule(const int& x, const int& y, const int& end_round, Matrix<ELEMENT>& take, vector<int>& take_max_points, vector<int>& take_min_points)const;
  //A + A'B = A + B
  bool ApplyReductionRule(SOP& products)const;
  //S4_5'S4_6' = S4_5'
  bool ApplySelfDominateRule(SOP& products)const;
  //S4_6'M1_6 = M1_6 only
  bool ApplyBracketRule(SOP& products)const;
  //S4_6' S4_6 = vanish
  bool ApplySelfDenyRule(SOP& products)const;
  //1 + X = 1
  bool ApplyAlwaysTrueRule(SOP& products)const;
  //S4_5' + S4_5 = ""
  bool ApplyComplementationRule(SOP& products)const;
  //A_1 + A_2 = A1; A_1' + A_2' = A_2'
  bool ApplyInterDominateRule(SOP& products)const;
  //S4_6' M1_6' W9_6' = E4_; S4_6' M1_5' W9_6' = E4_6 M1_5'; S4_5' = E4_5
  bool ApplyReverseRule(SOP& products)const;
  //Extract common part out
  void Extract(const Product& product_i, const Product& product_j, Product& common, SOP& reduced_pair)const;
  bool ApplyCombineRule(SOP& products)const;

  void Apply(const int& x, const int& y, Matrix<ELEMENT>& take)const;

  bool FindMinterm(const Matrix<ELEMENT>& diff, const int& points,const int& start_round, const int& start_team, const int& end_round, const int& end_team, vector<string>& minterms)const;
  bool FindMintermOpt(const Matrix<ELEMENT>& diff, const vector<int>& max_points, const vector<int>& min_points, const int& points, const int& start_round, const int& start_team, const int& end_round, const int& end_team, vector<string>& minterms)const;
};
#endif 
