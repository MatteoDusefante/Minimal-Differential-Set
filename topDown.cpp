/*********************************************************************
 * Top-down approach with boost::dynamic_bitset and fast preprocess  *
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
bool print = false, sol = false;

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

boost::dynamic_bitset<> check_minimality(boost::dynamic_bitset<> B){
  
  bool minimality;
  for(unsigned k=0;k<B.size();++k){
    if(B[k]){
      B[k] = 0;
      minimality = true;
      for(unsigned i=0;i<number_sets;++i){
        if((bitAA[i] & B) == empty){
          minimality = false;
          break;
        }
        if(minimality){
          for(unsigned j=0;j<number_sets;++j){
            if(i != j){
              if((bitAA[i] & B) == (bitAA[j] & B)){
                minimality = false;
                break;
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

/***********************************************************************/

void find_solutions(vector<boost::dynamic_bitset<> > bitAA, 
                      boost::dynamic_bitset<> B, unsigned m){
                        
  if(m>=number_sets){
    boost::dynamic_bitset<> M = check_minimality(B);
    if(M.count()<currentSize){
      currentSize = M.count();
      bestT = clock();
      t = (double) (bestT-startT) / CLOCKS_PER_SEC;
      cout << "** Best Solution " << setw(6)
           << currentSize << " elements. Time " 
           << setw(12) << t << "s " << " **"<< endl;
      out.clear();
    }
    if(print){
      print_solutions(M);
    }
    out.push_back(M);
    return;
  }else if((m==0) || ((bitAA[m] & B) == empty)){
    for(unsigned i=0;i<n;++i){
      if(bitAA[m][i]) {
        B[i] = 1;
        if(B.count()>currentSize)
          return;
        find_solutions(bitAA,B,m+1);
        B[i] = 0;
      }
    }
  }else{ 
    sol = true;
    for(unsigned j=0;j<m;++j){
      if(j!=m){
        if((bitAA[m] & B) == (bitAA[j] & B)){
          sol = false;
          boost::dynamic_bitset<> C = bitAA[m] ^ bitAA[j];
          for(unsigned i=0;i<n;++i){
            if(C[i] == 1){
              B[i] = 1;
              if(B.count()>currentSize)
                return;
              find_solutions(bitAA,B,m+1);
              B[i] = 0;
            }
          }
        }
      }
    }
    if(sol){
      sol = false;
      find_solutions(bitAA,B,m+1);
    }
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
  boost::dynamic_bitset<> B;
  
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
  
  n = universeF.size();
  currentSize = number_sets+1;
  empty.resize(n,0);
  B.resize(n,0);
  
  /* Sets random permutation */
  //srand(time(NULL));
  //random_shuffle(bitAA.begin(),bitAA.end());

  startT = clock();
  find_solutions(bitAA,B,0);
  endT = clock();
  t = (double) (endT-startT) / CLOCKS_PER_SEC;
  cout << "All solutions found in " << t << "s"<< endl;
  
  cout << "***************" 
       << " PostProcessing Solutions " 
       << "***************" << endl;

  sort(out.begin(),out.end());
  out.erase(unique(out.begin(),out.end()),out.end());
  save_data(output_file);
  cout << out.size() << " solutions found, saved to " << output_file 
       << endl << border;
  
  return 0;
 
}
