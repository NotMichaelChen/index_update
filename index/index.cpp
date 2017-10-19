#include "index.hpp"

#include <string>

#include "doc_analyzer/analyzer.h"
#include "Structures/documentstore.h"
#include "Structures/translationtable.h"

Index::Index() {
    docstore = Structures::DocumentStore();
    transtable = Structures::TranslationTable();
}

void Index::insert_document(std::string& url, std::string& newpage) {
    
    MatcherInfo results = indexUpdate(url, newpage, "put timestamp here", docstore, transtable);
}
