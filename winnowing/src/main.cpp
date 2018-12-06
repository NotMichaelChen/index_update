#include <iostream>
#include <vector>
#include <string>
#include "document_readers/WETreader.hpp"
#include "utility/morph.hpp"
using namespace std;

int main(int argc, char **argv) {
    WETReader wr("wet_files");
    vector<string> vs;
    int n = 2;
    while (n-- && wr.isValid()) {
        vs.push_back(wr.getCurrentDocument());
        wr.nextDocument();
    }
    cout << vs.size() << endl;

    DocumentMorpher dm(vs[0], vs[1], 100);
    while (dm.isValid()) {
        dm.nextVersion();
        vs.push_back(dm.getDocument());
    }

    // for (auto d : vs) {
    //     cout << d << endl;
    //     cout << "==============" << endl;
    // }
    cout << vs.size() << endl;
    return 0;
}
