#pragma once

#include <string>
#include <memory>

#include "externalposting.hpp"
#include "stringencoder.hpp"
#include "redis_interfaces/documentstore.hpp"
#include "landmark/landmark.hpp"
#include "landmark/landmarkdirectory.hpp"
#include "diff.hpp"

class DocumentParser {
public:
    // Note that LandmarkDirectory will be modified
    DocumentParser(std::string& url, std::string& newpage, std::string& timestamp, DocumentStore& docstore, LandmarkDirectory& landdir);

    bool isFirstDoc();
    bool termInOld(std::string& term);
    bool termInNew(std::string& term);
    std::vector<ExternPposting> getPPostings();
    std::vector<ExternNPposting> getNPPostings();
    unsigned int getDocID();

private:
    // Run when initializing a document with new landmarks
    void initializeDocument(const std::vector<int>& newdoc, LandmarkArray& landarray);

    void applyDiff(const std::vector<int>& olddoc, const std::vector<int>& newdoc,
        LandmarkArray& landarray, std::vector<DiffRange>& editscript);

    std::vector<ExternPposting> p_postings;
    std::vector<ExternNPposting> np_postings;

    std::unique_ptr<StringEncoder> se;
    unsigned int docID;
};