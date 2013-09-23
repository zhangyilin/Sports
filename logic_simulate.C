/************************************************
Name: logic_simulate.C
Source: logic_simulate.C 04/27/10
Purpose: Solve power game problem
Author: Zhuo Li
 ************************************************/

//----------------------------------------------
// Standard Include Library
//----------------------------------------------
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <string.h>

//----------------------------------------------
// Local Include Library
//----------------------------------------------
#include "logic_simulate.h"
#include "min_term.h"
#include "timer.h"
#include "constants.h"

//----------------------------------------------
// STL Include Library
//----------------------------------------------
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>  //std::accumulate
using namespace std;

void LogicSimulate::ReadConfigure (const string & configure_file){
  ifstream iconfigure;
  iconfigure.open(configure_file.c_str());
  while (iconfigure){
    string name;
    iconfigure >> name;
    if (name[0] == '#'){
      string comment;
      getline (iconfigure, comment);
      if (_debug_level >= DUMP)
        cout << "comment is "<< name << comment << endl;
    }else if (name == "team_file"){
      iconfigure >> _team_file;
      if (_debug_level >= DUMP)
        cout << "team_file is "<< _team_file << endl; 
    }else if (name == "pick_file"){
      iconfigure >> _pick_file;
      if (_debug_level >= DUMP)
        cout << "pick_file is "<< _pick_file << endl; 
    }else if (name == "outcome_file"){
      iconfigure >> _outcome_file;
      if (_debug_level >= DUMP)
        cout << "outcome_file is "<< _outcome_file << endl; 
    }else if (name == "completed_round"){
      iconfigure >> _completed_round;
      if (_debug_level >= DUMP)
        cout << "completed_round is "<< _completed_round<< endl; 
    }else if (name == "debug_level"){
      string debug_level;
      iconfigure >> debug_level;
      if (debug_level == "minimum"){
        _debug_level = MINIMUM;
        if (_debug_level >= DUMP)
          cout << "debug_level is minimum"<< endl; 
      }else if (debug_level == "detail"){
        _debug_level = DETAIL;
        if (_debug_level >= DUMP)
          cout << "debug_level is detail"<< endl; 
      }else if (debug_level == "dump"){
        _debug_level = DUMP;
        if (_debug_level >= DUMP)
          cout << "debug_level is dump"<< endl; 
      }else{
        assert (true);
      }   
    }else if (name == "explore_all_minterms"){
      string explore_all_minterms;
      iconfigure >> explore_all_minterms;
      if (explore_all_minterms == "on"){
        _explore_all_minterms = true;
        if (_debug_level >= DUMP)
          cout << "explore_all_minterms is true"<< endl; 
      }else if (explore_all_minterms == "off"){
        _explore_all_minterms = false;
        if (_debug_level >= DUMP)
          cout << "explore_all_minterms is false"<< endl; 
      }else{
        assert (true);
      }
    }else if (name == "optimization"){
      string optimization;
      iconfigure >> optimization;
      if (optimization == "on"){
        _optimization = true;
        if (_debug_level >= DUMP)
          cout << "optimization is on"<< endl; 
      }else if (optimization == "off"){
        _optimization = false;
        if (_debug_level >= DUMP)
          cout << "optimization is off"<< endl; 
      }else{
        assert (true);
      }
    }else{
      assert (true);
    }
  }
}
//---------------------------------------------------------------------------------------------------- 
//Run
//Main routine for the logic simulate. Compare all players and then print final statistics
//---------------------------------------------------------------------------------------------------- 
void LogicSimulate::Run()
{
  PrunePlayersWithMaxScore();

  MintermHelper minterm_helper(this);

  /*
  vector<string> test_minterms;
  test_minterms.push_back("M1_5' S4_5' E4_5' 100");
  SOP test_products(test_minterms);
  minterm_helper.ApplyReverseRule(test_products);
  cout << test_products.get_string();

  vector<string> test_minterms4;
  test_minterms4.push_back("M1_6 E4_6 S4_6 100");
  SOP test_products4(test_minterms4);
  minterm_helper.ApplyReverseRule(test_products4);
  cout << test_products4.get_string();

  vector<string> test_minterms1;
  test_minterms1.push_back("M1_5' E4_5' 100");
  SOP test_products1(test_minterms1);
  minterm_helper.ApplyReverseRule(test_products1);
  cout << test_products1.get_string();

  vector<string> test_minterms3;
  test_minterms3.push_back("M1_6' E4_6' S4_6' 100");
  SOP test_products3(test_minterms3);
  minterm_helper.ApplyReverseRule(test_products3);
  cout << test_products3.get_string();

  vector<string> test_minterms5;
  test_minterms5.push_back("M1_6 E4_6 100");
  SOP test_products5(test_minterms5);
  minterm_helper.ApplyReverseRule(test_products5);
  cout << test_products5.get_string();

  vector<string> test_minterms6;
  test_minterms6.push_back("M1_6' E4_6' W9_5' 100");
  SOP test_products6(test_minterms6);
  minterm_helper.ApplyReverseRule(test_products6);
  cout << test_products6.get_string();
  */
  CompareAllPlayers();
  PrintStatistics();
}
//---------------------------------------------------------------------------------------------------- 
//PrunePlayersWithMaxScore
//Prune the players whose max score is less than best current score of top player
//(This is a fast pruning and shrink the size of pool first)
//Prune the players who has no chance to win certain other players
//(His any max potential score is less than certain other player's corresponding score based on diff matrix)
//---------------------------------------------------------------------------------------------------- 
void LogicSimulate::PrunePlayersWithMaxScore()
{
  Timer timer1;

  timer1.starttimer();
  //Prune the players whose max score is less than best current score of top player
  for (size_t i=0; i<_players.size(); ++i)
    _players[i].ComputeMaxScore(_completed_round);
  for (size_t i=1; i<_players.size(); ++i) {
    if (_players[i].get_max_score() < _players[0].get_total_score()) {
      _players[i].set_alive(0);
    }
  }
  // Sort all players based on their current score and resort them based on best available score.
  std::sort(_players.begin(), _players.end());
  timer1.stoptimer();
  cout << "Max score pruning runtime is " << timer1.dtime() << endl;

  cout << "After pruning based on max score" << endl;
  PrintPlayerInfo();

  if ( 0 ) {
    timer1.starttimer();
    for (size_t p1 = 0; p1 < _players.size(); ++p1) {
      if (!_players[p1].get_alive() || _players[p1].get_same_pick_player() > -1)
        continue;
      for (size_t p2 = p1+1; p2 < _players.size(); ++p2) {
        if (_players[p2].get_total_score() == _players[p1].get_total_score())
          _players[p1].MarkSamePick(_players[p2], _completed_round, p1);
        else
          break;
      }
    }
    int num_same_pick_players = 0;
    for (size_t p1 = 0; p1 < _players.size(); ++p1)
      if (_players[p1].get_same_pick_player() > -1)
        num_same_pick_players ++;
    cout << "The number of players with exact same score and picks are " << num_same_pick_players << endl;

    timer1.stoptimer();
    cout << "Marking same pick runtime is " << timer1.dtime() << endl;
  }

  timer1.starttimer();
  for (size_t p1 = 0; p1 < _players.size(); ++ p1)
    //    if (_players[p1].get_alive() && _players[p1].get_same_pick_player() == -1)
    if (_players[p1].get_alive())
      for (size_t p2 = p1+1; p2 < _players.size(); ++ p2)
        _players[p1].PrunePlayer(_players[p2], _completed_round);
  timer1.stoptimer();
  cout << "Two player pruning runtime is " << timer1.dtime() << endl;

  if ( 0 ) {
    for (size_t p1 = 0; p1 < _players.size(); ++ p1)
      if (_players[p1].get_alive() && _players[p1].get_same_pick_player() > -1 && !_players[_players[p1].get_same_pick_player()].get_alive())
        _players[p1].set_alive(0);
  }

  cout << "After pruning based on diff matrix" << endl;
  PrintPlayerInfo();
}
//---------------------------------------------------------------------------------------------------- 
//CompareAllPlayers
//Compare all players after easy pruning of losers
//---------------------------------------------------------------------------------------------------- 
void LogicSimulate::CompareAllPlayers()
{
  // Go through all player pairs.
  for (size_t player_1=0; player_1 < _players.size(); ++player_1){
    if (!_players[player_1].get_alive())
      continue;
    _players[player_1].get_min_terms().resize(_players.size());
    vector<SOP> & player1_min_terms = _players[player_1].get_min_terms();
    size_t player_2;
    for (player_2 = 0; player_2 < _players.size(); ++ player_2){
      if (player_1 == player_2){//ignore comparing player himself
        player1_min_terms[player_2] = SOP();
        continue;
      }
      // if x player has been dominated before, no need to compare to him again.
      if (!_players[player_2].get_alive())
        continue;
      if (_debug_level >= DETAIL)
        cout << "compares for "<<_players[player_1].get_name()<< " and "<< _players[player_2].get_name()<< " starts"<< endl;
      //core function within
      player1_min_terms[player_2] = CompareTwo (_players[player_1], _players[player_2]);
      if (_debug_level >= DETAIL)
        cout << "compares for "<< _players[player_1].get_name() << " and "<< _players[player_2].get_name()<< " is done"<< endl;
      if (player1_min_terms[player_2].Empty()) {
        //no need to proceed on this guy, he is already eliminated 
        _players[player_1].set_alive(0);
        break;
      }
      // else
      //        vs_matrix.set_element(player_1 + 1, player_2 + 1, 1);
    }
    if (_debug_level >= DETAIL) {
      cout << "all compares for "<< _players[player_1].get_name()<< " are done"<< endl;
      for (size_t index = 0; index < _players.size(); index++)
        cout << _players[index].get_alive() << ' ';
      cout << endl;
    }
  }
}
//---------------------------------------------------------------------------------------------------- 
//PrintStatistics()
//Print all statistics
//---------------------------------------------------------------------------------------------------- 
void LogicSimulate::PrintStatistics()
{
  Matrix<short> vs_matrix (_players.size(), _players.size());
  vs_matrix.Clear();

  string s;
  cout << "All possible winners and their winning scenarios are " << endl;
  for (size_t player_1 = 0; player_1 < _players.size(); player_1++) {
    if (!_players[player_1].get_alive())
      continue;
    cout << _players[player_1].get_name() << ": The winner scenarios versus each person and whole pools are: " << endl;
    vector<SOP> & player1_min_terms = _players[player_1].get_min_terms();
    for (size_t player_2 = 0; player_2 < _players.size(); player_2++) {
      if (player_1 == player_2) {
        player1_min_terms[player_2].Clear();
        continue;
      }
      if (!_players[player_2].get_alive()) {
        player1_min_terms[player_2].Clear();
        continue;
      }
      SOP minterms = player1_min_terms[player_2];
      cout << "\t" << _players[player_2].get_name() <<  " : ";
      cout << minterms.get_string();
    }
    SOP one_minterm = And(player1_min_terms);
    cout << "\t" << "Whole Pool : ";
    cout << one_minterm.get_string();
  }
  //   if (_debug_level >= DETAIL)
  //     PrintVSMatrix(vs_matrix);
}

//---------------------------------------------------------------------------------------------------- 
//PrintVSMatrix
//return nothing
//The function will print the matrix with players information
//This determines which players are out of game
//---------------------------------------------------------------------------------------------------- 
void LogicSimulate::PrintVSMatrix(const Matrix<short>& matrix) const{
  vector<Player> players = get_players();
  assert (matrix.get_y_size() == players.size());
  assert (matrix.get_x_size() == players.size());
  int name_length = 15;
  cout << setw(name_length)<< "-";
  for (size_t i = 0; i < players.size(); ++i){
    cout << setiosflags(ios::left)<< setw(name_length)<< players[i].get_name();
  }
  cout << endl;
  for (size_t y = 1; y <= matrix.get_y_size(); ++y){
    cout << setiosflags(ios::left)<< setw(name_length)<< players[y-1].get_name();
    for (size_t x = 1; x <= matrix.get_x_size(); ++x){
      if (x == matrix.get_x_size())
        cout << matrix.get_element(y,x);
      else
        cout << setw(name_length)<< matrix.get_element(y,x);
    }
    cout << endl;
  }
}

//---------------------------------------------------------------------------------------------------- 
//ReadOutcome
//read the outcome file
//---------------------------------------------------------------------------------------------------- 
void LogicSimulate::ReadOutcome(ifstream & ioutcome)
{
  _outcome.set_name("outcome");
  // read first 4-round picks
  for (int i = 0; i < NUM_OF_REGIONS; ++ i){
    //int y_off = i*NUM_OF_REGION_TEAM;//i*16
    int num_of_region_game = NUM_OF_REGION_TEAM-1;//num_of_region_game 15
    int round = 1;
    int num_of_round = NUM_ROUNDS-2;//num_of_round = 6-2, 2 is for final rounds
    //cout << num_of_round<< endl;
    for (int j = 0; j < num_of_region_game; ++j){
      if (j > num_of_region_game - pow(2, (num_of_round-round)))
        round++;
      int pick, index;
      ioutcome >> pick;
      index = IndexTeam ((REGION_SYMBOL) i, pick);
      //_outcome.set_matrix_element(y_off+pick, round);
      if ( round <= _completed_round)
        _outcome.set_matrix_element(index, round);
    }
  }

  // read semi-final and final picks
  string semi_final1, semi_final2, final;
  ioutcome >> semi_final1 >> semi_final2 >> final;
  string semi_final1_region, semi_final1_team, semi_final2_region, semi_final2_team, final_region, final_team;
  semi_final1_region = semi_final1[0];
  semi_final1_team = semi_final1.substr(1);
  semi_final2_region = semi_final2[0];
  semi_final2_team = semi_final2.substr(1);
  final_region = final[0];
  final_team = final.substr(1);
  assert (semi_final1_region == "M" || semi_final1_region == "W");
  assert (semi_final2_region == "S" || semi_final2_region == "E");
  int semi_final1_y = IndexTeam (semi_final1_region, atoi(semi_final1_team.c_str()));
  if ( NUM_ROUNDS-1 <= _completed_round)
    _outcome.set_matrix_element(semi_final1_y, NUM_ROUNDS-1);
  int semi_final2_y = IndexTeam (semi_final2_region, atoi(semi_final2_team.c_str()));
  if ( NUM_ROUNDS-1 <= _completed_round)
    _outcome.set_matrix_element(semi_final2_y, NUM_ROUNDS-1);
  int final_y = IndexTeam (final_region, atoi(final_team.c_str()));
  if ( NUM_ROUNDS <= _completed_round)
    _outcome.set_matrix_element(final_y, NUM_ROUNDS);

}
//---------------------------------------------------------------------------------------------------- 
//ReadInput
//return nothing
//iteam is the file handler of the team file (13bracket)
//ipick is the file handler of the pick file (2picks.txt)
//---------------------------------------------------------------------------------------------------- 
void LogicSimulate::ReadInput()
{
  //iteam records point of each team; ipick records picks from people
  ifstream iteam, ipick, ioutcome;

  iteam.open(_team_file.c_str());
  ipick.open(_pick_file.c_str());
  ioutcome.open(_outcome_file.c_str());
  if (!iteam)
    cerr << "Input file " << _team_file << " does not exist!" << endl;
  if (!ipick)
    cerr << "Input file " << _pick_file << " does not exist!" << endl;
  if (!ioutcome)
    cerr << "Input file " << _outcome_file << " does not exist!" << endl;

  // Team file read first
  for (int i = 0; i < NUM_OF_REGIONS; ++ i){
    Region region;
    region.set_region_symbol((REGION_SYMBOL) i);
    for (int j = 0; j < NUM_OF_REGION_TEAM; ++j){
      int seeding;
      string name;
      double point, home_point, middle_point, away_point;
      iteam >> seeding >> name >> point >> home_point >> middle_point >> away_point;
      Team team(seeding, name, point, home_point, middle_point, away_point);
      region.add_team (team);
    }
    add_region (region);
  }

  Timer timer1, timer2, timer3;
  timer1.starttimer();
  //read in outcome 
  ReadOutcome(ioutcome);
  timer1.stoptimer();
  cout << "Reading outcome runtime is " << timer1.dtime() << endl;

  timer2.starttimer();
  // read pick file and build player class
  string name;
  while (getline (ipick, name)){
    Player player;
    if (name[name.size()-1]=='\r')
      name.resize(name.size()-1);
    player.set_name(name);
    string line;
    // read first 4-round picks
    for (int i = 0; i < NUM_OF_REGIONS; ++ i){
      //int y_off = i*NUM_OF_REGION_TEAM;//i*16
      int num_of_region_game = NUM_OF_REGION_TEAM-1;//num_of_region_game 15
      int round = 1;
      int num_of_round = NUM_ROUNDS - 2;//num_of_round = 6-2, 2 is for final rounds
      //cout << num_of_round<< endl;
      getline(ipick, line);
      stringstream ssline(line);
      for (int j = 0; j < num_of_region_game; ++j){
        if (j > num_of_region_game - POW2[num_of_round-round])
          round++;
        int pick, index;
        ssline >> pick;
        index = IndexTeam ((REGION_SYMBOL) i, pick);
        //player.set_matrix_element(y_off+pick, round);
        player.set_matrix_element(index, round);
        if (_outcome.get_matrix_element(index, _completed_round))
          player.get_non_zero_entries(round-1).push_back(index);
        int cur_round_score = player.get_ith_round_score(round-1);
        if (_outcome.get_matrix_element(index, round)) {
          player.set_ith_round_score(round-1, cur_round_score + POINTS[round-1]);
          player.set_total_score(player.get_total_score() + POINTS[round-1]);
        }
      }
    }
    // read semi-final and final picks
    string semi_final1, semi_final2, final;
    getline(ipick, line);
    stringstream ssline(line);
    ssline >> semi_final1 >> semi_final2 >> final;
    string semi_final1_region, semi_final1_team, semi_final2_region, semi_final2_team, final_region, final_team;
    semi_final1_region = semi_final1[0];
    semi_final1_team = semi_final1.substr(1);
    semi_final2_region = semi_final2[0];
    semi_final2_team = semi_final2.substr(1);
    final_region = final[0];
    final_team = final.substr(1);
    assert (semi_final1_region == "M" || semi_final1_region == "W");
    assert (semi_final2_region == "S" || semi_final2_region == "E");
    int semi_final1_y = IndexTeam (semi_final1_region, atoi(semi_final1_team.c_str()));
    player.set_matrix_element(semi_final1_y, NUM_ROUNDS-1);
    if (_outcome.get_matrix_element(semi_final1_y, _completed_round))
      player.get_non_zero_entries(NUM_ROUNDS-2).push_back(semi_final1_y);
    int cur_round_score = player.get_ith_round_score(NUM_ROUNDS-2);
    if (_outcome.get_matrix_element(semi_final1_y, NUM_ROUNDS-1)) {
      player.set_ith_round_score(NUM_ROUNDS-2, cur_round_score + POINTS[NUM_ROUNDS-2]);
      player.set_total_score(player.get_total_score() + POINTS[NUM_ROUNDS-2]);
    }
    int semi_final2_y = IndexTeam (semi_final2_region, atoi(semi_final2_team.c_str()));
    player.set_matrix_element(semi_final2_y, NUM_ROUNDS-1);
    if (_outcome.get_matrix_element(semi_final2_y, _completed_round))
      player.get_non_zero_entries(NUM_ROUNDS-2).push_back(semi_final2_y);
    cur_round_score = player.get_ith_round_score(NUM_ROUNDS-2);
    if (_outcome.get_matrix_element(semi_final2_y, NUM_ROUNDS-1)) {
      player.set_ith_round_score(NUM_ROUNDS-2, cur_round_score + POINTS[NUM_ROUNDS-2]);
      player.set_total_score(player.get_total_score() + POINTS[NUM_ROUNDS-2]);
    }
    int final_y = IndexTeam (final_region, atoi(final_team.c_str()));
    player.set_matrix_element(final_y, NUM_ROUNDS);
    if (_outcome.get_matrix_element(final_y, _completed_round))
      player.get_non_zero_entries(NUM_ROUNDS-1).push_back(final_y);
    cur_round_score = player.get_ith_round_score(NUM_ROUNDS-1);
    if (_outcome.get_matrix_element(final_y, NUM_ROUNDS)) {
      player.set_ith_round_score(NUM_ROUNDS-1, cur_round_score + POINTS[NUM_ROUNDS-1]);
      player.set_total_score(player.get_total_score() + POINTS[NUM_ROUNDS-1]);
    }
    _players.push_back(player);
  }
  timer2.stoptimer();
  cout << "Reading player runtime is " << timer2.dtime() << endl;

  timer3.starttimer();
  // Sort all players based on their current score
  std::sort(_players.begin(), _players.end());
  timer3.stoptimer();
  cout << "Sorting player runtime is " << timer3.dtime() << endl;

  _max_player_name_length = 0;
  for (size_t i = 0; i < _players.size(); i ++) {
    int temp_name_length = _players[i].get_name().length();
    _max_player_name_length = (temp_name_length > _max_player_name_length) ? temp_name_length : _max_player_name_length;
  }
  if (_max_player_name_length < 6)
    _max_player_name_length = 6;

  if (_debug_level >= DETAIL){
    for (size_t p1 = 0; p1 < _players.size(); p1++) {
      cout << _players[p1].get_name() << endl;
      for (size_t round = 0; round < NUM_ROUNDS; round ++) {
        for (list<short>::iterator team = _players[p1].get_non_zero_entries(round).begin(); team !=  _players[p1].get_non_zero_entries(round).end(); team ++)
          cout << *team << ' ';
        cout << endl;
      }
      cout << endl;
    }
  }


  iteam.close();
  ipick.close();
  ioutcome.close();
}

//---------------------------------------------------------------------------------------------------- 
//PrintPickMatrix
//return nothing
//The function will print the matrix with region and team information
//This can be used in Dump for each player and for diff matrix
//---------------------------------------------------------------------------------------------------- 
template <class T>
void LogicSimulate::PrintPickMatrix(const Matrix<T>& matrix) const{
  int team_seeding_length = 3;
  int team_name_length = 15;
  int element_length = 3;
  for (size_t y = 1; y <= matrix.get_y_size(); ++y){
    Region region = calculate_region(y);
    Team team = calculate_team(y);
    cout << setiosflags(ios::left);
    //cout M,W,S,E first; reserve 3 length and cout the seeding number; reserve 15 length and cout team name
    cout << region.read_region_symbol()<< setw(team_seeding_length)<< team.get_seeding()<< setw(team_name_length)<< team.get_name();
    for (size_t x = 1; x <= matrix.get_x_size(); ++x){
      if (x == matrix.get_x_size())
        cout << matrix.get_element(y,x);
      else
        cout << setw(element_length)<< matrix.get_element(y,x);
    }
    cout << endl;
  }
}

//---------------------------------------------------------------------------------------------------- 
//CompareTwo
//player1 and player2 are two players
//return the min terms that player1 can win player2
//Find string minterm by FindMintermOpt first, then ApplyCombineRule to optimize this minterm
//---------------------------------------------------------------------------------------------------- 
SOP LogicSimulate::CompareTwo (const Player& player1, const Player& player2)const{
  Matrix<short> matrix1 = player1.get_matrix(); 
  Matrix<short> matrix2 = player2.get_matrix(); 
  Matrix<short> orig_diff = player1.get_matrix() - player2.get_matrix(); 
  assert (orig_diff.Sum() == 0);

  Matrix<ELEMENT> diff(orig_diff.get_y_size(), orig_diff.get_x_size());
  diff.Initialize (orig_diff);
  if (_debug_level >= DETAIL){
    cout << "diff matrix is:"<< endl;
    PrintPickMatrix(diff);
    cout<<endl;
  }

  Matrix<short> mask = _outcome.get_matrix();
  diff.Masked(mask, _completed_round);
  if (_debug_level >= DETAIL){
    cout << "masked diff matrix is:"<< endl;
    PrintPickMatrix(diff);
  }

  //begin finding min terms
  int points = 0, start_round = diff.get_x_size(), start_team = 1, end_team = diff.get_y_size();
  //end_team = 50;
  //end_team = diff.get_y_size();
  MintermHelper minterm_helper(this);
  bool find;
  vector<string> minterms(0);
  //preparation for calling FindMintermOpt
  vector<int> max_points(diff.get_x_size(), 0); 
  vector<int> min_points(diff.get_x_size(), 0); 
  GenerateMaxMinPoints(diff, max_points, min_points);
  if (_debug_level >= DETAIL){
    cout<< "max_points is:";
    for (size_t i = 0; i<max_points.size(); ++i)
      cout << max_points[i]<< " ";
    cout << endl;
    cout<< "min_points is:";
    for (size_t i = 0; i<min_points.size(); ++i)
      cout << min_points[i]<< " ";
    cout << endl;
  }
  //find = minterm_helper.FindMinterm(diff, points, start_round, start_team, completed_round, end_team, minterms);
  find = minterm_helper.FindMintermOpt(diff, max_points, min_points, points, start_round, start_team, _completed_round+1, end_team, minterms);
  if (_debug_level >= DETAIL){
    cout << "pre-earned points is "<< points<< endl;
    cout<< "minterms size is "<< minterms.size()<< endl;
  }
  //change minterms to products
  SOP products(minterms);
  //optimize
  if (_optimization){
    while (minterm_helper.ApplyCombineRule(products)){}
  }

  //return minterms;
  return products;
}


//---------------------------------------------------------------------------------------------------- 
//And
//And all products
//return a new product 
//---------------------------------------------------------------------------------------------------- 
SOP LogicSimulate::And (const vector<SOP>& products)const{
  MintermHelper minterm_helper(this);
  vector<SOP> remained_products = products;
  //we want multiply until one product remained
  while (remained_products.size() > 1){
    SOP vp1 = remained_products.back();
    remained_products.pop_back();
    SOP vp2 = remained_products.back();
    remained_products.pop_back();
    SOP vp12;
    if (vp1.Size() == 0){
      remained_products.push_back(vp2);
      continue;
    }
    if (vp2.Size() == 0){
      remained_products.push_back(vp1);
      continue;
    }

    for (size_t i=0; i<vp1.Size(); ++i){
      for (size_t j=0; j<vp2.Size(); ++j){
        Product p = vp1[i]*vp2[j];
        vp12.add_product(p);
      }
    }
    if (_debug_level >= DETAIL){
      cout << vp1.get_string();
      cout << " times "<< vp2.get_string();
      cout<< " is "<< vp12.get_string()<< endl;
    }
    if (_optimization)
      while (minterm_helper.ApplyCombineRule(vp12)){}
    if (_debug_level >= DETAIL){
      cout << "After optimization product is: "<< vp12.get_string() << endl;
    }
    remained_products.push_back(vp12);
  }
  SOP remained_one = remained_products.back();
  minterm_helper.ApplyReverseRule(remained_one);
  while (minterm_helper.ApplyCombineRule(remained_one)){}
  if (_debug_level >= DETAIL){
    cout << "multiplied un-optimized product is: "<< remained_one.get_string() << endl;
  }
  return remained_one;
}
//---------------------------------------------------------------------------------------------------- 
//PrintPlayerInfo
//return nothing
//The function will print 
//each player with player name; team region, team name, picks for all rounds for each team
//---------------------------------------------------------------------------------------------------- 
void LogicSimulate::PrintPlayerInfo() const
{
  //cout << "total sources " << _sourceNodesIndex.size() << endl;
  //cout << "total sinks " << _sinkNodesIndex.size() << endl;
  if (get_debug_level() >= DETAIL) {
    Matrix<short> mask = _outcome.get_matrix();
    cout << _outcome.get_name()<< endl;
    PrintPickMatrix(mask);
    cout<< endl;
  }
  cout << "Standing after Round " << _completed_round << endl;
  cout << "Rank" << setw(2) << ' ';
  cout << "Name" << setw(_max_player_name_length - 3) << ' ';
  cout << " R64  R32  S16   E8   F4  NCG  Total  PPR Alv  Champion" << endl;
  int pre_score = 0; int pre_rank = 0;
  int number_players_alive = 0;
  for (size_t i = 0; i < _players.size(); ++i) {
    if (_players[i].get_total_score() == pre_score)
      cout << setw(4) << pre_rank << "  ";
    else {
      cout << setw(4) << i + 1 << "  ";
      pre_rank = i + 1;
    }
    _players[i].PrintScore(_max_player_name_length);
    const Matrix<short> & matrix = _players[i].get_matrix();
    for (size_t row = 0; row < matrix.get_y_size(); row++) {
      if (matrix.get_element(row+1, 6)) {
        if (_outcome.get_matrix().get_element(row+1, _completed_round))
          cout << " * " << calculate_team(row+1).get_name();
        else
          cout << " X " << calculate_team(row+1).get_name();
      }
    }
    cout << endl;
    pre_score = _players[i].get_total_score();
    if (get_debug_level() >= DETAIL) {
      Matrix<short> matrix = _players[i].get_matrix();
      //output the whole matrix for the player, according to region and team on y axis
      PrintPickMatrix(matrix);
      cout<< endl;
    }
    if (_players[i].get_alive()) number_players_alive ++;
      //cout<< "player " <<_players[i].get_name()<< " is still alive\n";}
  }
  cout << "Total alive players " << number_players_alive << endl;
}

//---------------------------------------------------------------------------------------------------- 
//IndexTeam
//return the matrix index of the team
//r is the region
//seeding is the seeding of the team
//---------------------------------------------------------------------------------------------------- 
int LogicSimulate::IndexTeam (REGION_SYMBOL r, int seeding)const{
  return (int)r * NUM_OF_REGION_TEAM + REV_SEED[seeding-1];
}

//---------------------------------------------------------------------------------------------------- 
//IndexTeam
//return the matrix index of the team
//r is the region name
//seeding is the seeding of the team
//---------------------------------------------------------------------------------------------------- 
int LogicSimulate::IndexTeam (string r, int seeding)const{
  if (r == "M")
    return IndexTeam(M, seeding);
  else if (r == "W")
    return IndexTeam(W, seeding);
  else if (r == "S")
    return IndexTeam(S, seeding);
  else if (r == "E")
    return IndexTeam(E, seeding);
  else{
    assert (true);//should not be the case
    return -1;
  }
}


// draw picture
void LogicSimulate::drawXgraph(const char * filename, const char * name) const
{
  /*
     ofstream fp(filename);
     fp << "TitleText: " << name << " Graph with " << _nodes.size() << " nodes. " << " total cost is " << _totalCost;
     fp << endl;

     fp << "\"Wires\"" << endl;
     for (list<PowerEdge>::const_iterator i = _edges.begin(); i != _edges.end(); i ++) {
     fp << "move " << _nodes[(*i).from()].x() << " " << _nodes[(*i).from()].y() << endl;
     fp << _nodes[(*i).to()].x() << " " << _nodes[(*i).to()].y() << endl;
     }
     fp << endl;

     fp << "\"Plants\"" << endl;
     for (size_t i = 0; i < _nodes.size(); i ++) {
     if (_nodes[i].type() == PLANT) {
     fp << "move " << _nodes[i].x() << " " << _nodes[i].y() - 0.5 << endl;
     fp << _nodes[i].x() << " " << _nodes[i].y() + 0.5 << endl;
     fp << _nodes[i].x() + 0.5 << " " << _nodes[i].y() + 0.5 << endl;
     fp << _nodes[i].x() + 0.5 << " " << _nodes[i].y() - 0.5 << endl;
     fp << _nodes[i].x() << " " << _nodes[i].y() - 0.5 << endl;
     }
     }
     fp << endl;

     fp << "\"Cities\"" << endl;
     for (size_t i = 0; i < _nodes.size(); i ++) {
     if (_nodes[i].type() == CITY) {
     fp << "move " << _nodes[i].x() << " " << _nodes[i].y() - 0.5 << endl;
     fp << _nodes[i].x() << " " << _nodes[i].y() + 0.5 << endl;
     fp << _nodes[i].x() + 0.5 << " " << _nodes[i].y() + 0.5 << endl;
     fp << _nodes[i].x() + 0.5 << " " << _nodes[i].y() - 0.5 << endl;
     fp << _nodes[i].x() << " " << _nodes[i].y() - 0.5 << endl;
     }
     }
     fp << endl;

     fp.close();
     char line[1052];
     sprintf(line,"xgraph -bb -tk -bg white -P %s &", filename);
     system(line);
   */
}

// draw picture
void LogicSimulate::drawTcl(char * filename) const
{
  /*
     ofstream fp(filename);
     fp << "%% description of the network." << endl;
     fp << "%% format:" << endl;
     fp << "%%	\"source\" name x y capacity (capacity is +ve)" << endl;
     fp << "%%	\"sink\" name x y capacity (capacity is -ve)" << endl;
     fp << "%%	\"point\" name x y" << endl;
     fp << "%%	\"type\" name cost (K Euro) capacity (MW)" << endl;
     fp << "%%	\"wire\" from to type" << endl;
     fp << "%% wire types (as specified in the document)" << endl;

     fp << "type	VHT	1000	1000" << endl;
     fp << "type	UGHT	1000	300" << endl;
     fp << "type	HT	300	1000" << endl;

     fp << "%% this the blockage (as specified in the document)" << endl;
     fp << "block	0	0	200	200" << endl;
     fp << "block	0	700	350	1000" << endl;

     fp << "%% these are the generators (as specified in the document)" << endl;
     for (size_t i = 0; i < _nodes.size(); i ++) {
     if (_nodes[i].type() == PLANT) {
     fp << "source " << i + 1 << "\t" << _nodes[i].x() << "\t" << _nodes[i].y() << "\t" << _nodes[i].power() << endl;
     }
     }

     fp << "%% these are the cities (as specified in the document)" << endl;
     for (size_t i = 0; i < _nodes.size(); i ++) {
     if (_nodes[i].type() == CITY) {
     fp << "sink " << i + 1 << "\t" << _nodes[i].x() << "\t" << _nodes[i].y() << "\t" << _nodes[i].power() << endl;
     }
     }

     fp << "%% these are the transformers (as specified in the document)" << endl;
     for (size_t i = 0; i < _nodes.size(); i ++) {
     if (_nodes[i].type() == TRANSFORMER) {
     fp << "tran " << i + 1 << "\t" << _nodes[i].x() << "\t" << _nodes[i].y() << "\t" << _nodes[i].power() << endl;
     }
     }

     fp << "%% these are the steiners (as specified in the document)" << endl;
     for (size_t i = 0; i < _nodes.size(); i ++) {
     if (_nodes[i].type() == STEINER) {
     fp << "steiner " << i + 1 << "\t" << _nodes[i].x() << "\t" << _nodes[i].y() << "\t" << _nodes[i].power() << endl;
     }
     }

     char *wire_type_string[3] = {"VHT", "UGHT", "HT"};
     fp << "%% Wires" << endl;
     for (list<PowerEdge>::const_iterator i = _edges.begin(); i != _edges.end(); i ++) {
     fp << "wire " << (*i).from() + 1 << "\t" << (*i).to() + 1 << "\t" << wire_type_string[(*i).type()] << "\t" << (*i).width() << endl;
     }
     fp << endl;

     fp.close();
     char line[1052];
     sprintf(line,"wish view.tcl %s &", filename);
     system(line);
   */
}


//---------------------------------------------------------------------------------------------------- 
//GenerateMaxMinPoints
//According to diff matrix, generate max_points and min_points
//---------------------------------------------------------------------------------------------------- 
void LogicSimulate::GenerateMaxMinPoints(const Matrix<ELEMENT>& diff, vector<int>& max_points, vector<int>& min_points)const{
  for (size_t x = 1; x <= diff.get_x_size(); ++x){
    for (size_t y = 1; y <= diff.get_y_size(); ++y){
      if (diff.get_element(y,x) == WIN){
        max_points[x-1] += POINTS[x-1];
      }else if (diff.get_element(y,x) == MUST_WIN){
        max_points[x-1] += POINTS[x-1];
        min_points[x-1] += POINTS[x-1];
      }else if (diff.get_element(y,x) == LOSE){
        min_points[x-1] -= POINTS[x-1];
      }else if (diff.get_element(y,x) == MUST_LOSE){
        min_points[x-1] -= POINTS[x-1];
        max_points[x-1] -= POINTS[x-1];
      }
    }
  }
}
