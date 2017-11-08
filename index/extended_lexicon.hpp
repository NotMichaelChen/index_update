#ifndef EXTLEX_H
#define EXTLEX_H

#include <string>
<<<<<<< HEAD
#include <map>
=======
#include <unordered_map>
>>>>>>> e1bf2a4af59bd032b60f645a07dabce190fba3d9
#include <vector>
#include "meta.hpp"

class ExtendedLexicon {
public:
<<<<<<< HEAD
    void addPositional(std::string term, mData entry);
    void addNonPositional(std::string term, mData entry);
private:
    std::map<std::string, std::vector<mData>> exlexnp;
    std::map<std::string, std::vector<mData>> exlexp;
=======
    void addPositional(unsigned int term, mDatap& entry);
    void addNonPositional(unsigned int term, mDatanp& entry);

    mDatap getPositional(unsigned int term, int index);
    mDatanp getNonPositional(unsigned int term, int index);

    std::vector<mDatap>::iterator getPositionalBegin(unsigned int term);
    std::vector<mDatap>::iterator getPositionalEnd(unsigned int term);
    std::vector<mDatanp>::iterator getNonPositionalBegin(unsigned int term);
    std::vector<mDatanp>::iterator getNonPositionalEnd(unsigned int term);

private:
    std::unordered_map<unsigned int, std::vector<mDatanp>> exlexnp;
    std::unordered_map<unsigned int, std::vector<mDatap>> exlexp;
>>>>>>> e1bf2a4af59bd032b60f645a07dabce190fba3d9
};

#endif
