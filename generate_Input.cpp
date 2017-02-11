#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <set>

using namespace std;


/*
A1 = [ 1 2 9 10 11 12 13 14 ]
A2 = [ 3 4 7 8 11 12 13 14 ]
A3 = [ 5 6 7 8 9 10 11 12 ]
*/


int main (int argc, char* argv[]) {
    
    int sets = 0;
    
    if(argc>1){
        sets = atoi(argv[1]);
    }
    
    string file_name ("sets.data");
    ofstream outFile;
    outFile.open (file_name.c_str(), ifstream::out);
    
    srand(time(NULL));
    
    for(int i=0;i<sets;i++){
        outFile << "A" << i+1  << " = [ ";
        unsigned cardinality = rand()%200+100;   //100 min, 300 max
        set<int> A;
        while(A.size() < cardinality){
            A.insert(rand()%9999+1); // range 1 --> 10000, 0 not allowed
        }
        for(set<int>::iterator it=A.begin(); it!=A.end(); ++it){
            outFile << *it << " ";
        }
        outFile << "]" << endl;
    }
    outFile.close();
    
return 0;
}
