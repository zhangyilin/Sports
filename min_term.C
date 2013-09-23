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
#include "timer.h"
#include "constants.h"
#include "min_term.h"

//----------------------------------------------
// STL Include Library
//----------------------------------------------
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>  //std::accumulate
using namespace std;


/*
//---------------------------------------------------------------------------------------------------- 
//CalculateTakeBound
//will generate start_round and start_team if the block of element (y,x) should be skipped
//---------------------------------------------------------------------------------------------------- 
void MintermHelper::CalculateTakeBound(const int& x, const int& y, const int& block_tail, const int& y_end, const int& end_round, int& take_start_round, int& take_start_team)const{
//generate the start and end for sub call
if (block_tail >= y_end){
if (x == end_round){//all finished 
take_start_round = -1;
take_start_team = 1;//don't care
}else{
take_start_round = x-1;
take_start_team = 1;
}
}else{
take_start_round = x;
take_start_team = block_tail+1;
}
}
 */
//---------------------------------------------------------------------------------------------------- 
//CalculateSubBound
//will generate start_round and start_team if only skipping one element (y,x)
//---------------------------------------------------------------------------------------------------- 
void MintermHelper::CalculateSubcallBound(const int& x, const int& y, const int& y_end, const int& end_round, int& not_take_start_round, int&not_take_start_team)const{
  if ( y >= y_end){
    if (x == end_round){//all finished 
      not_take_start_round = -1;
      not_take_start_team = 1;//don't care
    }else{
      not_take_start_round = x-1;
      not_take_start_team = 1;
    }
  }else{
    not_take_start_round = x;
    not_take_start_team = y+1;
  }
}
//---------------------------------------------------------------------------------------------------- 
//Apply
//work for FindMinterm, is basically same with ApplyExclusiveRule
//will set (y,x) to MUST_(y,x); set other elements in the exclusive block to ZERO
//---------------------------------------------------------------------------------------------------- 
void MintermHelper::Apply(const int& x, const int& y, Matrix<ELEMENT>& take)const{
  bool y_x_win = take.get_element(y,x)==MUST_WIN;
  //calculate the size and range of exclusive block(it is a vector)
  int size_exclusive = int(pow(2,x));
  int block_head = int(floor((y-1)/size_exclusive)*size_exclusive) + 1;
  int block_tail = block_head + size_exclusive - 1;
  //elements in exclusive block will be reset
  for (int k = block_head; k <= block_tail; ++k){
    //if this element needs reset
    if ((take.get_element(k, x)==LOSE && y_x_win)||
        (take.get_element(k, x) == WIN && !y_x_win)){
      take.set_element(k, x, ZERO);
    }
  }
  //not really useful
}
//---------------------------------------------------------------------------------------------------- 
//ApplyExclusiveRule
//work for FindMintermOpt
//will set (y,x) to MUST_(y,x); set other elements in the exclusive block to ZERO
//elements in exclusive block will be reset, with take_max_points, take_min_points updated
//---------------------------------------------------------------------------------------------------- 
void MintermHelper::ApplyExclusiveRule(const int& x, const int& y, Matrix<ELEMENT>& take, vector<int>& take_max_points, vector<int>& take_min_points)const{
  assert (take.get_element(y,x)==MUST_WIN || take.get_element(y,x)==MUST_LOSE);
  bool y_x_win = take.get_element(y,x)==MUST_WIN;
  //calculate the size and range of exclusive block(it is a vector)
  int size_exclusive = int(pow(2,x));
  int block_head = int(floor((y-1)/size_exclusive)*size_exclusive) + 1;
  int block_tail = block_head + size_exclusive - 1;
  for (int k = block_head; k <= block_tail; ++k){
    if (k == y)
      continue;
    assert (take.get_element(k, x)!=MUST_LOSE && take.get_element(k, x)!=MUST_WIN);
    if (take.get_element(k, x) == LOSE && y_x_win){
      take.set_element(k, x, ZERO);
      take_min_points[x-1] += POINTS[x-1];
    }
    if (take.get_element(k, x) == WIN && !y_x_win){
      take.set_element(k, x, ZERO);
      take_max_points[x-1] -= POINTS[x-1];
    }
  }
}
//---------------------------------------------------------------------------------------------------- 
//ApplyReverseRegressiveRule
//take_max_points(take_min_points) is the unknown point that we can maximally(minimally) get for each round, besides the points we know
//Once one element is set MUST_WIN MUST_LOSE, the results from previous rounds will also be set
//M1_5 M1_6 = M1_6
//---------------------------------------------------------------------------------------------------- 
void MintermHelper::ApplyReverseRegressiveRule(const int& x, const int& y, const int& end_round, Matrix<ELEMENT>& take, vector<int>& take_max_points, vector<int>& take_min_points)const{
  assert (take.get_element(y,x)==MUST_WIN || take.get_element(y,x)==MUST_LOSE);
  bool y_x_win = take.get_element(y,x)==MUST_WIN;
  //set previous rounds of element (y,x), e.g. L5\L4\L3\L2= L6 L1=0
  for (int k = end_round; k < x; ++k){
    if (y_x_win && take.get_element(y,k)==WIN){
      take.set_element(y, k, MUST_WIN);
      take_min_points[k-1] += POINTS[k-1];
      ApplyExclusiveRule(k, y, take, take_max_points, take_min_points);
    }
    if (!y_x_win && take.get_element(y,k)==LOSE){
      take.set_element(y, k, MUST_LOSE);
      take_max_points[k-1] -= POINTS[k-1];
      ApplyExclusiveRule(k, y, take, take_max_points, take_min_points);
    }
  }
}
//---------------------------------------------------------------------------------------------------- 
//ApplyReductionRule
//minterms will be modified as vector of strings
//A + A'B = A + B
//---------------------------------------------------------------------------------------------------- 
bool MintermHelper::ApplyReductionRule(SOP& products)const{
  bool optimized = false;
  queue<Variable> singles;
  for (size_t i=0; i<products.Size(); ++i){
    Product product = products[i];
    if (product.NumVariables() == 1){
      Variable single = product.get_ith_variable(1);
      singles.push(single);
    }
  }
  while (!singles.empty()){
    Variable single = singles.front();
    singles.pop();
    single.Negate();
    for (size_t i=0; i<products.Size(); ++i){
      Product product = products[i];
      if (product.Contain(single)){
        optimized = true;
        product.Remove(single);
        products[i] = product;
        if (product.NumVariables() == 1){
          Variable new_single (product.get_ith_variable(1));
          singles.push(new_single);
        }
      }
    }
  }
  return optimized;
}
//---------------------------------------------------------------------------------------------------- 
//ApplySelfDominateRule
//minterms will be modified as vector of strings
//S4_5'S4_6' = S4_5'
//S4_5S4_6 = S4_6
//For each product, will mark any variables which are redundant. Then sweep and only copy un-marked ones.
//---------------------------------------------------------------------------------------------------- 
bool MintermHelper::ApplySelfDominateRule(SOP& products)const{
  bool optimized = false;
  //mark
  for (size_t i=0; i<products.Size(); ++i){
    Product product = products[i];
    vector<bool> remained (product.NumVariables(), true);
    for (int j=1; j<=product.NumVariables() ; ++j){
      Variable variable = product.get_ith_variable(j);    
      if (variable.IsBar()){
        for (int k=j+1; k<=product.NumVariables() ; ++k){
          Variable other_variable = product.get_ith_variable(k);
          if (other_variable.IsBar() && variable.get_index() == other_variable.get_index()){
            optimized = true;
            int abandom = other_variable.get_round() > variable.get_round()? k-1:j-1;
            remained[abandom] = false;
            if (_p_logic_simulate->get_debug_level() >= DUMP){
              cout<< "In product " << product.get_string()<< 
                "bar variable "<< product.get_ith_variable(abandom+1).to_string()<< "is abandomed"<< endl;
            }
          }
        }
      }else{
        for (int k=j+1; k<=product.NumVariables() ; ++k){
          Variable other_variable = product.get_ith_variable(k);
          if (!other_variable.IsBar() && variable.get_index() == other_variable.get_index()){
            optimized = true;
            int abandom = other_variable.get_round() < variable.get_round()? k-1:j-1;
            remained[abandom] = false;
            if (_p_logic_simulate->get_debug_level() >= DUMP){
              cout<< "In product " << product.get_string()<< 
                " variable "<< product.get_ith_variable(abandom+1).to_string()<< " is abandomed"<< endl;
            }
          }
        }
      }
    }
    //sweep
    vector<Variable> reduced_variables(0);
    for (int j=1; j<=product.NumVariables() ; ++j){
      if (remained[j-1] == true)
        reduced_variables.push_back(product.get_ith_variable(j));
    }
    if ((int)reduced_variables.size() != product.NumVariables())
      products[i].set_variables(reduced_variables);
  }
  return optimized;
}
//---------------------------------------------------------------------------------------------------- 
//ApplyBracketRule
//S4_6' S1_6 = S1_6
//---------------------------------------------------------------------------------------------------- 
bool MintermHelper::ApplyBracketRule(SOP& products)const{
  bool optimized = false;
  //mark
  for (size_t i=0; i<products.Size(); ++i){
    Product product = products[i];
    vector<bool> remained (product.NumVariables(), 1);
    for (int j=1; j<=product.NumVariables() ; ++j){
      if (remained[j-1] == false) continue;
      Variable v = product.get_ith_variable(j);    
      if (!v.IsBar()){//S1_6
        int round = v.get_round();
        int seeding = v.get_seeding();
        REGION_SYMBOL region = v.get_region();
        int team = _p_logic_simulate->IndexTeam(region, seeding);
        int size_exclusive = int(pow(2,round));
        int block_head = int(floor((team-1)/size_exclusive)*size_exclusive) + 1;
        int block_tail = block_head + size_exclusive - 1;
        //cout << "Good v is "<< v.get_name()<< " with team "<< team<< 
          //" size_exclusive "<< size_exclusive<< " block_head "<< block_head<< 
          //" block_tail "<< block_tail<< " round "<< round<< endl;
        for (int k=1; k<=product.NumVariables() ; ++k){
          if (remained[k-1] == 0 || k==j) continue;
          Variable other_v = product.get_ith_variable(k);
          int other_seeding = other_v.get_seeding();
          REGION_SYMBOL other_region = other_v.get_region();
          int other_team = _p_logic_simulate->IndexTeam(other_region, other_seeding);
          //cout << "other v is "<< other_v.get_name()<< " with team "<< other_team<< 
            //" round "<< other_v.get_round()<< " is bar "<< other_v.IsBar() << endl;
          if (other_v.IsBar() && round==other_v.get_round() && 
              other_team<=block_tail && other_team>=block_head){
            optimized = true;
            remained[k-1] = 0;
            //cout << "Good other v is "<< other_v.get_name()<< " with team "<< other_team<< endl;
          }
        }
      }
    }
    //sweep
    vector<Variable> reduced_variables(0);
    for (int j=1; j<=product.NumVariables() ; ++j){
      if (remained[j-1] == 1)
        reduced_variables.push_back(product.get_ith_variable(j));
    }
    if ((int)reduced_variables.size() != product.NumVariables())
      products[i].set_variables(reduced_variables);
  }
  return optimized;
}
//---------------------------------------------------------------------------------------------------- 
//ApplySelfDenyRule
//S4_6' S4_6 = vanish
//S4_5' S4_6 = vanish
//---------------------------------------------------------------------------------------------------- 
bool MintermHelper::ApplySelfDenyRule(SOP& products)const{
  bool optimized = false;
  SOP copy_products;
  for (size_t i=0; i<products.Size(); ++i){
    Product product = products[i];
    bool vanish = false;
    for (int j=1; j<=product.NumVariables() ; ++j){
      Variable variable = product.get_ith_variable(j);    
      bool is_bar = variable.IsBar();//S1_6'
      int k = 1;
      for (k=j+1; k<=product.NumVariables() ; ++k){
        Variable other_variable = product.get_ith_variable(k);
        bool other_is_bar = other_variable.IsBar();
        //same team but opposite bar
        if (is_bar!=other_is_bar && variable.get_index()==other_variable.get_index()){
          //bar one is with earlier round
          if ((is_bar && variable.get_round()<=other_variable.get_round()) ||
              (other_is_bar && other_variable.get_round()<=variable.get_round())){
            optimized = true;
            vanish = true;
            break;
          }
        }
      }
      if (vanish)
        break;
    }
    if (!vanish)
      copy_products.add_product(product);
  }
  if (optimized)
    products = copy_products;
  return optimized;
}
//---------------------------------------------------------------------------------------------------- 
//ApplyAlwaysTrueRule
//empty product + X = empty product (1+X = X)
//---------------------------------------------------------------------------------------------------- 
bool MintermHelper::ApplyAlwaysTrueRule(SOP& products)const{
  bool optimized = false;
  bool always_true = false;
  for (size_t i=0; i<products.Size(); ++i){
    Product product_i = products[i];
    if (product_i.NumVariables() == 0){
      optimized = true;
      always_true = true;
      break;
    }
  }
  if (always_true){
    products.Clear();
    Product empty;
    products.add_product(empty);
  }
  return optimized;
}
//---------------------------------------------------------------------------------------------------- 
//ApplyComplementationRule
//S4_5' + S4_5 = empty product
//if one product is safe, it is copied to copy_products
//two complementary products will be merged into one empty product and copied to copy_products
//finally, products will be replaced by copy_products
//modified products is still in order
//---------------------------------------------------------------------------------------------------- 
bool MintermHelper::ApplyComplementationRule(SOP& products)const{
  bool optimized = false;
  vector<bool> visited(products.Size(), false);
  SOP copy_products;
  for (size_t i=0; i<products.Size(); ++i){
    if (visited[i])
      continue;
    Product product_i = products[i];
    if (product_i.NumVariables() != 1){
      copy_products.add_product(product_i);
      continue;
    }
    size_t j;
    for (j=i+1; j<products.Size(); ++j){
      if (visited[j])
        continue;
      Product product_j = products[j];
      if (product_j.NumVariables() != 1)
        continue;
      Variable variable_i_1 = product_i.get_ith_variable(1);
      Variable variable_j_1 = product_j.get_ith_variable(1);
      if ( 
          ( (variable_i_1.IsBar()&&!variable_j_1.IsBar()) || (!variable_i_1.IsBar()&&variable_j_1.IsBar()) )
          && variable_i_1.get_index() == variable_j_1.get_index()
          && variable_i_1.get_round() == variable_j_1.get_round()
         ){
        optimized = true;
        visited[j] = true;
        Product empty_product;
        copy_products.add_product(empty_product);
        break;
      }
    }
    //if no complementation happen
    if (j == products.Size())
      copy_products.add_product(product_i);
  }
  products = copy_products;
  return optimized;
}

//---------------------------------------------------------------------------------------------------- 
//ApplyInterDominateRule
//A_1 + A_2 = A1; A_1' + A_2' = A_2'
//if one product is safe, it is copied to copy_products
//two complementary products will be merged into one empty product and copied to copy_products
//finally, products will be replaced by copy_products
//modified products is still in order
//---------------------------------------------------------------------------------------------------- 
bool MintermHelper::ApplyInterDominateRule(SOP& products)const{
  bool optimized = false;
  vector<bool> visited(products.Size(), false);
  SOP copy_products;
  for (size_t i=0; i<products.Size(); ++i){
    if (visited[i])
      continue;
    Product product_i = products[i];
    if (product_i.NumVariables() != 1){
      copy_products.add_product(product_i);
      continue;
    }
    size_t j;
    for (j=i+1; j<products.Size(); ++j){
      if (visited[j])
        continue;
      Product product_j = products[j];
      if (product_j.NumVariables() != 1)
        continue;
      Variable variable_i_1 = product_i.get_ith_variable(1);
      Variable variable_j_1 = product_j.get_ith_variable(1);
      if ( 
          ( (variable_i_1.IsBar()&&variable_j_1.IsBar()) || (!variable_i_1.IsBar()&&!variable_j_1.IsBar()) )
          && variable_i_1.get_index() == variable_j_1.get_index()
          && variable_i_1.get_round() != variable_j_1.get_round()
         ){
        optimized = true;
        visited[j] = true;
        Product empty_product;
        if (variable_i_1.IsBar()){
          if (variable_i_1.get_round() < variable_j_1.get_round())
            copy_products.add_product(product_j);
          else
            copy_products.add_product(product_i);
        }else{
          if (variable_i_1.get_round() < variable_j_1.get_round())
            copy_products.add_product(product_i);
          else
            copy_products.add_product(product_j);
        }
        break;
      }
    }
    //if no complementation happen
    if (j == products.Size())
      copy_products.add_product(product_i);
  }
  products = copy_products;
  return optimized;
}

void MintermHelper::Extract(const Product& product_i, const Product& product_j, Product& common, SOP& reduced_pair)const{
  reduced_pair.Clear();
  reduced_pair.add_product(product_i);
  reduced_pair.add_product(product_j);

  for (int k=1; k<=product_i.NumVariables() ; ++k){
    Variable variable_k = product_i.get_ith_variable(k);
    for (int l=1; l<=product_j.NumVariables() ; ++l){
      Variable variable_l = product_j.get_ith_variable(l);
      if (variable_k.IsSame(variable_l)){
        common.add_variable(variable_k);
        reduced_pair[0].Remove(variable_k);
        reduced_pair[1].Remove(variable_k);
        break;//don't need proceed other product_j's variables
      }
    }
  }
  return;
}
//---------------------------------------------------------------------------------------------------- 
//ApplyReverseRule
//S4_6' M1_6' W9_6' = E4_6
//S4_6' M1_5' W9_6' = E4_6 M1_5'
//S4_5' = E4_5
//---------------------------------------------------------------------------------------------------- 
bool MintermHelper::ApplyReverseRule(SOP& products)const{
  bool optimized = false;
  int completed_round = _p_logic_simulate->get_completed_round();
  vector<int> remain_teams(0);//store value from 1~64
  Matrix<short> outcome = _p_logic_simulate->get_outcome().get_matrix();
  //remain_teams record the indexes of teams remained after _completed_round according to outcome
  for (size_t y=1; y<=outcome.get_y_size(); ++y){
    if (outcome.get_element(y,completed_round) == 1){
       remain_teams.push_back(y);
    }
  }
  assert ((int)remain_teams.size()==(int)pow(2,NUM_ROUNDS-completed_round));

  SOP copy_products;
  for (size_t i=0; i<products.Size(); ++i){
    Product product = products[i];
    bool vanish = false;
    //bars:       M1 W9 S4 E4
    //      round6: 0 0 0 0
    //      round5: 0 0 0 0
    Product copy_product;
    vector<vector<int> > bars(NUM_ROUNDS-completed_round, vector<int>(remain_teams.size(), 0));
    vector<vector<int> > nonbars(NUM_ROUNDS-completed_round, vector<int>(remain_teams.size(), 0));
    //for each product, write down the bars for variables within
    for (int j=1; j<=product.NumVariables() ; ++j){
      Variable variable = product.get_ith_variable(j);    
      int round = variable.get_round();
      int index = variable.get_index();
      bool is_bar = variable.IsBar();//S1_6'
      size_t k = 0;
      for (; k<remain_teams.size(); ++k){
        if (index == remain_teams[k]){
          if (is_bar){
            bars[round-completed_round-1][k] = 1;
          }else{
            copy_product.add_variable(variable);
            nonbars[round-completed_round-1][k] = 1;
          }
          break;
        }
      }
      assert (k != remain_teams.size());//should be able to find corresponding team index
    }
    //for each round, do the reverse
    for (int round = completed_round+1; round<= NUM_ROUNDS; ++round){
      int num_brackets = int(pow(2,NUM_ROUNDS-round));
      //in certain round, for each bracket check if it is legible for reverse
      for (int bracket = 0; bracket < num_brackets; ++bracket){
        int check_start = remain_teams.size()/num_brackets*bracket;
        int check_end = remain_teams.size()/num_brackets*(bracket+1)-1;
        int num_bar = 0;
        int num_nonbar = 0;
        int zero;
        for (int j=check_start; j<=check_end; ++j){
          int k=round;
          for (; k>completed_round; --k){
            if (bars[k-completed_round-1][j] == 1){
              num_bar++;
              break;
            }
          }
          if (k == completed_round){
            zero = j;
          }
          /*
          if (bars[round-completed_round-1][j] == 1)
            num_bar++;
          else
            zero = j;
            */
          k=round;
          for (; k<=NUM_ROUNDS; ++k){
            if (nonbars[k-completed_round-1][j] == 1){
              num_nonbar++;
              break;
            }
          }
        }
        assert (num_nonbar<=check_end-check_start+1 && num_bar<=check_end-check_start+1);
        if (num_nonbar > 1){//more than one nonbar
          optimized = true;
          vanish = true;
          break;
        }
        if (num_bar == check_end-check_start+1){//all are bar
          optimized = true;
          vanish = true;
          break;
        }else if (num_bar == check_end-check_start+1-1){//all but one are bar, the core case
          optimized = true;
          Variable reversed(remain_teams[zero], round, false);//M1_6
          copy_product.add_variable(reversed);
        }else{
          for (int j=check_start; j<=check_end; ++j){
            if (bars[round-completed_round-1][j] == 1){
              Variable keep(remain_teams[j], round, true);//W9_6'
              copy_product.add_variable(keep);
            }
          }
        }
      }
      if (vanish)
        break;
    }
    if (!vanish){
      copy_product.Sort();
      copy_products.add_product(copy_product);
    }
  }
  if (optimized)
    products = copy_products;
  return optimized;
}
//---------------------------------------------------------------------------------------------------- 
//ApplyCombineRule
//return true or false means if products got optimized
//---------------------------------------------------------------------------------------------------- 
bool MintermHelper::ApplyCombineRule(SOP& products)const{
  bool optimized = false;
  bool sub_optimized = ApplySelfDenyRule(products);//S4_6' S4_6 = vanish, S4_5' S4_6 = vanish
  optimized = optimized || sub_optimized;
  if (sub_optimized){
    if (_p_logic_simulate->get_debug_level() >= DETAIL){
      cout << "ApplySelfDenyRule turns products to be:";
      cout << products.get_string();
    }
  }

  sub_optimized = ApplySelfDominateRule(products);//S4_5'S4_6' = S4_5'
  optimized = optimized || sub_optimized;
  if (sub_optimized){
    if (_p_logic_simulate->get_debug_level() >= DETAIL){
      cout << "ApplySelfDominateRule turns products to be:";
      cout << products.get_string();
    }
  }
  sub_optimized = ApplyBracketRule(products);//S4_6' S1_6 = S1_6
  optimized = optimized || sub_optimized;
  if (sub_optimized){
    if (_p_logic_simulate->get_debug_level() >= DETAIL){
      cout << "ApplyBracketRule turns products to be:";
      cout << products.get_string();
    }
  }

  vector<bool> valid(products.Size(), true);
  for (size_t i=0; i<products.Size(); ++i){
    if (!valid[i])  continue;
    for (size_t j=i+1; j<products.Size(); ++j){
      if (!valid[j])  continue;
      if (_p_logic_simulate->get_debug_level() >= DUMP){
        cout << "i is "<< i<< " j is "<< j<< endl;
        cout << "products[i] is:";
        products[i].Print();
        cout << "products[j] is:";
        products[j].Print();
      }
      Product common;
      SOP reduced_pair;
      Extract (products[i], products[j], common, reduced_pair);

      if (_p_logic_simulate->get_debug_level() >= DUMP){
        cout << "common is:";
        common.Print();
        cout << "reduced_pair[0] is:";
        reduced_pair[0].Print();
        cout << "reduced_pair[1] is:";
        reduced_pair[1].Print();
      }
      sub_optimized = ApplyAlwaysTrueRule(reduced_pair);//1 + X = 1, X can even be ""
      optimized = optimized || sub_optimized;
      if (reduced_pair.Size() == 1){//get reduced
        valid[j] = false;
        products[i].set_variables(common.get_variables());
        if (_p_logic_simulate->get_debug_level() >= DUMP){
          cout << "ApplyAlwaysTrueRule turns products[i] to be:";
          products[i].Print();
        }
        continue;
      }
      sub_optimized = ApplyComplementationRule(reduced_pair);//A + A' = ""
      optimized = optimized || sub_optimized;
      if (reduced_pair.Size() == 1){//get reduced
        valid[j] = false;
        products[i].set_variables(reduced_pair[0].get_variables());
        products[i].add_variables(common.get_variables());
        if (_p_logic_simulate->get_debug_level() >= DUMP){
          cout << "ApplyComplementationRule turns products[i] to be:";
          products[i].Print();
        }
        continue;
      }
      sub_optimized = ApplyInterDominateRule(reduced_pair);//A_1 + A_2 = A1; A_1' + A_2' = A_2'
      optimized = optimized || sub_optimized;
      if (reduced_pair.Size() == 1){//get reduced
        valid[j] = false;
        products[i].set_variables(reduced_pair[0].get_variables());
        products[i].add_variables(common.get_variables());
        if (_p_logic_simulate->get_debug_level() >= DUMP){
          cout << "ApplyInterDominateRule turns products[i] to be:";
          products[i].Print();
        }
        continue;
      }
      sub_optimized = ApplyReductionRule(reduced_pair);//A + A'B = A + B
      optimized = optimized || sub_optimized;
      if (sub_optimized){
        products[i].set_variables(reduced_pair[0].get_variables());
        products[i].add_variables(common.get_variables());
        products[j].set_variables(reduced_pair[1].get_variables());
        products[j].add_variables(common.get_variables());
        if (_p_logic_simulate->get_debug_level() >= DUMP){
          cout << "ApplyReductionRule turns products[i] to be:";
          products[i].Print();
          cout << "ApplyReductionRule turns products[j] to be:";
          products[j].Print();
        }
      }
    }//end of inner for loop
  }//end of outter for loop
  SOP copy_products;
  for (size_t i=0; i<products.Size(); ++i){
    if (valid[i]){
      products[i].Sort();
      copy_products.add_product(products[i]);
    }
  }
  products = copy_products;
  return optimized;
}
//---------------------------------------------------------------------------------------------------- 
//FindMinterm
//return if player1 has any condition to win player2 based on _diff
//_diff is the guess-difference matrix from player1 - player2
//points is the pre-accumulated points
//start_round(end_round) is the round to start(end) the recursion. Round goes backwards.
//start_team(end_team) is the first(last) team to start(end) the recursion in start_round(end_round) round
//minterm is the condition player1 can win player2
//FindMinterm will scan back from last column in top-down manner. 
//Any 1 in the _diff matrix will recursively call two sub-FindMinterm
//Results will be combined and returned
//This FIndMinterm has no optimization in it
//----------------------------------------------------------------------------------------------------
bool MintermHelper::FindMinterm(const Matrix<ELEMENT>& diff, const int& points, const int& start_round, const int& start_team, const int& end_round, const int& end_team, vector<string>& minterms)const
{
  //search column by column backwards.
  for (int x = start_round; x >= end_round; --x){
    //In each column, search top down
    stringstream round;
    round << x;
    int y_end = x==end_round?end_team:diff.get_y_size();
    int y_start= x==start_round?start_team:1;
    //search for next 1 or -1. if find, recursive call and combine; if not find, return
    for (int y = y_start; y <= y_end; ++y){
      if (diff.get_element(y,x) == WIN || diff.get_element(y,x) == LOSE){//only care none 0 element
        bool positive_one = diff.get_element(y,x) == WIN;
        Team team = _p_logic_simulate->calculate_team(y);
        Region region = _p_logic_simulate->calculate_region(y);
        stringstream seeding;
        seeding << team.get_seeding();
        Matrix<ELEMENT> take = diff, not_take = diff;

        int take_start_round, take_start_team, not_take_start_round, not_take_start_team;
        //calculate the size and range of sub call. For take case, next will be zeros and it's OK
        CalculateSubcallBound(x, y, y_end, end_round, not_take_start_round, not_take_start_team);
        take_start_round = not_take_start_round;
        take_start_team = not_take_start_team;

        not_take.set_element(y, x, ZERO);
        take.set_element(y, x, positive_one?MUST_WIN:MUST_LOSE);
        Apply(x, y, take);
        //ApplyExclusiveRule(x, y, positive_one, block_head, block_tail, take, not_take, temp_take_max_points, temp_take_min_points);

        //sub calls
        int take_points = points + positive_one?points+POINTS[x-1]:points-POINTS[x-1];
        bool win;
        vector<string> take_minterms, not_take_minterms;
        win = FindMinterm (take, take_points, take_start_round, take_start_team, end_round, end_team, take_minterms);
        if (win){
          for (size_t l = 0; l < take_minterms.size(); ++l)
            take_minterms[l] = region.read_region_symbol() + seeding.str() + "_" + round.str() + " " + take_minterms[l];
        }
        win = FindMinterm (not_take, points, not_take_start_round, not_take_start_team, end_round, end_team, not_take_minterms);
        if (win){
          for (size_t l = 0; l < not_take_minterms.size(); ++l)
            not_take_minterms[l] = region.read_region_symbol() + seeding.str() + "_" + round.str() + "'" + " " + not_take_minterms[l]; 
        }
        minterms.clear();
        minterms.insert(minterms.end(), take_minterms.begin(), take_minterms.end());
        minterms.insert(minterms.end(), not_take_minterms.begin(), not_take_minterms.end());
        assert(minterms.size() == take_minterms.size() + not_take_minterms.size());
        return minterms.size() > 0;
      }//end of if (diff.get_element(y,x) == 1 || diff.get_element(y,x) == -1)
    }//end of y
  }//end of x (round)
  //assert (points != 0);//don't know how to do with tie
  //if no 1 or -1
  minterms.clear();
  if (points > 0){
    stringstream final_points;
    final_points << points;
    minterms.push_back(final_points.str());
    return true;
  }else{
    return false;
  }
}
//---------------------------------------------------------------------------------------------------- 
//FindMintermOpt
//return if player1 has any condition to win player2 based on _diff
//_diff is the guess-difference matrix from player1 - player2
//must is the matrix indicating which element must be took
//max_points represent max possible points player1 can achieve according to all knowledge
//points is the pre-accumulated points
//start_round(end_round) is the round to start(end) the recursion. Round goes backwards.
//start_team(end_team) is the first(last) team to start(end) the recursion in start_round(end_round) round
//minterm is the condition player1 can win player2
//FindMintermOpt will scan back from last column in top-down manner. 
//Any 1 in the _diff matrix will recursively call two sub-FindMintermOpt
//Results will be combined and returned
//This FIndMintermOpt has optimization in it; pass all integers with reference can reduce 70% runtime!
//----------------------------------------------------------------------------------------------------
bool MintermHelper::FindMintermOpt(const Matrix<ELEMENT>& diff, const vector<int>& max_points, const vector<int>& min_points, const int& points, const int& start_round, const int& start_team, const int& end_round, const int& end_team, vector<string>& minterms)const
{
  assert(max_points.size() == diff.get_x_size()); 
  assert(min_points.size() == diff.get_x_size()); 
  //search column by column backwards.
  for (int x = start_round; x >= end_round; --x){
    //In each column, search top down
    stringstream round;
    round << x;
    int y_end = x==end_round?end_team:diff.get_y_size();
    int y_start= x==start_round?start_team:1;
    //search for next 1 or -1. if find, recursive call and combine; if not find, return
    for (int y = y_start; y <= y_end; ++y){
      //if already done or already failed
      int max_sum_points = accumulate(max_points.begin(), max_points.begin() + x, 0);
      int min_sum_points = accumulate(min_points.begin(), min_points.begin() + x, 0);
      //already impossible to win. if points plus max possible points below or equal 0, no possible solution
      if (points + max_sum_points <= 0){
        minterms.clear();
        return false;
      }
      //if points plus min possible points is more than 0, the solution is matured
      if (points + min_sum_points > 0){
        stringstream final_points;
        final_points << points;
        minterms.clear();
        //minterms.push_back("earned points " + final_points.str());
        //For the constructor of product, it will eliminate the point
        minterms.push_back(final_points.str());
        return true; 
      }

      Team team = _p_logic_simulate->calculate_team(y);
      Region region = _p_logic_simulate->calculate_region(y);
      stringstream seeding;
      seeding << team.get_seeding();
      if (diff.get_element(y,x) == MUST_WIN || diff.get_element(y,x) == MUST_LOSE){
        int next_start_round, next_start_team, next_points;
        if (diff.get_element(y,x) == MUST_WIN)
          next_points = points + POINTS[x-1];
        else
          next_points = points - POINTS[x-1];
        CalculateSubcallBound(x, y, y_end, end_round, next_start_round, next_start_team);
        //points, max_points and min_points should not be changed because they are already changed
        if (_p_logic_simulate->get_debug_level() >= DUMP)
          cout << "Must points is: "<< next_points<< ", start_round is "<< next_start_round<< ", start_team is "<<next_start_team<<endl;
        return FindMintermOpt (diff, max_points, min_points, next_points, next_start_round, next_start_team, end_round, end_team, minterms);
      }
      else if (diff.get_element(y,x) == WIN || diff.get_element(y,x) == LOSE){//only care none 0 element
        //calculate the information for take and not_take calls
        int take_start_round, take_start_team, not_take_start_round, not_take_start_team;
        //calculate the size and range of sub call. For take case, next will be zeros and it's OK
        CalculateSubcallBound(x, y, y_end, end_round, not_take_start_round, not_take_start_team);
        take_start_round = not_take_start_round;
        take_start_team = not_take_start_team;

        Matrix<ELEMENT> take = diff, not_take = diff;
        //These two vectors show how much in each round player1 still can achieve maximally (minimally) according to current knowledge about MUST_WIN and MUST_LOSE. Only for non-considered elements.
        vector<int> take_max_points (max_points);
        vector<int> take_min_points (min_points);
        vector<int> not_take_max_points (max_points);
        vector<int> not_take_min_points (min_points);
        vector<string> take_minterms, not_take_minterms;

        //not_take case:
        not_take.set_element(y, x, ZERO); 
        if (diff.get_element(y,x) == WIN)
          not_take_max_points[x-1] -= POINTS[x-1];
        else
          not_take_min_points[x-1] += POINTS[x-1];
        bool win;
        if (_p_logic_simulate->get_debug_level() >= DUMP)
          cout << "not take points is: "<< points<< ", start_round is "<< not_take_start_round<< ", start_team is "<<not_take_start_team<<endl;
        win = FindMintermOpt (not_take, not_take_max_points, not_take_min_points, points, not_take_start_round, not_take_start_team, end_round, end_team, not_take_minterms);
        if (win){
          for (size_t l = 0; l < not_take_minterms.size(); ++l)
            not_take_minterms[l] = region.read_region_symbol() + seeding.str() + "_" + round.str() + "'" + " " + not_take_minterms[l]; 
        }
        //take case:
        take.set_element(y, x, diff.get_element(y,x) == WIN?MUST_WIN:MUST_LOSE);
        int take_points = diff.get_element(y,x) == WIN?points+POINTS[x-1]:points-POINTS[x-1];//This change of take_points is not within ApplyReverseRegressiveRule
        if (diff.get_element(y,x) == WIN)
          take_max_points[x-1] -= POINTS[x-1];
        else
          take_min_points[x-1] += POINTS[x-1];
        ApplyExclusiveRule(x, y, take, take_max_points, take_min_points);
        ApplyReverseRegressiveRule(x, y, end_round, take, take_max_points, take_min_points);
        if (_p_logic_simulate->get_debug_level() >= DUMP)
          cout << "take points is: "<< take_points<< ", start_round is "<< take_start_round<< ", start_team is "<<take_start_team<<endl;
        win = FindMintermOpt (take, take_max_points, take_min_points, take_points, take_start_round, take_start_team, end_round, end_team, take_minterms);
        if (win){
          for (size_t l = 0; l < take_minterms.size(); ++l)
            take_minterms[l] = region.read_region_symbol() + seeding.str() + "_" + round.str() + " " + take_minterms[l];
        }

        //combine both not_take and take cases
        minterms.clear();
        minterms.insert(minterms.end(), take_minterms.begin(), take_minterms.end());
        minterms.insert(minterms.end(), not_take_minterms.begin(), not_take_minterms.end());
        assert(minterms.size() == take_minterms.size() + not_take_minterms.size());
        return minterms.size() > 0;
      }//end of if (diff.get_element(y,x) == 1 || diff.get_element(y,x) == -1)
    }//end of y
  }//end of x (round)
  //assert (points != 0);//don't know how to do with tie
  //if no 1 or -1
  minterms.clear();
  if (points > 0){
    stringstream final_points;
    final_points << points;
    //minterms.push_back("earned points " + final_points.str());
    //For the constructor of product, it will eliminate the point
    minterms.push_back(final_points.str());
    return true;
  }else{
    return false;
  }
}
