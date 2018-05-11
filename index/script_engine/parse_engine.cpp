#include "parse_engine.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "index.hpp"

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
void parseCode(std::vector<std::string>& code, size_t begin, size_t end, Index& index) {
    size_t linenum = begin;
    while(linenum < code.size()) {
        //Split the command into space-separated tokens
        std::vector<std::string> arguments = splitLine(code[linenum]);

        //Lowercase the command
        std::string command = arguments[0];
        std::transform(command.begin(), command.end(), command.begin(), ::tolower);

        if(command == "reset") {
            index.clear();
        }
        else if(command == "insert") {

        }
        else if(command == "query") {

        }
        else if(command == "load") {

        }
        else if(command == "dump") {

        }
        else if(command == "loop") {
            //Get how many times to loop, then find end of loop
            size_t times = stoi(arguments[1]);
            size_t end = findEndLoop(code, linenum+1);

            //Run the segment of code multiple times
            for(size_t i = 0; i < times; ++i) {
                parseCode(code, linenum+1, end, index);
            }

            //Go past the end of the loop
            linenum = end+1;
        }
        else if(command == "endloop") {
            throw std::invalid_argument("Error: endloop found without matching loop");
        }
    }
}

void parseFile(std::string filename, std::vector<std::string>& filenames) {
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

    //Create index object
    Index index;

    //Parse the script
    parseCode(script, 0, script.size(), index);
}
