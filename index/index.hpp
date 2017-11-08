#ifndef INDEX_HPP
#define INDEX_HPP

#include <vector>
#include <map>

#include "lexicon.hpp"
#include "posting.hpp"
#include "meta.hpp"
#include "extended_lexicon.hpp"
#include "Structures/documentstore.h"
#include "Structures/translationtable.h"

#define PDIR "./disk_index/positional/"//path to static positional index
#define NPDIR "./disk_index/non_positional/"//path to static non-positional index
#define BLOCK 128

//This index does not use compression
class Index {
public:
    Index();
    void insert_document(std::string& url, std::string& newpage);

    void write_np(int indexnum = 0, char prefix = 'a');
    void write_p(int indexnum, char prefix);
private:
    std::map<std::string, std::vector<Posting>> positional_index;
    std::map<std::string, std::vector<nPosting>> nonpositional_index;
    Lexicon lex;
    ExtendedLexicon exlex;
    Structures::DocumentStore docstore;
    Structures::TranslationTable transtable;

    std::vector<std::string> read_directory( std::string path );

    bool check_contain(std::vector<std::string> v, std::string f);

    template <typename T>
    void write(std::vector<T> num, std::ofstream& ofile);

    template <typename T>
    void compress_posting(std::string namebase, std::ofstream& ofile, T ite, int positional);

    std::vector<uint8_t> compress_field(std::vector<unsigned int>& field, int method, int delta);

    std::vector<Posting> decompress_p(std::string namebase, unsigned int termID);

    std::vector<nPosting> decompress_np(std::string namebase, unsigned int termID);

    std::vector<char> read_com(std::ifstream& infile, long end_pos);

    void merge_test();

    void merge_p(int indexnum);

    void merge_np(int indexnum);
};

#endif
