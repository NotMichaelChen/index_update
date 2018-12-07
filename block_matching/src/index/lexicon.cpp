#include "lexicon.hpp"

#include "static_functions/bytesIO.hpp"

using namespace std;

Lexicon::Lexicon() : nextID(0) {}

Lex_data& Lexicon::getEntry(string& term) {
    auto iter = lex.find(term);
    if(iter == lex.end()) {
        iter = initEntry(term);
    }
    return iter->second;
}

//term must *NOT* exist inside of the lexicon already
spp::sparse_hash_map<std::string, Lex_data>::iterator Lexicon::initEntry(string& term) {
    auto results = lex.emplace(term, Lex_data{nextID, 0});
    nextID++;
    return results.first;
}

void Lexicon::dump(ofstream& ofile) {
    writeAsBytes(nextID, ofile);
    for(auto iter = lex.begin(); iter != lex.end(); ++iter) {
        // Use '\n' as the terminating character since we split on it during document processing
        ofile << iter->first << '\n';
        writeAsBytes(iter->second.termid, ofile);
        writeAsBytes(iter->second.f_t, ofile);
    }
}

void Lexicon::restore(ifstream& ifile) {
    if(!ifile)
        return;
    readFromBytes(this->nextID, ifile);

    std::string term;
    unsigned int termid;
    int f_t;
    while(std::getline(ifile, term)) {
        readFromBytes(termid, ifile);
        readFromBytes(f_t, ifile);
        lex.emplace(term, Lex_data{termid, f_t});
    }
}

void Lexicon::clear() {
    lex.clear();
    nextID = 0;
}

size_t Lexicon::getSize() {
    return lex.size();
}