/*********************************************************************
 * Hybrid approach with boost::dynamic_bitset and fast preprocess    *
 *********************************************************************/

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <cstring>
#include <algorithm>
#include <vector>
#include <set>
#include <iterator>
#include <fstream>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <thread>
#include "boost/dynamic_bitset.hpp"

using namespace std;
string borderL ("**************************************");
string border = borderL + borderL + "\n";
clock_t startT, bestT, endT;
time_t timer_start, timer_current, timer_end;
double seconds;
bool print = false, sol = false, exitTD = false;

set<int> universe, universeF;
multiset<int> rep;

unsigned n, logn;
unsigned currentSize, bestSize, number_sets;

boost::dynamic_bitset<> empty;
vector<boost::dynamic_bitset<> > bitAA, out;

/***********************************************************************
 * Simple procedure to load data from file.                            *
 ***********************************************************************/

vector<set<int> > load_data(string file_name){
  
  string line;
  const char* delimiters = " =[],";
  char *pch;
  ifstream inputFile;
  inputFile.open (file_name.c_str(), ifstream::in);
  
  vector<set<int> > AA;
  set<int> temp_set;
  set<int>::iterator it,it_u;
  multiset<int>::iterator it_r;
  
  if(inputFile.is_open()){
    it_u=universe.begin();
    it_r=rep.begin();
    while (inputFile.good()){
      getline (inputFile, line );
      if (line.compare(0,1,"#") == 0 ){
        getline (inputFile,line);        
        while((line.compare(0,1,"#")) != 0){
          getline ( inputFile, line );
        }
        getline (inputFile,line);
      }
      pch = strtok (&line[0],delimiters);
      it=temp_set.begin();
      while (pch != NULL){
        if(atoi(pch) != 0){
          temp_set.insert(it,atoi(pch));
          universe.insert(it_u,atoi(pch));
          rep.insert(it_r,atoi(pch));
        }
        pch = strtok (NULL, delimiters);
      }
      if(!temp_set.empty()){
        AA.push_back(temp_set);
        temp_set.clear();
      }
    }
  }else{
    cout << "Unable to open " << file_name << " " << endl;
    exit(1);
  }
  inputFile.close();
  
  return AA;
}

/***********************************************************************
 * Simple procedure to store the output into file.                     *
 ***********************************************************************/

void save_data(string file_name){
  
  ofstream outFile;
  outFile.open (file_name.c_str(), ifstream::out);
  
  srand(time(NULL));
  outFile << "# " << out.size() 
          << " solutions found " << endl;
  for(unsigned i=0;i<out.size();++i){
    outFile << "B" << " = [ ";
    set<int>::iterator it;
    for(unsigned k=0;k<out[i].size();++k){
      if(out[i][k]){
        it = universeF.begin();   //OPTIMIZE
        advance(it, k);
        outFile << *it << " ";
      }
    }
    outFile << "]" << endl;
  }
  outFile.close();
}

/***********************************************************************
 * The dynamic_fast_preprocess function preprocess the sets given as   *
 * input. The first step is to build the {0/1}^(m x n) matrix such     * 
 * that A[i][j] = 1 iff the i-th element of the universe belongs to    * 
 * set A_j. The next step is to remove duplicates. This is possibile   *
 * by sorting the matrix column-wise and then remove duplicates. O(mn) *
 * time to build the matrix, O(nlogn) for the real preprocess          *
 * (n = # of sets, m = total number of elements without duplicates,    *
 * i.e. the univers size)                                              *
 ***********************************************************************/

void dynamic_fast_preprocess(vector<set<int> > AA, set<int> universe){
  
  vector<pair<boost::dynamic_bitset<>, int> > set_value;
  set_value.resize(0);
  
  // build the matrix
  for(set<int>::iterator itU=universe.begin();itU!=universe.end();++itU){  // for each item x in the universe
    boost::dynamic_bitset<> position (number_sets, 0);
    for(unsigned i=0;i<number_sets;++i){
      if(*itU == *AA[i].begin()){                 // if x belongs to A_i
        position[i] = 1;                          // then set A[index(x)][i] = 1
        AA[i].erase(AA[i].begin());
      }
    }
    pair<boost::dynamic_bitset<>, int> temp(position,*itU);
    set_value.push_back(temp);
  }
  
  sort(set_value.begin(),set_value.end());       // sort the matrix column-wise

  // remove duplicates, just one scan, O(n)
  unsigned j=0, i=1;    
  while(i<set_value.size()){
    if(set_value[j].first == set_value[i].first){
      set_value.erase(set_value.begin()+i);
    }else{
      j=i;
      i++;
    }
  }
  
  bitAA.resize(number_sets, boost::dynamic_bitset<> (set_value.size(),0));
  set<int>::iterator itUF=universeF.begin();
  // compute the transpose matrix
  for(unsigned i=0;i<set_value.size();i++){
    universeF.insert(itUF,set_value[i].second);
    for(unsigned j=0;j<set_value[i].first.size();j++){
      bitAA[j][i] = set_value[i].first[j];
    }
  }
  
}


/***********************************************************************
 * Simple procedure aimed at printing the solutions                    *
 ***********************************************************************/


void print_solutions(boost::dynamic_bitset<> B){
  
  set<int>::iterator it;
  for(unsigned k=0;k<B.size();++k){
    if(B[k]){
      it = universeF.begin();
      advance(it, k);
      cout << *it << " ";
    }
  }
    cout << endl;
}
    

/**********************************************************************
 *                         TOP DOWN FUNCTIONS                         *
 **********************************************************************/
/***********************************************************************
 * chek_minimality checks whether the solution B is minimal or not     *
 * To test the minimality, an element x that occurs in B is deleted.   *
 * If B is still a solution then B is not minimal and the minimal      *
 * B' = B \ {x} is returned. If for all x \in B, B' = B \ {x} is not a *
 * solution then B is minimal and B is returned. This process has to   *
 * be performed for each item in the set. To check whether B is a      *
 * solution or not, the following tests are performed:                 *
 * (1) A_i \cap B' != \emptyset  for all i \in {1,...,n}               *
 * (2) A_i \cap B' != A_j \cap B' for all i,j \in {1,...,n}            *
 ***********************************************************************/

boost::dynamic_bitset<> check_minimality(boost::dynamic_bitset<> B){
  
  bool minimality;
  for(unsigned k=0;k<B.size();++k){
    if(B[k]){
      B[k] = 0;  // delete the k-th element of B, for all k. B' = B \ B[k]
      minimality = true;
      for(unsigned i=0;i<number_sets;++i){
        if((bitAA[i] & B) == empty){  // (1) if A_i \cap B' == \emptyset
          minimality = false;         // then B[k] covers set A_i
          break;
        }
        if(minimality){
          for(unsigned j=0;j<number_sets;++j){
            if(i != j){
              if((bitAA[i] & B) == (bitAA[j] & B)){  // (2) if A_i \cap B' == A_j \cap B'
                minimality = false;                  // then B[k] differentiate
                break;                               // A_i from A_j
              }
            }
          }
        }
      }
      if(minimality)
        return B;
      B[k] = 1;
    }
  }
  return B;
}


/***********************************************************************
 * td_find_solutions finds recursively the minimal differential sets.  *
 * Let A_1,A_2,...,A_n be an ordering of the given sets.               *
 * The function starts with B = \emptyset.                             *
 * -- If m=1 then for all x \in A_1, B = B \cup {x} and compute        *
 *    find_sol(B,m+1).                                                 *
 * -- If m>1 then                                                      *
 *   (1) if A_m \cap B == \emptyset then for all x \in A_m             *
 *       B = B \cup {x} and compute find_sol(B,m+1)                    *
 *   (2) if there exists 0 <= j < m s.t. A_m \cap B == A_j \cap B then *
 *       for all x in A_m \sd A_j, B = B \cup {x} and compute          *
 *       find_sol(B,m+1)                                               *
 *   (3) if either (1) and (2) are false the compute find_sol(B,m+1)   *
 *       with B unchanged                                              *
 ***********************************************************************/

void td_find_solutions(vector<boost::dynamic_bitset<> > bitAA, 
                         boost::dynamic_bitset<> B, unsigned m){
                           
  if(B.count()>currentSize){
    return;
  }else{
    if(m>=number_sets){                                // if all the sets have been analyzed
      boost::dynamic_bitset<> M = check_minimality(B); // and the solution is minimal
      if(M.count()<currentSize){                       // and the current size is strictly less
        currentSize = M.count();                       // than the current best => found an new optimum
        timer_current = time(NULL);
        seconds = (double) (timer_current-timer_start);
        cout << "** Top Down Algorithm found a solution of size " 
           << setw(8) << currentSize << " Time " 
           << setw(10) << seconds << "s " << " **"<< endl;
        out.clear();
      }
      if(print){
        print_solutions(M);
      }
      out.push_back(M);
      return;
    }else if((m==0) || ((bitAA[m] & B) == empty)){ // (1) if A_m \cap B == \emptyset
      for(unsigned i=0;i<n;++i){                   // for all x \in A_m
        if(bitAA[m][i]) {
          B[i] = 1;                                // B = B \cup {x}                      
          td_find_solutions(bitAA,B,m+1);          // find_sol(B,m+1)
          B[i] = 0;                                // analyze the next set
        }
      }
    }else{ 
      sol = true;
      for(unsigned j=0;j<m;++j){
        if(j!=m){                                 // (2) if there exists  0<= j < m
          if((bitAA[m] & B) == (bitAA[j] & B)){   // s.t. A_m \cap B == A_j \cap B
            sol = false;
            boost::dynamic_bitset<> C = bitAA[m] ^ bitAA[j];  // C = symmetric difference of A_m and A_j
            for(unsigned i=0;i<n;++i){                        // for all x \in C
              if(C[i] == 1){
                B[i] = 1;                                     // B = B \cup {x}
                td_find_solutions(bitAA,B,m+1);               // find_sol(B,m+1)
                B[i] = 0;                                     // analyze the next set
              }
            }
          }
        }
      }
      if(sol){                        // (3) otherwise (1) and (2) are false
        sol = false;
        td_find_solutions(bitAA,B,m+1);  // find_sol(B,m+1), B unchanged
      }
    }
  }
}

/***********************************************************************
 * Simple procedure aimed at synchronize the two algorithms            *
 ***********************************************************************/

void pre_td_find_solutions(vector<boost::dynamic_bitset<> > bitAA, 
                             boost::dynamic_bitset<> B, unsigned m){
                               
  td_find_solutions(bitAA,B,m);
  exitTD = true;
  cout << "**********************"
       << " Top Down Algorithm terminated  "
       << "**********************" << endl;
}


/**********************************************************************
 *                         BOTTOM UP FUNCTIONS                        *
 **********************************************************************/
/***********************************************************************
 * check_Solution checks whether BC is a solution or not. To test      *
 * whether a given set B is a solution two actions have to be          *
 * performed:                                                          *
 * (1) for all i \in {1,...,n} A_i \cap B != \emptyset                 *
 * (2) for all i,j \in {1,...,n} A_i \cap B != A_j \cap B              *
 ***********************************************************************/

bool check_Solution(boost::dynamic_bitset<> BC){
  
  bool solution = true;
  for(unsigned i=0;i<number_sets;i++){
    if((bitAA[i] & BC) == empty){ // (1) for all i \in {1,...,n} A_i \cap B != \emptyset
      solution = false;
      break;
    }
    if(solution){
      for(unsigned j=0;j<number_sets;j++){
        if(i != j){
          if((bitAA[i] & BC) == (bitAA[j] & BC)){  // (2) for all i,j \in {1,...,n} A_i \cap B != A_j \cap B
            solution = false;
            break;
            break;
          }
        }
      }
    }
  }
  return solution;
}

/***********************************************************************
 * bu_find_solutions finds iteratively the minimal differential sets   *
 * The procedure generates a mask of "ones" bits 1 of lenght m. For    *
 * istance, if ones = 1 and m = 3, the first mask will be 100 and      *
 * then, starting from the first mask, all the possible permutation    *
 * are computed, i.e. 010,001. In the next step ones is increased and  *
 * all the possible permutation are computed, e.g. ones = 2, 110,101,  *
 * 011. Intuitively, if M is a mask, M[i] = 1 and B is the set         *
 * generated from the mask M, then the i-th element of the universe    *
 * will belong to B. The procedure computes all the masks starting     *
 * from ones = logn to ones = n where n is the number of sets.         *
 * If the procedure finds out a solution then the control is left to   *
 * the topDown algorithm which is usually faster (is goal-driven)      *
 ***********************************************************************/

void bu_find_solutions(){
  vector<bool> mask;
  boost::dynamic_bitset<> BC (n, 0);
  bool solution = false, printSolution = true;
  for(unsigned ones=logn; ones<=number_sets; ++ones){
    mask.resize(n,0);                  // generate a mask of n
    for(unsigned i=0; i<ones; ++i)     // and set "ones" bit equals to 1
        mask[i] = 1;                   // (ones is an int and stands for the number of elements taken at this turn)
    do{
      for(unsigned i=0; i<n; ++i)
          BC[i] = mask[i];             // and generate the "possible" solution BC from the mask,
      if(solution && BC.count()>bestSize)
        return;
      if(BC.count()>bestSize){
        bestSize = BC.count();
        timer_current = time(NULL);
        seconds = (double) (timer_current-timer_start);
        cout << "** Bottom Up Algorithm analyzing sets of size " 
             << setw(9) << bestSize << " Time " 
             << setw(10) << seconds << "s " << " **"<< endl;
      }
      if(check_Solution(BC)){      // check whether BC is a solution
        solution = true;           // if BC is a solution
        if(printSolution){
          currentSize = bestSize;     // update the current best size
          timer_current = time(NULL);
          seconds = (double) (timer_current-timer_start);
          cout << "** Bottom Up Algorithm found a solution of size " 
           << setw(7) << bestSize << " Time " 
           << setw(10) << seconds << "s " << " **"<< endl;
          printSolution = false;
          cout << "**********************"
             << " Bottom Up Algorithm terminated "
             << "**********************" << endl;
          cout << "** Top Down Algorithm analyzing sets of size " 
           << setw(10) << bestSize << " Time " 
           << setw(10) << seconds << "s " << " **"<< endl;
          return;                   // and return, now the control is given to topDown only
        }
      }
      if(bestSize == currentSize){  // if topDown algorithm has already found a solution of size currentSize
                                    // topDown and bottomUp just meet (w.r.t. the size of the set)
        cout << "**********************"
             << " Bottom Up Algorithm terminated "
             << "**********************" << endl;
        cout << "** Top Down Algorithm analyzing sets of size " 
           << setw(10) << bestSize << " Time " 
           << setw(10) << seconds << "s " << " **"<< endl;
        return;                      // return and give the control to topDown only
      }
      if(exitTD){
        cout << "**********************"
             << " Bottom Up Algorithm terminated "
             << "**********************" << endl;
        return;
      }
    }while(next_permutation( mask.rbegin(), mask.rend() ) );  // compute the next permutation of the mask
  }
}

/***********************************************************************/

int main (int argc, char* argv[]) {
  
  string input_file ("sets.data");
  string output_file ("output.data");
  
  // Input functionalities
  if(argc>1){
    for(int i=1; i<argc;++i){
      if (strcmp(argv[i],"-v") == 0){
        print = true;
      }else if (strcmp(argv[i],"-i") == 0){
        if(argc>i+1){
          input_file = argv[i+1];
          i++;
        }else{
          cout << "Error while parsing input." << endl;
          exit(1);
        }
      }else if (strcmp(argv[i],"-o") == 0){
        if(argc>i+1){
          output_file = argv[i+1];
          i++;
        }else{
          cout << "Error while parsing input." << endl;
          exit(1);
        }
      }else{
        cout << "Error while parsing input." << endl;
        exit(1);
      }
    }
  }
    
  cout << fixed;
  cout << setprecision(1);
  
  vector<set<int> > AA;  // the input
  boost::dynamic_bitset<> B;  //the output
  
  AA = load_data(input_file);  // load the sets from the file
  number_sets = AA.size();
  
  cout << border <<"Number of Sets: " << number_sets << endl;
  cout << "Total Number of Elements: " << rep.size() << endl;  
  cout << "Number of non repeated Elements: " << universe.size() << endl;  
  
  cout << "****************************" 
       << " Preprocessing Sets " 
       << "****************************" << endl;

  startT = clock();
  dynamic_fast_preprocess(AA,universe);        // preprocess sets
  endT = clock();
  seconds = (double) (endT-startT) / CLOCKS_PER_SEC;
  
  cout << "Preprocess completed in " << seconds << "s"<< endl;
  cout << "Number of non repeated Elements after preprocess: " 
       << universeF.size() << endl << border;

  logn = floor ((log(number_sets) / log(2)));
  n = universeF.size();
  currentSize = number_sets+1;
  bestSize = 0;
  empty.resize(n,false);
  B.resize(n,false);

  timer_start = time(NULL);
  thread td(pre_td_find_solutions,bitAA,B,0);       // thread for the topDown algorithm
  thread bu(bu_find_solutions);                     // thread for the bottomUp algorithm
  bu.join();
  td.join();
  timer_end = time(NULL);
  seconds = (double) (timer_end-timer_start);
  cout << "All solutions found in " << seconds << "s"<< endl;
  cout << "*************************" 
       << " PostProcessing Solutions " 
       << "*************************" << endl;
       
  // the postprocess is just a task that removes
  // duplicates and saves the solutions to file
  
  sort(out.begin(),out.end());
  out.erase(unique(out.begin(),out.end()),out.end());
  save_data(output_file);
  cout << out.size() << " solutions found, saved to " << output_file 
       << endl << border;
  
  return 0;
 
}
