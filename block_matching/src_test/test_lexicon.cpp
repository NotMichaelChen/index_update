#include "libs/catch.hpp"

#include <iostream>
#include <fstream>

#include "index/lexicon.hpp"

TEST_CASE("Test getEntry", "[lexicon]") {
    Lexicon lex;

    std::string term = "abcd";
    Lex_data& entry = lex.getEntry(term);
    
    REQUIRE(entry.termid == 0);
    REQUIRE(entry.f_t == 0);

    entry = lex.getEntry(term);

    REQUIRE(entry.termid == 0);

    entry.f_t++;
    entry = lex.getEntry(term);
    REQUIRE(entry.termid == 0);
    REQUIRE(entry.f_t == 1);
}

TEST_CASE("Test dump/restore", "[lexicon]") {
    std::ifstream ifile("lexicon", std::ios::binary);
    if(!ifile)
        std::remove("lexicon");
    ifile.close();

    Lexicon lex;

    std::string term = "a";
    lex.getEntry(term);
    term = "b";
    lex.getEntry(term);
    term = "c";
    lex.getEntry(term);
    term = "d";
    lex.getEntry(term);


    std::ofstream ofile("lexicon", std::ios::binary);
    lex.dump(ofile);
    ofile.close();

    ifile.open("lexicon", std::ios::binary);
    lex.restore(ifile);
    ifile.close();

    term = "a";
    Lex_data& entry = lex.getEntry(term);
    REQUIRE(entry.termid == 0);
    term = "b";
    entry = lex.getEntry(term);
    REQUIRE(entry.termid == 1);
    term = "c";
    entry = lex.getEntry(term);
    REQUIRE(entry.termid == 2);
    term = "d";
    entry = lex.getEntry(term);
    REQUIRE(entry.termid == 3);
}