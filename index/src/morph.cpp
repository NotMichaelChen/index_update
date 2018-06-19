#include "morph.hpp"

#include <algorithm>
#include <random>

DocumentMorpher::DocumentMorpher(std::string from, std::string to, int numversions) : versioncount(numversions), versionsleft(numversions+1) {
    //Shrink one of the documents so that they are the same length
    if(from.length() != to.length()) {
        size_t shorterlen = std::min(from.length(), to.length());
        std::string& longerstr = from.length() > to.length() ? from : to;
        longerstr.resize(shorterlen);
    }

    oldstream = std::stringstream(from);
    newstream = std::stringstream(to);
}

//Returns an empty string if there are no more valid versions to generate
std::string DocumentMorpher::getVersion() {
    if(versionsleft <= 1)
        return "";

    //Represents the probability that we switch from old to new. 1-prob represents the probabilty of switching the other way
    double prob = 1.0 / versionsleft;

    std::string result;

    //http://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<double> dis(0, 1.0);

    std::string oldword, newword;
    //State a = olddoc, state b = newdoc
    bool in_state_a = true;
    while(oldstream >> oldword && newstream >> newword)
    {
        double outcome = dis(gen);
        //State change
        if(in_state_a)
        {
            if(outcome <= prob)
                in_state_a = false;
        }
        else
        {
            if(outcome <= 1-prob)
                in_state_a = true;
        }

        result.append((in_state_a ? oldword : newword) + " ");
    }

    oldstream = std::stringstream(result);
    --versionsleft;

    return result;
}

bool DocumentMorpher::isValid() {
    return versionsleft > 1;
}