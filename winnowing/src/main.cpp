#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include "document_readers/WETreader.hpp"
#include "utility/morph.hpp"
#include "utility/util.hpp"
using namespace std;

const unsigned int B_WINNOW = 50;
const unsigned int W_WINNOW = 100;

template<typename T>
void printVec(const vector<T>& v) {
    for (const T& e : v) {
        cout << e << ' ';
    }
    cout << endl;
}

vector<size_t> hashFile(const vector<unsigned char>& file) {
    vector<size_t> hashResult(file.size() - B_WINNOW);
    hash<string> strHash;
    for (size_t i = 0; i < hashResult.size(); ++i) {
        string s_i = string(file.begin()+i, file.begin()+i+B_WINNOW-1);
        hashResult[i] = strHash(s_i);
    }
    return hashResult;
}

vector<size_t> cutFile(const vector<size_t>& hashedFile) {
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

vector<unsigned char> processFile(const string& origFile) {
    vector<unsigned char> processResult;
    hash<string> strHash;
    vector<string> words = Utility::splitString(origFile, " \n\t\r\f!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~");
    for(const string& w : words) {
        if (!Utility::isAlnum(w)) { continue; }
        processResult.push_back((unsigned char) strHash(w));
    }
    return processResult;
}

int main(int argc, char **argv) {
    WETReader wr("wet_files");
    vector<string> vs;
    int n = 1;
    while (n-- && wr.isValid()) {
        vs.push_back(wr.getCurrentDocument());
        wr.nextDocument();
    }
    // cout << vs.size() << endl;
    // printVec(vs);
    string testPage = vs[0];
    vector<unsigned char> processed = processFile(testPage);
    vector<size_t> hashed = hashFile(processed);
    vector<size_t> cut = cutFile(hashed);
    size_t lastCut = 0;
    size_t cutSz = 0;
    size_t cutNo = 0;
    for (size_t i = 0; i < cut.size(); ++i) {
        if (cut[i]) {
            cout << "Cut #" << (++cutNo) << ' ' << (i - lastCut) << endl;
            cutSz += (i - lastCut);
            lastCut = i;
        }
    }
    cout << processed.size() << endl;
    cout << cutSz / cutNo << endl;


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

    // vector<size_t> testHashedFile = {23, 17, 45, 13, 48, 13, 48, 87, 19, 7, 21, 12, 29, 13};
    // vector<size_t> cuts = cutFile(testHashedFile);
    // vector<size_t> expectedCuts = {0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0};
    // assert(cuts == expectedCuts);



    return 0;
}
