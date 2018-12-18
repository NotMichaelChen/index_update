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
    DocumentParser(std::string& url, std::string& newpage, std::string& timestamp, DocumentStore& docstore, LandmarkDirectory& landdir);

    bool isFirstDoc();
    bool termInOld(std::string& term);
    bool termInNew(std::string& term);
    std::vector<ExternPposting> getPPostings();
    std::vector<ExternNPposting> getNPPostings();
    std::vector<Landmark> getLandmarks();
    unsigned int getDocID();

private:
    void applyDiff(const std::vector<int>& olddoc, const std::vector<int>& newdoc,
    LandmarkArray& landarray, std::vector<DiffRange>& editscript);
    std::vector<ExternPposting> pospostings;

    std::unique_ptr<StringEncoder> se;
    unsigned int docID;
};