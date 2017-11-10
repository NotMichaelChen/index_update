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

typedef std::map<unsigned int, std::vector<Posting>>::iterator P_ITE;
typedef std::map<unsigned int, std::vector<nPosting>>::iterator NP_ITE;
typedef std::vector<Posting>::iterator P_V;
typedef std::vector<nPosting>::iterator NP_V;

//This index does not use compression
class Index {
public:
    Index();
    void insert_document(std::string& url, std::string& newpage);

    void write_np(int indexnum = 0, char prefix = 'a');
    void write_p(int indexnum = 0, char prefix = 'a');
private:
    std::map<unsigned int, std::vector<Posting>> positional_index;
    std::map<unsigned int, std::vector<nPosting>> nonpositional_index;
    Lexicon lex;
    ExtendedLexicon exlex;
    Structures::DocumentStore docstore;
    Structures::TranslationTable transtable;

    std::vector<std::string> read_directory( std::string path );

    bool check_contain(std::vector<std::string> v, std::string f);

    template <typename T>
    void write(std::vector<T> num, std::ofstream& ofile);

    template <typename T1, typename T2>
    void compress_posting(std::string namebase, std::ofstream& ofile, T1& ite, T1& end, T2& vit, T2& vend, int positional);

    std::vector<uint8_t> compress_field(std::vector<unsigned int>& field, int method, int delta);

    void decompress_p_posting(unsigned int termID, std::ifstream& ifile, std::string namebase);

    std::vector<char> read_com(std::ifstream& infile, long end_pos);

    void merge_test();

    void merge(int indexnum, int positional);

    void decompress_np_posting(unsigned int termID, std::ifstream& filez,
        std::ifstream& filei, std::string namebase1, std::string namebase2);
};

#endif
