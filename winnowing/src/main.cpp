#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include "document_readers/WETreader.hpp"
#include "utility/morph.hpp"
using namespace std;

template<typename T>
void printVec(const vector<T>& v) {
    for (const T& e : v) {
        cout << e << ' ';
    }
    cout << endl;
}

const unsigned int B_WINNOW = 3;
const unsigned int W_WINNOW = 5;
vector<size_t> cutFile(const vector<size_t>& hashedFile) {
    // size_t hSize = file.size() - B_WINNOW;
    // vector<size_t> H(hSize);
    // std::hash<unsigned char> hashFunc;
    // for (size_t i = 0; i < hSize; ++i) {
    //     H[i] = hashFunc(file[i]);
    // }

    vector<size_t> cutResults(hashedFile.size(), 0);
    for (size_t w = 0; w < cutResults.size() - W_WINNOW; ++w) {
        vector<size_t> possibleCuts;
        for (size_t i = w; i < w + W_WINNOW; ++i) {
            bool cut = true;
            for (size_t j = w; j < w + W_WINNOW; ++j) {
                if (i == j) { continue; }
                if (hashedFile[i] > hashedFile[j]) {
                    cut = false;
                    break;
                }
            }
            if (cut) { possibleCuts.push_back(i); }
        }
        if (possibleCuts.size() == 1) { cutResults[possibleCuts[0]] = 1; }
        else {
            bool alreadyCut = false;
            for (size_t c : possibleCuts) {
                if (cutResults[c] == 1) {
                    alreadyCut = true;
                }
            }
            if (!alreadyCut) { cutResults[possibleCuts.back()] = 1; }
        }
    }

    return cutResults;
}

int main(int argc, char **argv) {
    // WETReader wr("wet_files");
    // vector<string> vs;
    // int n = 2;
    // while (n-- && wr.isValid()) {
    //     vs.push_back(wr.getCurrentDocument());
    //     wr.nextDocument();
    // }
    // cout << vs.size() << endl;

    // DocumentMorpher dm(vs[0], vs[1], 100);
    // while (dm.isValid()) {
    //     dm.nextVersion();
    //     vs.push_back(dm.getDocument());
    // }

    // // for (auto d : vs) {
    // //     cout << d << endl;
    // //     cout << "==============" << endl;
    // // }
    // cout << vs.size() << endl;

    vector<size_t> testHashedFile = {23, 17, 45, 13, 48, 13, 48, 87, 19, 7, 21, 12, 29, 13};
    vector<size_t> cuts = cutFile(testHashedFile);
    vector<size_t> expectedCuts = {0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0};
    assert(cuts == expectedCuts);

    return 0;
}
