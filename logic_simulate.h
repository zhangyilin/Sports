#ifndef LOGIC_SIMULATE_H
#define LOGIC_SIMULATE_H
/************************************************
Name: logic_simulate.h
Source: logic_simulate.h 04/16/10
Purpose: graph class for power game
Author: Zhuo Li
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

//----------------------------------------------
// STL Include Library
//----------------------------------------------
#include <list>
#include <vector>
#include <queue>
#include <string>
using namespace std;

class LogicSimulate {
  string _team_file;
  string _pick_file;
  string _outcome_file;
  int _completed_round;
  bool _explore_all_minterms;
  DebugLevel _debug_level;
  bool _optimization;
  int _max_player_name_length;

  vector<Region> _regions;
  vector<Player> _players;
  Player _outcome;
  public:
  // Constructor
  LogicSimulate(){}
  ~LogicSimulate() {};

  // useful functions
  void ReadConfigure (const string & configure_file);
  void ReadInput();
  void ReadOutcome(ifstream & outcome);
  void Run();
  void PrunePlayersWithMaxScore();
  void CompareAllPlayers();
  void PrintStatistics();
  int IndexTeam (REGION_SYMBOL r, int seating)const;
  int IndexTeam (string r, int seating)const;
  //bool FindMinterm(const Matrix & diff, int& points, int& start_round, int& start_team, int& end_round, int& end_team, vector<string>& minterms)const;
  //bool FindMintermOpt(const Matrix & diff, const Matrix & must, int& points, int& start_round, int& start_team, int& end_round, int& end_team, vector<string>& minterms)const;
  void PrintPlayerInfo() const;
  template <class T>
  void PrintPickMatrix(const Matrix<T>& diff) const;
  //void PrintPickMatrix(const Matrix<short>& diff) const;
  //void PrintPickMatrix(const Matrix<ELEMENT>& diff) const;
  void PrintVSMatrix(const Matrix<short>& vs) const;
  void GenerateMaxMinPoints(const Matrix<ELEMENT>& diff, vector<int>& max_points, vector<int>& min_points)const;
  SOP CompareTwo(const Player& player1, const Player& player2)const;
  SOP And (const vector<SOP>& products)const;

  // set and get functions
  string get_team_file()const{return _team_file;}
  string get_pick_file()const{return _pick_file;}
  string get_outcome_file()const{return _outcome_file;}
  int get_completed_round()const{return _completed_round;}
  Player get_outcome()const{return _outcome;}
  bool get_explore_all_minterms()const{return _explore_all_minterms;}
  DebugLevel get_debug_level()const{return _debug_level;}

  void add_region (Region region){_regions.push_back(region);}
  vector<Player> get_players() const{return _players;}
  void set_players(const vector<Player>& players){_players = players;}
  // input is the REGION_SYMBOL, output the corresponding Region 
  Region get_region(REGION_SYMBOL r)const{return _regions[(int)r];}
  // input is the y index of the matrix, will return the corresponding region
  Region calculate_region(int y) const {
    int index = (int)floor((y-1)/NUM_OF_REGION_TEAM);
    return _regions[index];
  }
  // input is the y index of the matrix, will return the corresponding team
  Team calculate_team(int y)const{
    Region region = calculate_region(y);
    vector<Team> teams = region.get_teams();
    return teams[y-region.get_region_symbol()*NUM_OF_REGION_TEAM - 1];
  }
  vector<Team> get_all_teams()const{
    vector<Team> all_teams;
    for (size_t i = 0; i < _regions.size(); ++i){
      Region region = _regions[i];
      vector<Team> teams = region.get_teams();
      all_teams.insert(all_teams.end(), teams.begin(), teams.end());
    }
    return all_teams;
  }



  // draw xgraph of this graph
  void drawXgraph(const char * filename, const char * name) const;

  // draw Sani's tcl viewer format of this graph
  void drawTcl(char * filename) const;
};

#endif 
