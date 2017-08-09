#include <vector>
#include "meta.hpp"
using namespace std;

Lstptr::Lstptr(vector<mDatanp> np, Indexer& ind){

    string namebase = np.filename;
    long start_pos = np.start_pos;
    long end_pos = np.end_pos;
    long meta_doc_start = np.meta_doc_start;
    long

    last_id = 
    doc_block_size = ind.decompress_np(namebase, meta_doc_start, meta_freq_start);
    freq_block_size = ind.decompress_np(namebase, meta_freq_start, posting_start);



    dsizeit = doc_block_size.begin();
    lidit = last_id.begin();
}
