/* Methods related to compressing posting list */
#include <vector>
#include <string>

#include "compressor.hpp"
#include "varbyte.hpp"

using namespace std;

void write(vector<uint8_t> num, ofstream& ofile){
	/**
	 * Write the compressed posting to file byte by byte.
	 */
	for(vector<uint8_t>::iterator it = num.begin(); it != num.end(); it++){
		ofile.write(reinterpret_cast<const char*>(&(*it)), 1);
	}
}

mData compress_p(string namebase, ofstream& ofile,
    std::vector<unsigned int>& v_docID,
    std::vector<unsigned int>& v_fragID,
    std::vector<unsigned int>& v_pos){
	/**
	 * Write compressed positional postings to disk and store the corresponding start
	 * and end position to metadata of the term.
	 */
	string filename = string(PDIR) + namebase;
	int method = 1;// 1: Variable Bytes Encoding

	std::vector<unsigned int> v_last_id;
	std::vector<uint8_t> docID_biv;
	std::vector<uint8_t> fragID_biv;
	std::vector<uint8_t> pos_biv;

	std::vector<uint8_t> last_id_biv;
	std::vector<uint8_t> size_doc_biv;
	std::vector<uint8_t> size_frag_biv;
	std::vector<uint8_t> size_pos_biv;

	docID_biv = compress(v_docID, method, 1, size_doc_biv, v_last_id);
	last_id_biv = VBEncode(v_last_id);

	fragID_biv = compress(v_fragID, method, 0, size_frag_biv);
	pos_biv = compress(v_pos, method, 0, size_pos_biv);

	mData meta;
    meta.filename = namebase;
	meta.comp_method = method;

    meta.start_pos = ofile.tellp();
	write(last_id_biv, ofile);

	meta.meta_doc_start = ofile.tellp();
	write(size_doc_biv, ofile);

	meta.meta_frag_start = ofile.tellp();
	write(size_frag_biv, ofile);

	meta.meta_pos_start = ofile.tellp();
	write(size_pos_biv, ofile);

	meta.posting_start = ofile.tellp();
	write(docID_biv, ofile);

	meta.frag_start = ofile.tellp();
	write(fragID_biv, ofile);

	meta.pos_start = ofile.tellp();
	write(pos_biv, ofile);

    meta.end_pos = ofile.tellp();

	return meta;
}

mDatanp compress_np(string namebase, ofstream& ofile,
	std::vector<unsigned int>& v_docID,
	std::vector<unsigned int>& v_freq,
	std::vector<unsigned int>& v_sign){
	/**
	 * Writing compressed non-positional postings to disk and store the starting and ending positions.
	 * Similar to compress_p.
	 */

	string filename = string(NPDIR) + namebase;
	int method = 1;

	std::vector<unsigned int> v_last_id;
	std::vector<uint8_t> docID_biv;
	std::vector<uint8_t> freq_biv;
    std::vector<uint8_t> sign_biv;

	std::vector<uint8_t> last_id_biv;
	std::vector<uint8_t> size_doc_biv;
	std::vector<uint8_t> size_freq_biv;

	docID_biv = compress(v_docID, method, 1, size_doc_biv, v_last_id);
	last_id_biv = VBEncode(v_last_id);

	freq_biv = compress_freq(v_freq, method, 0, size_freq_biv);

    //compress sign vector
    vector<unsigned int>::iterator it = v_sign.begin();
    int num;
    uint8_t b;
    bitset<8> byte;
    while( it != v_sign.end() ){
        num = 0;
        while(num != 8 && it != v_sign.end()){
            if( *it == 1 )
                byte.flip( num );
            num ++;
            it ++;
        }
        b = byte.to_ulong();
        sign_biv.push_back(b);
    }

	mDatanp meta;
    meta.filename = namebase;
	meta.comp_method = method;

    meta.start_pos = ofile.tellp();
	write(last_id_biv, ofile);

	meta.meta_doc_start = ofile.tellp();
	write(size_doc_biv, ofile);

	meta.meta_freq_start = ofile.tellp();
	write(size_freq_biv, ofile);

	meta.posting_start = ofile.tellp();
	write(docID_biv, ofile);

	meta.freq_start = ofile.tellp();
	write(freq_biv, ofile);

	meta.sign_start = ofile.tellp();
	write(sign_biv, ofile);

    meta.end_pos = ofile.tellp();

	return meta;
}

vector<uint8_t> compress(std::vector<unsigned int>& field, int method, int sort, vector<uint8_t> &meta_data_biv, vector<unsigned int> &last_id_biv){
	/**
	 * Compress the document ID field and store the last ID of each block to last_id_biv,
	 * and store the length of each block to meta_data_biv.
	 */
	std::vector<unsigned int> block;
	std::vector<unsigned int>::iterator it = field.begin();
	std::vector<uint8_t> field_biv;
	std::vector<uint8_t> biv;

	if(method){
		unsigned int prev = 0;
		int size_block;
		while(it != field.end()){
			size_block = 0;
			block.clear();

			while(size_block < 64 && it != field.end()){
				block.push_back(*it - prev);
				prev = *it;
				size_block ++;
				it ++;
			}
			biv = VBEncode(block);
			last_id_biv.push_back(prev);//the last element of every block needs to be stored
			field_biv.insert(field_biv.end(), biv.begin(), biv.end());
			meta_data_biv.push_back(biv.size());//meta data stores the number of bytes after compression
		}
		return field_biv;
	}
	else{
		return field_biv;
	}
}

vector<uint8_t> compress(std::vector<unsigned int>& field, int method, int sort, vector<uint8_t> &meta_data_biv){
	/**
	 * Compress fragment ID and positions, and store the length of each block to meta_data_biv.
	 * meta_data_biv here is acutally not a binary vector.
	 * TODO: the meta_data_biv should not be binary vector, instead it should be vector of unsigned int.
	 */
	std::vector<unsigned int> block;
	std::vector<unsigned int>::iterator it = field.begin();
	std::vector<uint8_t> field_biv;
	std::vector<uint8_t> biv;

	if(method){
		int prev;
		int size_block;
		while(it != field.end()){
			size_block = 0;
			block.clear();
			prev = 0;//the first element of every block needs to be renumbered

			while(size_block < 64 && it != field.end()){
				block.push_back(*it - prev);
				prev = *it;
				size_block ++;
				it ++;
			}
			biv = VBEncode(block);

			field_biv.insert(field_biv.end(), biv.begin(), biv.end());
			meta_data_biv.push_back(biv.size());//meta data stores the number of bytes after compression
		}
		return field_biv;
	}
	else{
		return field_biv;
	}
}

vector<uint8_t> compress_freq(std::vector<unsigned int>& field, int method, int sort, vector<uint8_t> &meta_data_biv){
	/* Compress a field which doesn't require delta coding.
	 * In this case, frequency. Frequency cannot be delta coded because it is not strictly increasing.
	 */
	std::vector<unsigned int> block;
	std::vector<unsigned int>::iterator it = field.begin();
	std::vector<uint8_t> field_biv;
	std::vector<uint8_t> biv;

	if(method){

		int size_block;
		while(it != field.end()){
			size_block = 0;
			block.clear();

			while(size_block < 64 && it != field.end()){
                //cout <<"Freq " <<  *it << endl;
				block.push_back(*it);
				size_block ++;
				it ++;
			}
			biv = VBEncode(block);

			field_biv.insert(field_biv.end(), biv.begin(), biv.end());
			meta_data_biv.push_back(biv.size());//meta data stores the number of bytes after compression
		}
		return field_biv;
	}
	else{
		return field_biv;
	}
}

//TODO: decompress to positional index not a vector of posting
std::vector<Posting> decompress_p(string namebase, unsigned int termID){
	/**
	 * Decompress positional postings.
	 * First read each field from disk and pass the bianry vector to decode.
	 * Finally add the terms up since we used delta encoding before.
	 */
	Reader r;

	ifstream ifile;
    string filename = string(PDIR) + namebase;
	ifile.open(filename, ios::binary);
    vector<Posting> result;

    if(ifile.is_open()){
        //cout << namebase << " Opened for Decompressing" << endl;
    	char c;
    	vector<char> readin;

    	vector<unsigned int> docID;
    	vector<unsigned int> fragID;
    	vector<unsigned int> pos;

        vector<mData>& mvec = dict[termID].first;
        vector<mData>::iterator currMVec;

        for( currMVec = mvec.begin(); currMVec != mvec.end(); currMVec ++){
            if( currMVec->filename == namebase )
                break;
        }

    	ifile.seekg(currMVec->posting_start);
    	while(ifile.tellg() != currMVec->frag_start){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        docID = r.VBDecode(readin);
        readin.clear();

    	ifile.seekg(currMVec->frag_start);
    	while(ifile.tellg() != currMVec->pos_start){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        fragID = r.VBDecode(readin);
        readin.clear();

    	ifile.seekg(currMVec->pos_start);
    	while(ifile.tellg() != currMVec->end_pos){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        pos = r.VBDecode(readin);
        readin.clear();

    	vector<unsigned int>::iterator itdoc = docID.begin();
    	vector<unsigned int>::iterator itfrag = fragID.begin();
    	vector<unsigned int>::iterator itpos = pos.begin();

        unsigned int prevdoc =0;
        unsigned int prevfrag = 0;
        unsigned int prevpos = 0;

        int posting_num = 0;

    	while(itdoc != docID.end()){
            if(posting_num == 64){
                //keep track of block
                prevfrag = 0;
                prevpos = 0;
                posting_num = 0;
            }

            Posting p(termID, (*itdoc + prevdoc), (prevfrag+*itfrag), (prevpos+*itpos));
            //cout << termID << ' ' <<*itdoc << ' '<< *itfrag << ' ' << *itpos << endl;
            prevdoc = *itdoc + prevdoc;
            prevfrag = *itfrag + prevfrag;
            prevpos = *itpos + prevpos;
            result.push_back(p);
            itdoc ++;
            itfrag ++;
            itpos ++;
            posting_num ++;

    	}
    }else{
        cerr << "File cannot be opened." << endl;
    }
    return result;
}

std::vector<nPosting> decompress_np(string namebase, unsigned int termID){
    //cout << "Decompressing np" << endl;
	Reader r;
	ifstream ifile;
    string filename = string(NPDIR) + namebase;
	ifile.open(filename, ios::binary);
    vector<nPosting> result;

    if(ifile.is_open()){
        //cout << namebase << " Opened for Decompressing" << endl;
    	char c;
    	vector<char> readin;

    	vector<unsigned int> docID;
    	vector<unsigned int> freq;
    	vector<unsigned int> sign;

        vector<mDatanp>& mvec = dict[termID].second;
        vector<mDatanp>::iterator currMVec;

        for( currMVec = mvec.begin(); currMVec != mvec.end(); currMVec ++){
            //cout << currMVec->filename << endl;
            //cout << currMVec->posting_start << ' ' << currMVec->frag_start << ' ' << currMVec->pos_start << endl;
            if( currMVec->filename == namebase )
                break;
        }

    	ifile.seekg(currMVec->posting_start);
    	while(ifile.tellg() != currMVec->freq_start){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        docID = r.VBDecode(readin);
        readin.clear();

    	ifile.seekg(currMVec->freq_start);
    	while(ifile.tellg() != currMVec->sign_start){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        freq = r.VBDecode(readin);
        readin.clear();

    	ifile.seekg(currMVec->sign_start);
    	while(ifile.tellg() != currMVec->end_pos){
            //cout << ifile.tellg() << endl;
            //cout << currMVec->end_pos << endl;
    		ifile.get(c);
    		readin.push_back(c);
    	}
        //decompress sign

        for( vector<char>::iterator it = readin.begin(); it != readin.end(); it ++ ){
            bitset<8> byte(*it);
            unsigned int ui;
            for( int i = 0; i < 8; i ++ ){
                ui = byte[i];
                sign.push_back(ui);
            }
        }
        readin.clear();

    	vector<unsigned int>::iterator itdoc = docID.begin();
    	vector<unsigned int>::iterator itfreq = freq.begin();
    	vector<unsigned int>::iterator itsign = sign.begin();

        unsigned int prevdoc =0;
        //cout << "In " << filename << ", there are "<< endl;
    	while(itdoc != docID.end()){
            nPosting p(termID, (*itdoc + prevdoc), *itfreq, *itsign);
            //cout << termID << ' ' << *itdoc + prevdoc << ' '<< *itfreq << ' ' << *itsign << endl;
            prevdoc = *itdoc + prevdoc;
            result.push_back(p);
            itdoc ++;
            itfreq ++;
            itsign ++;
    	}
    }else{
        cerr << "File cannot be opened." << endl;
    }
    return result;
}
