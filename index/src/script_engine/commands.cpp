#include "commands.hpp"

void commandInsert(std::unique_ptr<Index>& indexptr, std::unique_ptr<ReaderInterface>& docreader, std::vector<std::string>& arguments) {
    if(indexptr == nullptr)
        throw std::runtime_error("Error: index is not initialized");
    if(docreader == nullptr)
        throw std::runtime_error("Error: no docreader specified");
    if(arguments.size() != 2)
        throw std::invalid_argument("Error: invalid number of arguments to insert");

    int doccount = stoi(arguments[1]);

    auto begin = std::chrono::high_resolution_clock::now();

    int docsinserted = 0;
    for(int i = 0; i < doccount && docreader->isValid(); i++) {
        std::string url = docreader->getURL();
        std::string contents = docreader->getCurrentDocument();

        std::cout << "Inserting file #" << docsinserted << ": " << url << std::endl;

        indexptr->insert_document(url, contents);
        docsinserted++;

        docreader->nextDocument();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto dur = end - begin;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    std::cout << "Inserted " << docsinserted << " documents in " << ms << "ms for an average of " << ms / (double)docsinserted
        << " ms/doc\n";

    indexptr->printSize();
}