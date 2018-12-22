template<typename T>
void writeAsBytes(T var, std::ofstream& ofile) {
    ofile.write(reinterpret_cast<const char *>(&var), sizeof(var));
}

template<typename T>
void readFromBytes(T& var, std::ifstream& ifile) {
    ifile.read(reinterpret_cast<char *>(&var), sizeof(var));
}