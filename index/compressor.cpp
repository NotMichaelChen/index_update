/* Methods related to compressing posting list */
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <dirent.h>

#include "index.hpp"
#include "varbyte.hpp"

#define PDIR "./disk_index/positional/"//path to static positional index
#define NPDIR "./disk_index/non_positional/"//path to static non-positional index

std::vector<std::string> Index::read_directory( std::string path ){
    /**
     * List all the files in a directory.
     *
     * @param: the path of a directory
     * @return: the names of all the files in this directory.
     */
    std::vector <std::string> result;
    dirent* de;
    DIR* dp;
    errno = 0;
    dp = opendir( path.empty() ? "." : path.c_str() );
    if (dp){
    		while (true){
      			errno = 0;
      			de = readdir( dp );
      			if (de == NULL) break;
                std::string d_n(de->d_name);
      			result.push_back( d_n );
      		}
    		closedir( dp );
        }
    return result;
}

bool Index::check_contain(std::vector<std::string> v, std::string f){
    //check if a vector contains an element
    for( std::vector<std::string>::iterator it = v.begin(); it != v.end(); it ++){
        if( *it == f) return true;
    }
    return false;
}

void Index::write(std::vector<uint8_t> num, std::ofstream& ofile){
    /**
     * Write the compressed posting to file byte by byte.
     */
    for(std::vector<uint8_t>::iterator it = num.begin(); it != num.end(); it++){
    ofile.write(reinterpret_cast<const char*>(&(*it)), 1);
    }
}

mDatap Index::compress_p(std::string namebase, std::ofstream& ofile,
    std::vector<unsigned int>& v_docID,
    std::vector<unsigned int>& v_fragID,
    std::vector<unsigned int>& v_pos){
    /**
     * Write compressed positional postings to disk and store the corresponding start
     * and end position to metadata of the term.
     */
    std::string filename = std::string(PDIR) + namebase;
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

    mDatap meta;
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

mDatanp Index::compress_np(std::string namebase, std::ofstream& ofile,
    std::vector<unsigned int>& v_docID,
    std::vector<unsigned int>& v_freq){
    /**
     * Writing compressed non-positional postings to disk and store the starting and ending positions.
     * Similar to compress_p.
     */

    std::string filename = std::string(NPDIR) + namebase;
    int method = 1;

    std::vector<unsigned int> v_last_id;
    std::vector<uint8_t> docID_biv;
    std::vector<uint8_t> freq_biv;

    std::vector<uint8_t> last_id_biv;
    std::vector<uint8_t> size_doc_biv;
    std::vector<uint8_t> size_freq_biv;

    docID_biv = compress(v_docID, method, 1, size_doc_biv, v_last_id);
    last_id_biv = VBEncode(v_last_id);

    freq_biv = compress_freq(v_freq, method, 0, size_freq_biv);

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

    meta.end_pos = ofile.tellp();

    return meta;
}

std::vector<uint8_t> Index::compress(std::vector<unsigned int>& field, int method, int sort, std::vector<uint8_t> &meta_data_biv, std::vector<unsigned int> &last_id_biv){
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

std::vector<uint8_t> Index::compress(std::vector<unsigned int>& field, int method, int sort, std::vector<uint8_t> &meta_data_biv){
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

std::vector<uint8_t> Index::compress_freq(std::vector<unsigned int>& field, int method, int sort, std::vector<uint8_t> &meta_data_biv){
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
                //std::cout <<"Freq " <<  *it << std::endl;
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
std::vector<Posting> Index::decompress_p(std::string namebase, unsigned int termID){
    /**
     * Decompress positional postings.
     * First read each field from disk and pass the bianry vector to decode.
     * Finally add the terms up since we used delta encoding before.
     */

    std::ifstream ifile;
    std::string filename = std::string(PDIR) + namebase;
    ifile.open(filename, std::ios::binary);
    std::vector<Posting> result;

    if(ifile.is_open()){
        //std::cout << namebase << " Opened for Decompressing" << std::endl;
    	char c;
    	std::vector<char> readin;

    	std::vector<unsigned int> docID;
    	std::vector<unsigned int> fragID;
    	std::vector<unsigned int> pos;

        std::vector<mDatap>::iterator currMVec;

        for( currMVec = exlex.getPositionalBegin(termID); currMVec != exlex.getPositionalEnd(termID); currMVec++){
            if( currMVec->filename == namebase )
                break;
        }

    	ifile.seekg(currMVec->posting_start);
    	while(ifile.tellg() != currMVec->frag_start){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        docID = VBDecode(readin);
        readin.clear();

    	ifile.seekg(currMVec->frag_start);
    	while(ifile.tellg() != currMVec->pos_start){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        fragID = VBDecode(readin);
        readin.clear();

    	ifile.seekg(currMVec->pos_start);
    	while(ifile.tellg() != currMVec->end_pos){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        pos = VBDecode(readin);
        readin.clear();

    	std::vector<unsigned int>::iterator itdoc = docID.begin();
    	std::vector<unsigned int>::iterator itfrag = fragID.begin();
    	std::vector<unsigned int>::iterator itpos = pos.begin();

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
            //std::cout << termID << ' ' <<*itdoc << ' '<< *itfrag << ' ' << *itpos << std::endl;
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
        std::cerr << "File cannot be opened." << std::endl;
    }
    return result;
}

std::vector<nPosting> Index::decompress_np(std::string namebase, unsigned int termID){
    //std::cout << "Decompressing np" << std::endl;
    std::ifstream ifile;
    std::string filename = std::string(NPDIR) + namebase;
    ifile.open(filename, std::ios::binary);
    std::vector<nPosting> result;

    if(ifile.is_open()){
        //std::cout << namebase << " Opened for Decompressing" << std::endl;
    	char c;
    	std::vector<char> readin;

    	std::vector<unsigned int> docID;
    	std::vector<unsigned int> freq;
    	//std::vector<unsigned int> sign;

    //TODO:
        std::vector<mDatanp>::iterator currMVec;

        for( currMVec = exlex.getNonPositionalBegin(termID); currMVec != exlex.getNonPositionalEnd(termID); currMVec++){
            //std::cout << currMVec->filename << std::endl;
            //std::cout << currMVec->posting_start << ' ' << currMVec->frag_start << ' ' << currMVec->pos_start << std::endl;
            if( currMVec->filename == namebase )
                break;
        }

    	ifile.seekg(currMVec->posting_start);
    	while(ifile.tellg() != currMVec->freq_start){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        docID = VBDecode(readin);
        readin.clear();

    	ifile.seekg(currMVec->freq_start);
    	while(ifile.tellg() != currMVec->end_pos){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        freq = VBDecode(readin);
        readin.clear();

    	std::vector<unsigned int>::iterator itdoc = docID.begin();
    	std::vector<unsigned int>::iterator itfreq = freq.begin();

        unsigned int prevdoc =0;
        //std::cout << "In " << filename << ", there are "<< std::endl;
    	while(itdoc != docID.end()){
            nPosting p(termID, (*itdoc + prevdoc), *itfreq);
            //std::cout << termID << ' ' << *itdoc + prevdoc << ' '<< *itfreq << ' ' << *itsign << std::endl;
            prevdoc = *itdoc + prevdoc;
            result.push_back(p);
            itdoc ++;
            itfreq ++;
    	}
    }else{
        std::cerr << "File cannot be opened." << std::endl;
    }
    return result;
}

std::vector<char> Index::read_com(std::ifstream& infile, long end_pos){//read compressed forward index
    char c;
    std::vector<char> result;
    while(infile.tellg() != end_pos){
        infile.get(c);
    result.push_back(c);
    }
    //the last one is not read in the loop
    infile.get(c);
    result.push_back(c);
    return result;
}

void Index::merge_test(){
    /**
     * Test if there are two files of same index number on disk.
     * If there is, merge them and then call merge_test again until
     * all index numbers has only one file each.
     */
    int indexnum = 0;
    std::string dir = std::string(PDIR);
    std::string npdir = std::string(NPDIR);
    std::vector<std::string> files = read_directory(dir);
    std::string fp = std::string("I") + std::to_string(indexnum);
    std::string fnp = std::string("L") + std::to_string(indexnum);

    while(check_contain(files, fp)){
    //if In exists already, merge In with Zn
        files.clear();
    merge_p(indexnum);
    indexnum ++;
        fp = std::string("I") + std::to_string(indexnum);
        files.clear();
        files = read_directory(dir);
    }
    indexnum = 0;
    std::vector<std::string> npfiles = read_directory(npdir);
    while(check_contain(npfiles, fnp)){
    //if Ln exists already, merge In with Xn
        npfiles.clear();
    merge_np(indexnum);
    indexnum ++;
        fnp = std::string("L") + std::to_string(indexnum);
        npfiles.clear();
        npfiles = read_directory(npdir);
    }
}

void Index::merge_p(int indexnum){
    /**
     * Merge two positional index.
     */
    std::ifstream filez;
    std::ifstream filei;
    std::ofstream ofile;
    std::string pdir(PDIR);

    //determine the name of the output file, if "Z" file exists, than compressed to "I" file.
    char flag = 'Z';
    filez.open(pdir + "Z" + std::to_string(indexnum));
    filei.open(pdir + "I" + std::to_string(indexnum));

    ofile.open(pdir + "Z" + std::to_string(indexnum + 1), std::ios::app | std::ios::binary);
    if(ofile.tellp() != 0){
    ofile.close();
    ofile.open(pdir + "I" + std::to_string(indexnum + 1), std::ios::ate | std::ios::binary);
        flag = 'I';
    }

    std::cout << "Merging into " << flag << indexnum + 1 << "------------------------------------" << std::endl;

    std::string file1 = "Z" + std::to_string(indexnum);
    std::string file2 = "I" + std::to_string(indexnum);
    //std::vector<f_meta>& v1 = filemeta[file1];
    //std::vector<f_meta>& v2 = filemeta[file2];
    //std::vector<f_meta>::iterator it1 = v1.begin();
    //std::vector<f_meta>::iterator it2 = v2.begin();

    /**
     * Go through the meta data of each file, do
     * if there is a termID appearing in both, decode the part and merge
     * else copy and paste the corresponding part of postinglist
     * update the corresponding fileinfo of that termID
     * assume that the posting of one term can be stored in memory
     */

     /*
    while( it1 != v1.end() && it2 != v2.end() ){
    if( it1->termID == it2->termID ){
    //decode and merge
    //update meta data corresponding to the term
    std::vector<Posting> vp1 = decompress_p(file1, it1->termID);
            std::vector<Posting> vp2 = decompress_p(file2, it2->termID);
    std::vector<Posting> vpout; //s§tore the sorted result

    //use NextGQ to write the sorted std::vector of Posting to disk
    std::vector<Posting>::iterator vpit1 = vp1.begin();
    std::vector<Posting>::iterator vpit2 = vp2.begin();
    while( vpit1 != vp1.end() && vpit2 != vp2.end() ){
    //NextGQ
    if( *vpit1 < *vpit2 ){
    vpout.push_back(*vpit1);
    vpit1 ++;
    }
    else if( *vpit1 > *vpit2 ){
    vpout.push_back(*vpit2);
    vpit2 ++;
    }
    else if ( *vpit1 == *vpit2 ){
    std::cout << "Error: same posting appearing in different indexes." << std::endl;
    break;
    }
    }
            while( vpit1 != vp1.end()){
                vpout.push_back(*vpit1);
                vpit1 ++;
            }
            while( vpit2 != vp2.end()){
                vpout.push_back(*vpit2);
                vpit2 ++;
            }

    compress_p(vpout, indexnum + 1, flag);
    it1 ++;
    it2 ++;
    }
    else if( it1->termID < it2->termID ){
    std::vector<Posting> vp = decompress_p(file1, it1->termID);
            compress_p(vp, indexnum + 1, flag);
    it1 ++;
    }
    else if( it1->termID > it2->termID ){
            std::vector<Posting> vp = decompress_p(file2, it2->termID);
            compress_p(vp, indexnum + 1, flag);
    it2 ++;
    }
    }

    */

    /**
     * TODO: decompress from the old index and then compress to
     * the new one to update metadata is time-consuming
     * need to find a more efficient way to update metadata while tranfering positngs
     */

    /*
    while (it1 != v1.end() ){
        std::vector<Posting> vp = decompress_p(file1, it1->termID);
        //update_t_meta(it1->termID, file1, dict);
        compress_p(vp, indexnum + 1, flag);
        it1 ++;
    }
    while (it2 != v2.end() ){
        std::vector<Posting> vp = decompress_p(file2, it2->termID);
        compress_p(vp, indexnum + 1, flag);
        it2 ++;
    }

    */

    filez.close();
    filei.close();
    ofile.close();
    std::string filename1 = pdir + "Z" + std::to_string(indexnum);
    std::string filename2 = pdir + "I" + std::to_string(indexnum);

    /*
    for( std::vector<f_meta>::iterator it = filemeta[file1].begin(); it != filemeta[file1].end(); it ++){
        update_t_meta(it->termID, file1);
    }

    for( std::vector<f_meta>::iterator it = filemeta[file2].begin(); it != filemeta[file2].end(); it ++){
        update_t_meta(it->termID, file2);
    }
    */
    //TODO: update ExtendedLexicon

    //deleting two files
    if( remove( filename1.c_str() ) != 0 )
        std::cout << "Error deleting file" << std::endl;

    if( remove( filename2.c_str() ) != 0 )
        std::cout << "Error deleting file" << std::endl;
}

void Index::merge_np(int indexnum){
    /**
     * Merge non-positional index.
     */
    std::ifstream filez;
    std::ifstream filei;
    std::ofstream ofile;
    std::string pdir(NPDIR);
    char flag = 'X';//determine the name of the output file
    filez.open(pdir + "X" + std::to_string(indexnum));
    filei.open(pdir + "L" + std::to_string(indexnum));

    ofile.open(pdir + "X" + std::to_string(indexnum + 1), std::ios::app | std::ios::binary);
    if(ofile.tellp() != 0){
        std::cout << "cannot merge to " << flag << indexnum + 1 << std::endl;
    ofile.close();
    ofile.open(pdir + "L" + std::to_string(indexnum + 1), std::ios::ate | std::ios::binary);
        flag = 'L';
    }

    std::cout << "Merging into " << flag << indexnum + 1 << "------------------------------------" << std::endl;

    std::string file1 = "X" + std::to_string(indexnum);
    std::string file2 = "L" + std::to_string(indexnum);
    //std::vector<f_meta>& v1 = filemeta[file1];
    //std::vector<f_meta>& v2 = filemeta[file2];
    //std::vector<f_meta>::iterator it1 = v1.begin();
    //std::vector<f_meta>::iterator it2 = v2.begin();

    /**
     * Go through the meta data of each file, do
     * if there is a termID appearing in both, decode the part and merge
     * else copy and paste the corresponding part of postinglist
     * update the corresponding fileinfo of that termID
     * assume that the posting of one term can be stored in memory
     */

     /*
    while( it1 != v1.end() && it2 != v2.end() ){
        //std::cout << it1->termID << ' ' << it2->termID << std::endl;
    if( it1->termID == it2->termID ){
    //decode and merge
    //update meta data corresponding to the term
    std::vector<nPosting> vp1 = decompress_np(file1, it1->termID);
            std::vector<nPosting> vp2 = decompress_np(file2, it2->termID);
    std::vector<nPosting> vpout; //store the sorted result

    //use NextGQ to write the sorted std::vector of Posting to disk
    std::vector<nPosting>::iterator vpit1 = vp1.begin();
    std::vector<nPosting>::iterator vpit2 = vp2.begin();
    while( vpit1 != vp1.end() && vpit2 != vp2.end() ){
    //NextGQ
    if( *vpit1 < *vpit2 ){
    vpout.push_back(*vpit1);
    vpit1 ++;
    }
    else if( *vpit1 > *vpit2 ){
    vpout.push_back(*vpit2);
    vpit2 ++;
    }
    else if ( *vpit1 == *vpit2 ){
                    vpout.push_back(*vpit1);
                    vpout.push_back(*vpit2);
    vpit1 ++;
                    vpit2 ++;
    break;
    }
    }
            while( vpit1 != vp1.end()){
                vpout.push_back(*vpit1);
                vpit1 ++;
            }
            while( vpit2 != vp2.end()){
                vpout.push_back(*vpit2);
                vpit2 ++;
            }
    compress_np(vpout, indexnum + 1, flag);
    it1 ++;
    it2 ++;
    }
    else if( it1->termID < it2->termID ){
    std::vector<nPosting> vp = decompress_np(file1, it1->termID);
            compress_np(vp, indexnum + 1, flag);
    it1 ++;
    }
    else if( it1->termID > it2->termID ){
            std::vector<nPosting> vp = decompress_np(file2, it2->termID);
            compress_np(vp, indexnum + 1, flag);
    it2 ++;
    }
    }
    */

    /**
     * TODO: decompress from the old index and then compress to the new one to update metadata is time-consuming
     * need to find a more efficient way to update metadata while tranfering positngs
     */
    /*
    while (it1 != v1.end() ){
        std::vector<nPosting> vp = decompress_np(file1, it1->termID);
        compress_np(vp, indexnum + 1, flag);
        it1 ++;
    }
    while (it2 != v2.end() ){
        std::vector<nPosting> vp = decompress_np(file2, it2->termID);
        compress_np(vp, indexnum + 1, flag);
        it2 ++;
    }
    */
    filez.close();
    filei.close();
    ofile.close();
    std::string filename1 = pdir + "X" + std::to_string(indexnum);
    std::string filename2 = pdir + "L" + std::to_string(indexnum);

    //TODO: update ExtendedLexicon
    /*

    for( std::vector<f_meta>::iterator it = filemeta[file1].begin(); it != filemeta[file1].end(); it ++){
        update_t_meta(it->termID, file1);
    }

    for( std::vector<f_meta>::iterator it = filemeta[file2].begin(); it != filemeta[file2].end(); it ++){
        update_t_meta(it->termID, file2);
    }
    */

    //deleting two files
    if( remove( filename1.c_str() ) != 0 )
        std::cout << "Error deleting file" << std::endl;

    if( remove( filename2.c_str() ) != 0 )
        std::cout << "Error deleting file" << std::endl;
}
