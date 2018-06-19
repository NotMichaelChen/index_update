#include "parse_engine.hpp"

#include "index.hpp"
#include "document_readers/WETreader.hpp"
#include "document_readers/RAWreader.hpp"

std::vector<std::string> splitLine(std::string& line) {
    std::stringstream linestream(line);
    std::vector<std::string> arguments;

    std::string arg;
    while(getline(linestream, arg, ' ')) {
        arguments.push_back(arg);
    }

    return arguments;
}

//start is inclusive
size_t findEndLoop(std::vector<std::string>& code, size_t start) {
    //Loop through the vector looking for an endloop command
    for(size_t i = start; i < code.size(); ++i) {
        std::string lower = code[i];
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if(lower == "endloop")
            return i;
    }

    //Otherwise return the end of the script
    return code.size();
}

//begin inclusive, end exclusive
void parseCode(std::vector<std::string>& code, size_t begin, size_t end, std::string& dir, std::unique_ptr<Index>& indexptr,
    std::unique_ptr<ReaderInterface>& docreader)
{
    size_t linenum = begin;
    while(linenum < code.size() && linenum < end) {
        //Split the command into space-separated tokens
        std::vector<std::string> arguments = splitLine(code[linenum]);

        //Lowercase the command
        std::string command = arguments[0];
        std::transform(command.begin(), command.end(), command.begin(), ::tolower);

        if(command == "reset") {
            indexptr.reset();
            linenum++;
        }
        else if(command == "insert") {
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

            linenum++;
        }
        else if(command == "query") {
            std::cout << "Query not implemented!" << std::endl;
            linenum++;
        }
        else if(command == "setdir") {
            if(arguments.size() != 2)
                throw std::invalid_argument("Error: invalid number of arguments to setdir");

            dir = arguments[1];
            indexptr = std::make_unique<Index>(dir);

            linenum++;
        }
        else if(command == "loadcurrentdir") {
            if(indexptr == nullptr)
                throw std::runtime_error("Error: index is not initialized");
            if(dir.empty())
                throw std::invalid_argument("Error: working directory not set");

            indexptr->restore();

            linenum++;
        }
        else if(command == "dump") {
            if(indexptr == nullptr)
                throw std::runtime_error("Error: index is not initialized");
            if(dir.empty())
                throw std::invalid_argument("Error: working directory not set");

            indexptr->dump();

            linenum++;
        }
        else if(command == "docinput") {
            if(arguments.size() != 3)
                throw std::invalid_argument("Error: invalid number of arguments to docinput");
            
            std::string path = arguments[1];
            std::string reader = arguments[2];
            std::transform(reader.begin(), reader.end(), reader.begin(), ::tolower);

            if(reader == "wet") {
                docreader = std::make_unique<WETReader>(path);
            }
            else if(reader == "raw") {
                docreader = std::make_unique<RAWReader>(path);
            }
            else {
                throw std::runtime_error("Error: invalid document reader specified");
            }

            linenum++;
        }
        else if(command == "loop") {
            //Get how many times to loop, then find end of loop
            size_t times = stoi(arguments[1]);
            size_t end = findEndLoop(code, linenum+1);

            //Run the segment of code multiple times
            for(size_t i = 0; i < times; ++i) {
                parseCode(code, linenum+1, end, dir, indexptr, docreader);
            }

            //Go past the end of the loop
            linenum = end+1;
        }
        else if(command == "endloop") {
            throw std::invalid_argument("Error: endloop found without matching loop");
        }
        else {
            throw std::runtime_error("Error: invalid command");
        }
    }
}

void parseFile(std::string filename) {
    //Read in the file
    std::ifstream ifile(filename);
    if(!ifile) {
        std::cout << "Error opening file " << filename << std::endl;
    }

    //Store whole file in script
    std::vector<std::string> script;
    std::string command;
    while(std::getline(ifile, command)) {
        script.push_back(command);
    }

    //Create index object and reader interface
    std::unique_ptr<Index> indexptr = nullptr;
    // Index index;
    std::unique_ptr<ReaderInterface> docreader = nullptr;

    //Parse the script
    std::string dir;
    parseCode(script, 0, script.size(), dir, indexptr, docreader);
}
