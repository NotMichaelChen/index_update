#include "parse_engine.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

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
    for(size_t i = start; i < code.size(); ++i) {
        std::string lower = code[i];
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if(lower == "endloop")
            return i;
    }
    return code.size();
}

//begin inclusive, end exclusive
void parseCode(std::vector<std::string>& code, size_t begin, size_t end) {
    size_t index = 0;
    while(index < code.size()) {
        std::vector<std::string> arguments = splitLine(code[index]);

        std::string command = arguments[0];
        std::transform(command.begin(), command.end(), command.begin(), ::tolower);

        if(command == "reset") {

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
            size_t times = stoi(arguments[1]);
            size_t end = findEndLoop(code, index+1);

            for(size_t i = 0; i < times; ++i) {
                parseCode(code, index+1, end);
            }
            index = end+1;
        }
        else if(command == "endloop") {
            throw std::invalid_argument("Error: endloop found without matching loop");
        }
    }
}

void parseFile(std::string filename, std::vector<std::string>& filenames) {
    std::ifstream ifile(filename);
    if(!ifile) {
        std::cout << "Error opening file " << filename << std::endl;
    }

    std::vector<std::string> script;
    std::string command;
    //Store whole file in script
    while(std::getline(ifile, command)) {
        script.push_back(command);
    }

    parseCode(script, 0, script.size());
}
