#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#include "builder.hpp"
#include "strless.hpp"
using namespace std;

typedef map<string, unsigned int, strless> lexmap;

void Builder::build_lexical(lexmap lex){
    int count = 0;
    string line;
    string term;
    string ID;
    ifstream ifile;
    ifile.open("./test_data/termIDs");

    while(getline(ifile, line)){
        count ++;
        stringstream lineStream(line);
        lineStream >> ID >> term;
        lex[term] = stoi(ID);
        cout << '\r' << count << " Finished.";
        if( count == 1000000) break;
    }
}

void Builder::display_lexical(lexmap lex){
    for(lexmap::iterator it = lex.begin(); it != lex.end(); ++it){
        cout << it->first << ' ' << it->second << endl;
    }
}
