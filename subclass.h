#ifndef SUBCLASS_H
#define SUBCLASS_H
#include "constants.h"

using namespace std;
template <class T>
class Matrix{
  vector<vector<T> > _data;
  public:
  // Constructor
  Matrix (int team_size, int round_size){
    _data.resize(team_size, vector<T>(round_size, T(0)));
/*     _data.clear(); */
/*     for (int y = 1; y <= y_size; ++y){ */
/*       vector<T> v(x_size, T(0)); */
/*       _data.push_back(v); */
/*     } */
  }
  void Initialize (const Matrix<short>& base);

  // useful functions
  void Print() const{
    for (size_t y = 1; y <= _data.size(); ++y){
      for (size_t x = 1; x <= _data[0].size(); ++x){
        cout << _data[y-1][x-1];
        if (x != _data[0].size())
          cout << "\t";
      }
      cout << endl;
    }
  }
  void Clear(){
    for (size_t y = 1; y <= _data.size(); ++y){
      for (size_t x = 1; x <= _data[0].size(); ++x){
        set_element(y,x,T(0));
      }
    }
  }
  
  short Sum() const;
  short SumRow() const;
  
  // operator overload
  Matrix<T> operator - (const Matrix<T>& other)const;

  // Mask one matrix to final output matrix
  void Masked(const Matrix<short>& mask, const int& end_round);

  // set and get functions
  void set_element (int y, int x, T value){
    assert (y>=1 && (size_t)y <= _data.size());
    assert (_data.size() > 0);
    assert (x>=1 && (size_t)x <= _data[0].size());
    _data[y-1][x-1] = value;
  }
  T get_element(int y, int x)const{
/*     assert (y>=1 && (size_t)y <= _data.size()); */
/*     assert (_data.size() > 0); */
/*     assert (x>=1 && (size_t)x <= _data[0].size()); */
    return _data[y-1][x-1];
  }
  size_t get_y_size()const {return _data.size();}
  size_t get_x_size()const {
    if (get_y_size() == 0)
      return 0;
    return _data[0].size();
  }
  vector<vector<T> > get_data(){return _data;}

};
class Variable{
  REGION_SYMBOL _region;
  int _seeding;
  int _round;//1~6
  bool _bar;

  int _index;//1~64
  public:
  Variable (){}
  Variable (const string & name){
    size_t p1 = name.find("_");
    string region_seeding = name.substr(0, p1);
    size_t p2 = region_seeding.find_first_of("0123456789");
    string region = region_seeding.substr(0, p2);
    if (region == "M"){
      _region = M;
    }else if (region == "W"){
      _region = W;
    }else if (region == "S"){
      _region = S;
    }else if (region == "E"){
      _region = E;
    }else{
      assert (1);
    }
    _seeding = atoi(region_seeding.substr(p2).c_str());
    _index = (int)_region * NUM_OF_REGION_TEAM + REV_SEED[_seeding-1]; 

    string round_bar = name.substr(p1+1);
    assert (round_bar.length() > 0);
    if (round_bar[round_bar.length()-1] == '\''){
      _round =  atoi(round_bar.substr(0, round_bar.length()-1).c_str());
      _bar = true;
    }else{
      _round =  atoi(round_bar.c_str());
      _bar = false;
    }
  
  }
  Variable (int index, int round, bool bar){
    _round = round;
    _bar = bar;
    _index = index;
    int region = (index-1)/NUM_OF_REGION_TEAM;
    _region = (REGION_SYMBOL) region;
    _seeding = SEED[index-(int)_region*NUM_OF_REGION_TEAM-1];
  }
  void Negate(){
    _bar = !_bar;
  }
  bool IsSame(const Variable& other)const{
    return _region==other._region && _seeding==other._seeding && _round==other._round && _bar==other._bar;
//    return _name == other._name;
  }

  // set and get functions
  REGION_SYMBOL get_region()const{return _region;}
  int get_seeding()const{return _seeding;}
  int get_round()const{return _round;}
  int get_index()const{return _index;}
  bool IsBar()const{return _bar;}

  string to_string()const{
    string name = "";
    if (_region == M){
      name += "M";
    }else if (_region == W){
      name += "W";
    }else if (_region == S){
      name += "S";
    }else if (_region == E){
      name += "E";
    }
    std::stringstream ss;
    ss << _seeding;
    name += ss.str();
    name += "_";
    ss.str(std::string());//standard 2-step clean of ss
    ss.clear();
    ss << _round;
    name += ss.str();
    if (_bar)
      name += "'";
    return name;
  }
  /*
  string get_name()const{return _name;}
  string get_team()const{
    return _team;
  }
  REGION_SYMBOL get_region()const{
    string team = get_team();
    size_t p = team.find_first_of("0123456789");
    string region = team.substr(0, p);
    if (region == "M"){
      return M;
    }else if (region == "W"){
      return W;
    }else if (region == "S"){
      return S;
    }else if (region == "E"){
      return E;
    }else{
      assert (1);
    }
  }
  int get_seeding()const{
    string team = get_team();
    size_t p = team.find_first_of("0123456789");
    return atoi(team.substr(p).c_str());
  }
  int get_round()const{
    string name = _name;
    if (IsBar())
      name = name.substr(0, name.length()-1);
    size_t p = _name.find("_");
    return atoi(name.substr(p+1).c_str());
  }*/
  //W1_6 < W1_6'< W1_5 < W1_5'
  bool operator < (const Variable& rhs)const{
    if (_round > rhs._round)  return true;
    if (_round < rhs._round)  return false;
    //then round is same
    if (_index < rhs._index)
      return true;
    if (_index > rhs._index)
      return false;
    //then round, region, seeding are all same
    if (rhs.IsBar())  return true;
    return false;
  }
};
//Product is the product of Variables
class Product{
  protected://This means private, but can be inherited
    vector<Variable> _variables;
    //string _points;
  public:
    // Constructor
  Product(){
    _variables = vector<Variable> (0);
    //_points = -1;
  }
  /*
  Product(const Product& other){
    _variables = other._variables;
  }*/
  Product(const string& one_term){
    size_t start = 0;
    size_t find_blank = one_term.find(" ");
    while (find_blank != string::npos){
      Variable variable (one_term.substr(start, find_blank - start));
      _variables.push_back(variable);
      start = find_blank + 1;
      find_blank = one_term.find(" ", start);
    }
    //_points = one_term.substr(start);
    //the last one is the points, like 320
  }

  Product operator * (const Product& rhs)const{
    Product p = *this;
    for (size_t i=0; i<rhs._variables.size(); ++i){
      Variable new_variable = rhs._variables[i];
      if (!p.Contain(new_variable))
        p.add_variable(new_variable);
    }
    return p;
  }
  // useful functions
  int NumVariables()const{return _variables.size();}
  //if fully contain another product
  bool Contain(const Variable& variable)const{
    for (size_t i=0; i<_variables.size(); ++i){
      if (_variables[i].IsSame(variable))
        return true;
    }
    return false;
  }
  //must ensure this has that variable before calling this function
  void Remove(const Variable& variable){
    vector<Variable> variables_copy(0);
    for (size_t i=0; i<_variables.size(); ++i){
      if (!_variables[i].IsSame(variable)){ 
        variables_copy.push_back(_variables[i]); 
      }
    } 
    assert (_variables.size() != variables_copy.size());
    _variables = variables_copy;
  }
  /*
  bool Contain(const Product& other)const{
    vector<string> other_variables = other._variables;
    for (size_t i=0; i<_variables.size(); ++i){
      vector<string> other_variables_copy(0);
      for (size_t j=0; j<other_variables.size(); ++j){
        if (_variables[i] != other_variables[j]){
          other_variables_copy.push_back(other_variables[j]); 
        }
      }
      if (other_variables_copy.size() < other_variables.size())//means this->ith variable is same with certain variable in other, and this variable is removed from other
        other_variables = other_variables_copy;
    } 
    return other_variables.empty();
  }
  //remove another product from this
  void Remove(const Product& other){
    vector<string> other_variables = other._variables;
    vector<string> variables_copy(0);
    for (size_t i=0; i<_variables.size(); ++i){
      vector<string> other_variables_copy(0);
      for (size_t j=0; j<other_variables.size(); ++j){
        if (_variables[i] != other_variables[j]){
          other_variables_copy.push_back(other_variables[j]); 
        }
      }
      if (other_variables_copy.size() < other_variables.size()){//means this->ith variable is same with certain variable in other, and this variable is removed from other
        other_variables = other_variables_copy;
      }else{
        variables_copy.push_back(_variables[i]); 
      }
    } 
    assert(other_variables.empty());//make sure other is completely removed
    _variables = variables_copy;
  }
  */
  void Sort(){
    sort(_variables.begin(), _variables.end());
  }

  void Print()const{
    string s = get_string();
    cout << s<< endl;
  }

  // set and get functions
  //return the original input string
  void set_variables (const vector<Variable> & variables){_variables = variables;}
  vector<Variable> get_variables ()const{return _variables;}
  void add_variable (const Variable& variable){
    _variables.push_back(variable);
  }
  void add_variables (const vector<Variable> & variables){
    _variables.insert(_variables.end(), variables.begin(), variables.end());
  }
  //no points attached; if it is empty, then returns 1
  string get_string()const{
    string s = "";
    for (size_t i=0; i<_variables.size(); ++i){
      if (s != "")
        s += " ";
      s += _variables[i].to_string();
    }
    //s += " " + _points;
    if (s =="")
      s = "1";
    return s;
  }
  Variable get_ith_variable (int i) const{
    assert ((int)_variables.size()>=i && i>=1);
    return _variables[i-1];
  }
};

//SOP is the sum of products
class SOP{
  vector<Product> _products;
  public:
  //constructor
  SOP (){
    _products.clear();
  }
  SOP (const vector<string> & minterms){
    _products.clear();
    for (size_t i=0; i<minterms.size(); ++i){
      add_product(Product(minterms[i]));
    }
  }
  Product& operator [] (int i){
    assert (i>=0 && i<(int)Size());
    return _products[i];
  }
  string get_string()const{
    string s;
    for (size_t i=0; i<_products.size(); ++i){
      if ( i != 0)
        s += " + ";
      s += _products[i].get_string();
    }
    s += "\n";
    return s;
  }
  size_t Size()const{
    return _products.size();
  }
  void Clear(){
    _products.clear();
  }
  void Print()const{
    cout << get_string();
  }
  bool Empty()const{
    return _products.empty();
  }
  //get and set
  void add_product(const Product& product){
    _products.push_back(product);
  }
};

class Team{
  int _seeding;
  string _name;
  double _point;
  vector<double> _home_points;
  public:
  Team () {};
  Team (int seeding, string name, double point, double home_point_12, double home_point_34, double home_point_56):
  _seeding(seeding), _name(name), _point(point) {
    _home_points.resize(6);
    _home_points[0] = _home_points[1] = home_point_12;
    _home_points[2] = _home_points[3] = home_point_34;
    _home_points[4] = _home_points[5] = home_point_56;
  };
  void set_seeding(int seeding){_seeding = seeding;}
  int get_seeding()const{return _seeding;}
  void set_name(string name){_name = name;}
  string get_name()const{return _name;}
  void set_point(double point){_point = point;}
  double get_point()const{return _point;}
  void set_home_points(size_t i, double value){_home_points[i] = value;}
  const double get_home_points(size_t i)const{return _home_points[i];}
};
class Region{
  REGION_SYMBOL _region_symbol;
  vector<Team> _teams;
  string _name;
  public:
  // Constructor
  Region() {};

  // set and get functions
  void set_region_symbol(REGION_SYMBOL region_symbol){_region_symbol = region_symbol;}
  vector<Team> get_teams()const{return _teams;}
  REGION_SYMBOL get_region_symbol()const{return _region_symbol;}
  string read_region_symbol()const{
    switch (_region_symbol){
      case M: return "M";
      case W: return "W";
      case S: return "S";
      case E: return "E";
      default: {
                 assert (true);//should not be the case
                 return "";
               }
    }
  }
  void add_team (Team team) {_teams.push_back(team);}
};
class Player{
  string _name;
  Matrix<short> _matrix;
  bool _alive;
  double _pr_win;
  int _total_score;
  int _max_score;
  vector<int> _score_rounds;
  vector<SOP> _min_terms;
  vector<list<short> > _non_zero_entries;
  int _same_pick_player;
  public:
  Player():_matrix(NUM_OF_TOTAL_TEAM, NUM_ROUNDS){
    _alive = 1;
    _total_score = 0;
    _max_score = 0;
    _same_pick_player = -1;
    _score_rounds.resize(NUM_ROUNDS, 0);
    _non_zero_entries.resize(NUM_ROUNDS);
  }

  // set and get functions
  void set_name(string name){_name = name;}
  string get_name()const{return _name;}
  void set_alive(bool alive){_alive = alive;}
  bool get_alive()const{return _alive;}
  void set_pr_win(double pr_win){_pr_win = pr_win;}
  double get_pr_win()const{return _pr_win;}
  void set_ith_round_score(size_t round, int value) {_score_rounds[round] = value;}
  int get_ith_round_score(size_t round)const{return _score_rounds[round];}
  int get_total_score () const { return _total_score; }
  void set_total_score (int score) { _total_score = score; }
  int get_max_score () const { return _max_score; }
  void set_max_score (int score)  { _max_score = score; }
  int get_same_pick_player () const { return _same_pick_player; }
  list<short> & get_non_zero_entries(int round) { return _non_zero_entries[round]; }
  vector<SOP> & get_min_terms() { return _min_terms; }
  void set_matrix_element(int y, int x)//from up left corner, y and x start from 1
  {_matrix.set_element(y, x, 1);}
  int get_matrix_element(int y, int x)//from up left corner, y and x start from 1
  {return _matrix.get_element(y, x);}
  Matrix<short> get_matrix()const{return _matrix;}

  void PrintScore(int max_player_name_length) const;
  void ComputeMaxScore(const int & comlete_round);
  void PrunePlayer(Player & player, const int & comlete_round);
  void MarkSamePick(Player & player, const int & comlete_round, const int & p1_index);

  bool operator < (const Player & rhs) const
  {
    return (_total_score > rhs._total_score ) ? true : ((_total_score == rhs._total_score ) ? _max_score - _total_score > rhs._max_score - rhs._total_score : false) ;
  }



  // useful functions
};

#endif
