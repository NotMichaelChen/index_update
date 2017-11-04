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

	//std::map<std::string, std::vector<f_meta>, strless> filemeta; //extended lexicon

    Lexicon lex;
    ExtendedLexicon exlex;
    Structures::DocumentStore docstore;
    Structures::TranslationTable transtable;


    std::vector<std::string> read_directory( std::string path );

    bool check_contain(std::vector<std::string> v, std::string f);

    void write(std::vector<uint8_t> num, std::ofstream& ofile);

    mDatap compress_p(std::string namebase, std::ofstream& ofile,
        std::vector<unsigned int>& v_docID,
        std::vector<unsigned int>& v_fragID,
        std::vector<unsigned int>& v_pos);

    mDatanp compress_np(std::string namebase, std::ofstream& ofile,
    	std::vector<unsigned int>& v_docID,
    	std::vector<unsigned int>& v_freq);

    std::vector<uint8_t> compress(std::vector<unsigned int>& field, int method, int sort,
        std::vector<uint8_t> &meta_data_biv,
        std::vector<unsigned int> &last_id_biv);

    std::vector<uint8_t> compress(std::vector<unsigned int>& field, int method, int sort, std::vector<uint8_t> &meta_data_biv);

    std::vector<uint8_t> compress_freq(std::vector<unsigned int>& field, int method, int sort, std::vector<uint8_t> &meta_data_biv);

    std::vector<Posting> decompress_p(std::string namebase, unsigned int termID);

    std::vector<nPosting> decompress_np(std::string namebase, unsigned int termID);

    std::vector<char> read_com(std::ifstream& infile, long end_pos);

    void merge_test();

    void merge_p(int indexnum);

    void merge_np(int indexnum);
};

#endif
