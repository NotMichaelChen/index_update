#include "commands.hpp"

#include "utility/morph.hpp"
#include "utility/timer.hpp"

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
    Utility::Timer stopwatch;

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
            DocumentMorpher morpher(currentdoc, nextdoc, versioncount);

            while(true) {
                std::string newdoc = morpher.getDocument();
                std::cout << "Inserting file #" << docsinserted << ": " << currenturl << std::endl;
                stopwatch.start();
                indexptr->insert_document(currenturl, newdoc);
                stopwatch.stop();
                docsinserted++;
                
                if(!morpher.isValid())
                    break;

                morpher.nextVersion();
            }
        }
        //Otherwise just insert the document
        else {
            std::cout << "Inserting file #" << docsinserted << ": " << currenturl << std::endl;
            stopwatch.start();
            indexptr->insert_document(currenturl, currentdoc);
            stopwatch.stop();
            docsinserted++;
        }
    }

    std::cout << "Inserted " << docsinserted << " documents in " << stopwatch.getCumulative() << "ms for an average of "
        << stopwatch.getCumulative() / (double)docsinserted << " ms/doc\n";

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