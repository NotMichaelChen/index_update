#include "morph.hpp"

#include <algorithm>
#include <random>

#include "util.hpp"

//Advances the iterator, obtaining a valid token and the whitespace that follows it
std::string getTokenWhitespace(std::string::iterator& iter, std::string::iterator end) {
    auto tokenstart = iter;

    while(iter != end && !std::isspace(*iter))
        iter++;
    
    while(iter != end && std::isspace(*iter))
        iter++;
    
    //This works since iter is pointing past the end of the string we want
    return std::string(tokenstart, iter);
}

DocumentMorpher::DocumentMorpher(std::string& from, std::string& to, int numversions) :
    olddoc(from), newdoc(to), versionsleft(numversions+1)
{
    std::istringstream oldstream(olddoc);
    std::istringstream newstream(newdoc);

    std::string discard;
    long oldpos = -1;
    long newpos = -1;
    while(oldstream >> discard && newstream >> discard) {
        oldpos = oldstream.tellg();
        newpos = newstream.tellg();
    }
    if(oldpos != -1 && newpos != -1) {
        if(!oldstream) {
            newdoc.resize(newpos);
        }
        else {
            olddoc.resize(oldpos);
        }
    }
}

std::string DocumentMorpher::getDocument() {
    return olddoc;
}

void DocumentMorpher::nextVersion() {
    if(versionsleft <= 1)
        return;

    //Represents the probability that we switch from old to new. 1-prob represents the probabilty of switching the other way
    double prob = 1.0 / versionsleft;

    std::string result;

    auto olditer = olddoc.begin();
    auto newiter = newdoc.begin();

    //skip whitespace in beginning
    while(olditer != olddoc.end() && std::isspace(*olditer)) {
        olditer++;
    }
    while(newiter != newdoc.end() && std::isspace(*newiter)) {
        newiter++;
    }

    //http://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<double> dis(0, 1.0);

    //State a = olddoc, state b = newdoc
    bool in_state_a = true;

    //while olditer/newiter are valid
    while(olditer != olddoc.end() && newiter != newdoc.end()) {
        //read token+whitespace pair
        std::string oldtok = getTokenWhitespace(olditer, olddoc.end());
        std::string newtok = getTokenWhitespace(newiter, newdoc.end());

        //choose whether to copy from olditer or newiter
        double outcome = dis(gen);
        //State change
        if(in_state_a) {
            if(outcome <= prob)
                in_state_a = false;
        }
        else {
            if(outcome <= 1-prob)
                in_state_a = true;
        }

        //append onto result
        result.append((in_state_a ? oldtok : newtok));
    }

    //copy result into olddoc
    olddoc = std::move(result);

    --versionsleft;
}

bool DocumentMorpher::isValid() {
    return versionsleft > 1;
}