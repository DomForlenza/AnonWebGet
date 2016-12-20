#include "Awget.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <fstream>
#include <signal.h>
#include <memory>

using std::cerr;
using std::endl;
using std::string;

std::unique_ptr<Awget> theClient = NULL;

// Sig handler to close all connections upon ^C termination
void sig_handler(int signal){
	if (theClient != NULL)
		theClient->~Awget();
	exit(1);	
}

// Usage message
string usage() {
    string tempstring = "HELP:\n Awget: ./awget <URL> [-c chainfile] (chainfile is optional)\n";
    return tempstring;
}

// Main
int main(int argc, char* argv[]) {
    
	signal(SIGINT/SIGTERM/SIGKILL, sig_handler);
    
	string chainfile;
	string URL;
	chainfile = "chaingang.txt";

	
    // Arg checking
	if (argc > 4) {
		cerr << usage() << endl;
		exit(0);
	}
	else if (argc < 2) {
		cerr << usage() << endl;
		exit(0);
	}
	else if (argc == 3) {
		cerr << usage() << endl;
		exit(0);
	}
	if (argc == 4) {
		if (strcmp(argv[2], "-c")!=0) {
			cerr << usage() << endl;
			exit(0);
		}
		chainfile = argv[3];
	}

	URL = argv[1];
	
	std::ifstream inputFile(chainfile);
	
	if (inputFile.fail()) {
		cerr << "Error: Problem opening the input file." << endl;
		exit(0);
	}
	
	// File variables
	int numSS;
	string IP;
	string port;
	
	string line;
	getline(inputFile, line);
	numSS = stoi(line);
	vector<vector<string>> ssData;
	
	for (int i = 0; i < numSS; i++) {
 		getline(inputFile, line);
		vector<string> lineData;
		std::stringstream ss(line);
		
		ss >> IP;
		ss >> port;
		
		lineData.push_back(IP); 
		lineData.push_back(port);
		ssData.push_back(lineData);
	} 
	
	srand (time(NULL));
	
    // Chose random SS from file
	int chosenSS = rand() % numSS;
	IP = ssData[chosenSS][0];
	port = ssData[chosenSS][1];
	ssData.erase(ssData.begin() + chosenSS);
	string stringSSData;
	std::cout << IP << std::endl;
	for (size_t i = 0; i < ssData.size(); i++){
		stringSSData+= ssData[i][0];
		stringSSData+=",";
		stringSSData+= ssData[i][1];
		stringSSData+=";";
	}
	theClient = std::unique_ptr<Awget> (new Awget(IP,port,URL,stringSSData));
	theClient->client();
	std::cout << "File received" << std::endl;
	return 0;
}