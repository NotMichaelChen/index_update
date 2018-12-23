#include "partitioner.hpp"
#include "utility/string_util.hpp"

#include <iostream>
#include <vector>
#include <functional>

Partitioner::Partitioner(const unsigned int b, const unsigned int w) : B_WINNOW(b), W_WINNOW(w) {}

std::vector<Fragment> Partitioner::partitionPage(size_t did, const std::string& origPage) {
    std::vector<Fragment> partitionres;

    std::vector<std::string> normalized = normalizeFile(origPage);
    std::cout << "Normalizing result size: " << normalized.size() << '\n';

    std::vector<unsigned char> wordHashed = wordHashFile(normalized);
    std::cout << "Word hashing result size: " << wordHashed.size() << '\n';

    std::vector<size_t> hashed = hashFile(wordHashed);
    std::cout << "Substring hashing result size: " << hashed.size() << '\n';

    std::vector<size_t> cut = cutFile(hashed);
    std::cout << "Cut result size: " << cut.size() << '\n';

    size_t lastCut = 0;
    size_t totalcutsize = 0;
    size_t cutNo = 0;
    for (size_t i = 0; i < cut.size(); ++i) {
        if (cut[i]) {
            std::cout << "Cut #" << (++cutNo) << " - size: " << (i - lastCut + 1) << '\n';
            totalcutsize += (i - lastCut + 1);
            Fragment newfrag;
            newfrag.docid = did;
            newfrag.startpos = lastCut;
            newfrag.fragcontent = StringUtil::vecToString(std::vector<std::string>(
                                    normalized.begin() + lastCut,
                                    normalized.begin() + i));
            partitionres.push_back(newfrag);
            lastCut = i;
        }
    }

    return partitionres;
}

std::vector<std::string> Partitioner::normalizeFile(const std::string& origFile) {
    std::cout << "Normalizing page...\n";
    std::vector<std::string> processResult;
    std::vector<std::string> words = StringUtil::splitString(origFile, " \n\t\r\f!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~");
    for(const std::string& w : words) {
        if (!StringUtil::isAlnum(w)) { continue; }
        processResult.push_back(w);
    }
    return processResult;
}

std::vector<unsigned char> Partitioner::wordHashFile(const std::vector<std::string>& normalized) {
    std::cout << "Hashing each word in page...\n";
    std::vector<unsigned char> wordHashRes;
    std::hash<std::string> strHash;
    for(const std::string& w : normalized) {
        wordHashRes.push_back((unsigned char) strHash(w));
    }
    return wordHashRes;
}

std::vector<size_t> Partitioner::hashFile(const std::vector<unsigned char>& file) {
    std::cout << "Hashing substrings in page...\n";
    std::hash<std::string> strHash;

    if (file.size() <= B_WINNOW) {
        std::string s_i = std::string(file.begin(), file.end());
        std::vector<size_t> hashResult;
        hashResult.push_back(strHash(s_i));
        return hashResult;
    }

    std::vector<size_t> hashResult(file.size() - B_WINNOW);
    for (size_t i = 0; i < hashResult.size(); ++i) {
        std::string s_i = std::string(file.begin()+i, file.begin()+i+B_WINNOW-1);
        hashResult[i] = strHash(s_i);
    }
    return hashResult;
}

std::vector<size_t> Partitioner::cutFile(const std::vector<size_t>& hashedFile) {
    std::cout << "Cutting page...\n";
    std::vector<size_t> cutResults(hashedFile.size(), 0);

    if (cutResults.size() <= W_WINNOW) {
        cutResults.back() = 1;
        return cutResults;
    }

    for (size_t w = 0; w < cutResults.size() - W_WINNOW; ++w) {
        std::vector<size_t> possibleCuts;
        for (size_t i = w; i < w + W_WINNOW; ++i) {
            bool cut = true;
            for (size_t j = w; j < w + W_WINNOW; ++j) {
                if (i == j) { continue; }
                if (hashedFile[i] > hashedFile[j]) {
                    cut = false;
                    break;
                }
            }
            if (cut) { possibleCuts.push_back(i); }
        }
        if (possibleCuts.size() == 1) { cutResults[possibleCuts[0]] = 1; }
        else {
            bool alreadyCut = false;
            for (size_t c : possibleCuts) {
                if (cutResults[c] == 1) {
                    alreadyCut = true;
                }
            }
            if (!alreadyCut) { cutResults[possibleCuts.back()] = 1; }
        }
    }

    return cutResults;
}