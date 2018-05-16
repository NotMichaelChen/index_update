#include "indexwriter.hpp"

#include <unistd.h>
#include <sys/stat.h>
#include <exception>

#include "util.hpp"

void writeIndex(std::string name, Index& index) {
    std::string foldername = Utility::getTimestamp() + name;

    struct stat st;
    if(!(stat(foldername.c_str(),&st) == 0 && st.st_mode & (S_IFDIR != 0))) {
        mkdir(foldername.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    chdir(foldername.c_str());

    index.dump();

    //TODO: Somehow write static indexes here?

    chdir("..");
}

void readIndex(std::string name, Index& index) {
    struct stat st;
    if(!(stat(name.c_str(),&st) == 0 && st.st_mode & (S_IFDIR != 0))) {
        throw std::invalid_argument("Error: Directory does not exist");
    }

    chdir(name.c_str());

    index.restore();

    //TODO: Set static index directory here?

    chdir("..");
}