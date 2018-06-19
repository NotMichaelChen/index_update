#include "commands.hpp"

#include "morph.hpp"

void commandInsert(std::unique_ptr<Index>& indexptr, std::unique_ptr<ReaderInterface>& docreader, std::vector<std::string>& arguments) {
    //Check that arguments are valid
    if(indexptr == nullptr)
        throw std::runtime_error("Error: index is not initialized");
    if(docreader == nullptr)
        throw std::runtime_error("Error: no docreader specified");
    if(arguments.size() < 2 || arguments.size() > 3)
        throw std::invalid_argument("Error: invalid number of arguments to insert");

    //Parse args
    int doccount = stoi(arguments[1]);
    int versioncount = -1;
    if(arguments.size() >= 3)
        versioncount = stoi(arguments[2]);

    //Begin timed section
    auto begin = std::chrono::high_resolution_clock::now();

    int docsinserted = 0;
    std::string nexturl = docreader->getURL();
    std::string nextdoc = docreader->getCurrentDocument();

    for(int i = 0; i < doccount && docreader->isValid(); i++) {
        std::string currenturl = nexturl;
        std::string currentdoc = nextdoc;

        docreader->nextDocument();
        nexturl = docreader->getURL();
        nextdoc = docreader->getCurrentDocument();

        //If there are versions, generate them based on the next document
        if(versioncount > 0) {
            //Only resize currentdoc if it's longer than the nextdoc; this is to avoid big deletions when versioning
            if(currentdoc.length() > nextdoc.length())
                currentdoc.resize(nextdoc.length());

            DocumentMorpher morpher(currentdoc, nextdoc, versioncount);

            while(morpher.isValid()) {
                std::string newdoc = morpher.getVersion();
                std::cout << "Inserting file #" << docsinserted << ": " << currenturl << std::endl;
                indexptr->insert_document(currenturl, newdoc);
                docsinserted++;
            }
        }
        //Otherwise just insert the document
        else {
            std::cout << "Inserting file #" << docsinserted << ": " << currenturl << std::endl;
            indexptr->insert_document(currenturl, currentdoc);
            docsinserted++;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto dur = end - begin;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    std::cout << "Inserted " << docsinserted << " documents in " << ms << "ms for an average of " << ms / (double)docsinserted
        << " ms/doc\n";

    // int docsinserted = 0;
    // for(int i = 0; i < doccount && docreader->isValid(); i++) {
    //     std::string url = docreader->getURL();
    //     std::string contents = docreader->getCurrentDocument();

    //     std::cout << "Inserting file #" << docsinserted << ": " << url << std::endl;

    //     indexptr->insert_document(url, contents);
    //     docsinserted++;

    //     docreader->nextDocument();
    // }


    indexptr->printSize();
}