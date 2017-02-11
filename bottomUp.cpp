/*********************************************************************
 * Bottom-up approach with boost::dynamic_bitset and fast preprocess *
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
#include "boost/dynamic_bitset.hpp"

using namespace std;
string borderL ("****************************");
string border = borderL + borderL + "\n";
clock_t startT, bestT, endT;
double t;
bool print = false;

set<int> universe, universeF;
multiset<int> rep;

unsigned n, logn;
unsigned currentSize, number_sets;

boost::dynamic_bitset<> empty;
vector<boost::dynamic_bitset<> > bitAA, out;

/***********************************************************************/

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

/***********************************************************************/

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
        it = universeF.begin();
        advance(it, k);
        outFile << *it << " ";
      }
    }
    outFile << "]" << endl;
  }
  outFile.close();
}

/***********************************************************************/

void dynamic_fast_preprocess(vector<set<int> > AA, set<int> universe){
  
  vector<pair<boost::dynamic_bitset<>, int> > set_value;
  set_value.resize(0);
  
  for(set<int>::iterator itU=universe.begin();itU!=universe.end();++itU){
    boost::dynamic_bitset<> position (number_sets, 0);
    for(unsigned i=0;i<number_sets;++i){
      if(*itU == *AA[i].begin()){
        position[i] = 1;
        AA[i].erase(AA[i].begin());
      }
    }
    pair<boost::dynamic_bitset<>, int> temp(position,*itU);
    set_value.push_back(temp);
  }
  
  sort(set_value.begin(),set_value.end());

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
  
  for(unsigned i=0;i<set_value.size();i++){
    universeF.insert(itUF,set_value[i].second);
    for(unsigned j=0;j<set_value[i].first.size();j++){
      bitAA[j][i] = set_value[i].first[j];
    }
  }
  
}

/***********************************************************************/

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

/***********************************************************************/

bool check_Solution(boost::dynamic_bitset<> BC){
  
  bool solution = true;
  for(unsigned i=0;i<number_sets;i++){
    if((bitAA[i] & BC) == empty){
      solution = false;
      break;
    }
    if(solution){
      for(unsigned j=0;j<number_sets;j++){
        if(i != j){
          if((bitAA[i] & BC) == (bitAA[j] & BC)){
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

/***********************************************************************/

void find_solutions(){
  vector<bool> mask;
  boost::dynamic_bitset<> BC (n, 0);
  bool solution = false, printSolution = true;
  for(unsigned ones=logn; ones<=number_sets; ++ones){
    mask.resize(n,0);
    for(unsigned i=0; i<ones; ++i)
        mask[i] = 1;
    do{
      for(unsigned i=0; i<n; ++i)
          BC[i] = mask[i];
      if(solution && BC.count()>currentSize)
        return;
      if(BC.count()>currentSize){
        currentSize = BC.count();
        bestT = clock();
        t = (double) (bestT-startT) / CLOCKS_PER_SEC;
        cout << "** Analyzing sets of size " 
             << setw(9) << currentSize << " Time " 
             << setw(10) << t << "s " << " **"<< endl;
      }
      if(check_Solution(BC)){
        solution = true;
        if(printSolution){
          bestT = clock();
          t = (double) (bestT-startT) / CLOCKS_PER_SEC;
          cout << "*****" << setw(6) << " Solution of size" 
               << setw(6) << currentSize << " found. Time  " 
               << setw(6) << t << "s " << " *****" <<endl;
          printSolution = false;
        }
        if(print){
          print_solutions(BC);
        }
        out.push_back(BC);
      }
    }while(next_permutation( mask.rbegin(), mask.rend() ) );
  }
}

/***********************************************************************/

int main (int argc, char* argv[]) {
  
  string input_file ("sets.data");
  string output_file ("output.data");
  
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
  
  vector<set<int> > AA;
  
  AA = load_data(input_file);
  number_sets = AA.size();
  
  cout << border <<"Number of Sets: " << number_sets << endl;
  cout << "Total Number of Elements: " << rep.size() << endl;  
  cout << "Number of non repeated Elements: " << universe.size() << endl;  
  
  cout << "******************" 
       << " Preprocessing Sets "
       << "******************" << endl;
  startT = clock();
  dynamic_fast_preprocess(AA,universe);
  endT = clock();
  t = (double) (endT-startT) / CLOCKS_PER_SEC;
  
  cout << "Preprocess completed in " << t << "s"<< endl;
  cout << "Number of non repeated Elements after preprocess: " 
       << universeF.size() << endl << border;

  logn = floor ((log(number_sets) / log(2)));
  n = universeF.size();
  currentSize = 0;
  empty.resize(n,false);

  startT = clock();
  find_solutions();
  endT = clock();
  t = (double) (endT-startT) / CLOCKS_PER_SEC;
  cout << "All solutions found in " << t << "s"<< endl;
  
  save_data(output_file);
  cout << out.size() << " solutions found, saved to " << output_file 
       << endl << border;
  
  return 0;
 
}
