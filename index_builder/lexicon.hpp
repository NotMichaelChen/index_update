/**
 * Lexicon is used to look up termID given a term. Thus, lexicon uses term as key, termID as value.
 * Dictionary in the indexer can also be called extended lexicon. It uses termID as key.
 */

#ifndef LEXICON_H
#define LEXICON_H

#include <string>
#include <vector>
#include <sstream>

struct Lex_data {
    unsigned int termid;
    //How many documents the term appeared in
    int f_t;
    //Where in the dynamic index the term appears in
    long p_ptr;
    long np_ptr;
};

class Lexicon {
public:
    Lexicon();

    Lex_data getEntry(std::string& term);
    void updateFreq(std::string& term, int delta);
    void updatePositional(std::string& term, long pos);
    void updateNonPositional(std::string& term, long pos);

    //Dumps contents of lexicon into file called "lexdump"
    void dump();
    //Restores the lexicon from a file called "lexdump" on disk
    //Returns true if restored from file
    bool restore();

private:
    void initEntry(std::string& term);
    std::map<std::string, Lex_data> lex;
    unsigned int nextID;
};

#endif
