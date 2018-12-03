#ifndef BM25_HPP
#define BM25_HPP

#include <vector>

//freq and docscontaining indexes correspond to the same term
double BM25(std::vector<unsigned int>& freq, std::vector<unsigned int>& docscontaining, unsigned int doclength, double avgdoclength, unsigned int totaldocs);

#endif