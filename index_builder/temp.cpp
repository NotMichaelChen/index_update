#include <fstream>
#include <iostream>
#include <bitset>
using namespace std;

int main(){
	ofstream file;
	uint8_t a = 255;
	file.open("test", ios::binary);
	file.write(reinterpret_cast<const char*>(&a), 1);
	file.close();

	ifstream ifile;
	char c;
	ifile.open("test", ios::binary);
	ifile.get(c);
	cout << c << endl;

	
	ifile.close();
}