#include <iostream>
#include <fstream>
#include <random>

void generateVersion(std::ifstream& afile, std::ifstream& bfile, std::string& ofilename, double aswitch, double bswitch)
{
    std::ofstream ofile(ofilename, std::ios::out);

    //http://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution 
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<double> dis(0, 1.0);

    std::string a, b;
    bool in_state_a = true;
    //TODO: better handle files of different lengths
    while(afile >> a && bfile >> b)
    {
        double outcome = dis(gen);
        //State change
        if(in_state_a)
        {
            if(outcome <= aswitch)
                in_state_a = false;
        }
        else
        {
            if(outcome <= bswitch)
                in_state_a = true;
        }

        ofile << (in_state_a ? a : b) << " ";
    }
}

int main(int argc, char** argv)
{
    if(argc != 4)
    {
        std::cout << "Error, invalid number of arguments" << std::endl;
        std::cout << "Arguments are \"file1 file2 steps\"" << std::endl;
        return 1;
    }

    std::ifstream afile(argv[1]);
    if(!afile)
    {
        std::cout << "Error opening file 1" << std::endl;
        return 2;
    }

    std::ifstream bfile(argv[2]);
    if(!bfile)
    {
        std::cout << "Error opening file 2" << std::endl;
        return 3;
    }
    
    int steps = std::stoi(argv[3]);
    double stepsize = 1.0 / (steps+1);
    double prob = stepsize;

    for(int i = 0; i < steps; ++i)
    {
        std::string base = "v" + std::to_string(i);
        generateVersion(afile, bfile, base, prob, 1-prob);

        //Rewind files
        afile.clear();
        bfile.clear();
        afile.seekg(0);
        bfile.seekg(0);

        prob += stepsize;
    }

    return 0;
}