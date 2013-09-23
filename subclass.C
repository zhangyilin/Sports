#include "subclass.h"
//---------------------------------------------------------------------------------------------------- 
//Matrix template specialization
//This has to be instantiated before first use, therefore it is before LogicSimulate class
//---------------------------------------------------------------------------------------------------- 
template <> short Matrix<short>::Sum() const{
  short sum = 0;
  for (size_t y = 1; y <= _data.size(); ++y){
    for (size_t x = 1; x <= _data[0].size(); ++x){
      sum += _data[y-1][x-1];
    }
  }
  return sum;
}


// operator overload
template <>  void Matrix<ELEMENT>::Initialize (const Matrix<short>& base){
  Matrix<ELEMENT> diff(base.get_y_size(), base.get_x_size());
  for (size_t x = 1; x <= base.get_x_size(); ++x){
    for (size_t y = 1; y <= base.get_y_size(); ++y){
      if (base.get_element(y,x) == 1){
        set_element(y,x,WIN);
      }else if (base.get_element(y,x) == -1){
        set_element(y,x,LOSE);
      }else{
        assert (base.get_element(y,x) == 0);
        set_element(y,x,ZERO);
      }
    }
  }
}
template <> Matrix<short> Matrix<short>::operator - (const Matrix<short>& other)const{
  assert (_data.size() == other._data.size());
  if (_data.size() == 0)
    return Matrix<short>(0, 0);
  Matrix<short> result(_data.size(), _data[0].size());
  for (size_t y = 1; y <= _data.size(); ++y){
    for (size_t x = 1; x <= _data[0].size(); ++x){
      result.set_element(y, x, get_element(y, x)-other.get_element(y,x));
    }
  }
  return result;
}
//mask only specialized for Matrix<ELEMENT>
//only mask from column 1 to end_round, then propogate from end_round+1 to the last column
template <> void Matrix<ELEMENT>::Masked(const Matrix<short>& mask, const int& end_round){
  assert (get_x_size() == mask.get_x_size()); 
  assert (get_y_size() == mask.get_y_size()); 
  for (size_t y = 1; y <= _data.size(); ++y){
    assert ((size_t)end_round <= _data[0].size());
    for (int x = 1; x<=end_round; ++x){
      //if (get_element(y,x)!=ZERO && mask.get_element(y,x)==0)
      if (mask.get_element(y,x)==0){
        set_element (y,x,ZERO);
        //propagate 0 to the end since this team can not win in the later round
        if (x==end_round){
          for (size_t i=end_round+1; i<=_data[0].size(); ++i)
            set_element (y,i,ZERO);
        }
      }
      if (get_element(y,x)==WIN && mask.get_element(y,x)==1)
        set_element (y,x,MUST_WIN);
      if (get_element(y,x)==LOSE && mask.get_element(y,x)==1)//mask 1 means this team must win which means player1 must lose which means player1 must lose
        set_element (y,x,MUST_LOSE);
    }
  }
}

//---------------------------------------------------------------------------------------------------- 
//PrintScore
//The function will print 
//each player with player name; team region, team name, picks for all rounds for each team
//---------------------------------------------------------------------------------------------------- 
void Player::PrintScore(int max_player_name_length) const
{
  // make sure at least one space is added
  cout << get_name() << setw(max_player_name_length - get_name().length() + 1) << ' ';
  // 7 = "Round 1"
  for (size_t i = 0; i < _score_rounds.size(); i ++) {
    cout << setw(4) << _score_rounds[i] << ' ';
  }
  // 11 = "Total Score"
  cout << setw(6) << _total_score << ' ';
  cout << setw(4) << _max_score - _total_score << ' ';
  if (_alive)
    cout << "  Y  ";
  else
    cout << "  N  ";
}

//---------------------------------------------------------------------------------------------------- 
//ComputeMaxScore
//The function will compute the max possible score for each player
//---------------------------------------------------------------------------------------------------- 
void Player::ComputeMaxScore(const int & complete_round)
{
  _max_score = _total_score;
  for (int round = complete_round+1; round <= NUM_ROUNDS; ++round ) {
    //    for (size_t team = 0; team < _matrix.get_data().size(); ++team){
    //      if (_matrix.get_element(team+1, round) && outcome.get_matrix().get_element(team+1, complete_round)) {
    for (list<short>::iterator team_iter = _non_zero_entries[round-1].begin(); team_iter != _non_zero_entries[round-1].end(); team_iter++) {
      if (_matrix.get_element(*team_iter, round)) {
        _max_score += POINTS[round-1];
      }
    }
  }
}

//---------------------------------------------------------------------------------------------------- 
//MarkSamePick
//The function will mark p2 if p2 has same pick as p1. P1's total_score should be larger or equal to p2
//---------------------------------------------------------------------------------------------------- 
void Player::MarkSamePick(Player & p2, const int & complete_round, const int & p1_index)
{
  //  if (!p2._alive || p2._same_pick_player > -1)
  if (!p2._alive)
    return;
  for (int round = NUM_ROUNDS; round > complete_round; --round )
    if (_non_zero_entries[round-1].size() != p2._non_zero_entries[round-1].size())
      return;
  for (int round = NUM_ROUNDS; round > complete_round; --round )
    for (list<short>::iterator p1_team_iter = _non_zero_entries[round-1].begin(), p2_team_iter = p2._non_zero_entries[round-1].begin(); p1_team_iter != _non_zero_entries[round-1].end(), p2_team_iter != p2._non_zero_entries[round-1].end(); p1_team_iter++, p2_team_iter++)
      if (*p1_team_iter != *p2_team_iter )
        return;
//   p2._same_pick_player = p1_index;
}

//---------------------------------------------------------------------------------------------------- 
//PrunePlayer
//The function will compute the max possible score for each player
// p2 needs to have less total_score than current player (p1)
//---------------------------------------------------------------------------------------------------- 
void Player::PrunePlayer(Player & p2, const int & complete_round)
{
//   if (_total_score <= p2._total_score || !p2._alive || p2._same_pick_player > -1)
  if (_total_score <= p2._total_score || !p2._alive)
    return;
  int max_diff_score = 0;
  int current_diff_score = _total_score - p2._total_score;
  //  for (int round = complete_round+1; round <= NUM_ROUNDS; ++round ) {
  for (int round = NUM_ROUNDS; round > complete_round; --round ) {
    for (list<short>::iterator team_iter = (p2._non_zero_entries[round-1]).begin(); team_iter != (p2._non_zero_entries[round-1]).end(); team_iter++) {
      if (_matrix.get_element(*team_iter, round) == 0) {
        max_diff_score += POINTS[round-1];
        if (max_diff_score >= current_diff_score)
          break;
      }
    }
    if (max_diff_score >= current_diff_score)
      break;
  }
  if (current_diff_score > max_diff_score) {
//     cout << "Player " << _name << " prune " << p2._name << ". p1 total: " << _total_score << ", p2 total: " << p2._total_score << ", p2 max diff: " << max_diff_score << endl;
    p2._alive = 0;
  }
}
