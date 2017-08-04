#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <map>
#include <string>
#include <cstdio>
#include <dirent.h>
#include <utility>

#include "reader.hpp"
#include "posting.hpp"
#include "meta.hpp"
#include "compressor.hpp"
#include "strless.hpp"
#include "comparison.hpp"

#define NO_DOC 10 //temporary use
#define POSTING_LIMIT 500 //make sure doesn't exceed memory limit
#define INDEX "./test_data/compressedIndex"
#define INFO "./test_data/docInfo"
#define PDIR "./disk_index/positional/"
#define NPDIR "./disk_index/non_positional/"

using namespace std;

int main(){
	Compressor comp;
    map<string, unsigned int> lexical;
	map<unsigned int, std::pair<vector<mData>, vector<mDatanp>>> dict;
	map<string, vector<f_meta>, strless> filemeta;
	comp.start_compress(filemeta, dict);

    for( map<unsigned int, pair<vector<mData>, vector<mDatanp>>>::iterator it = dict.begin(); it != dict.end(); it ++){
        cout << it->first << endl;
        vector<mData> vec = it->second.first;
        for( vector<mData>::iterator ite = vec.begin(); ite != vec.end(); ite ++){
            cout << ite->filename << ' ' << ite->start_pos << ' ';
        }
        cout << endl;
    }

    for( map<string, vector<f_meta>, strless>::iterator it = filemeta.begin(); it != filemeta.end(); it++){
        cout << it->first << endl;
        vector<f_meta> vec = it->second;
        for( vector<f_meta>::iterator ite = vec.begin(); ite != vec.end(); ite++){
            cout << ite->termID << ' ';
        }
        cout << endl;
    }
    /*Querior q;
    string str;
    cin >> str;
    q.query(str, lexical);
    */

	return 0;
}
