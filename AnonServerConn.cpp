#include "AnonServerConn.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <cstring>
#include <sstream>
#include <string>
 
 
using std::cout;
using std::endl;
using std::cerr;
using std::string;

AnonServerConn::AnonServerConn() {
	client = NULL;
}

AnonServerConn::~AnonServerConn() {
	if (clientFD > 0)
		close(clientFD);
}

// Get file
void AnonServerConn::GetFile(int fd){
	// Step 1: rec SS list/parse
	// Step 2:
	//	 A) if no SS -> awget
	//	 B) if SS -> target next SS
	//          i) create client with info -> client do
	// Step 3: send file to previous
	// Step 4: delete file, exit
	clientFD = fd;
	auto theSSChain = serverRecSSStruct();
	
	string fileName;
	if (theSSChain.numberOfSS > 0) {
		srand (time(NULL));
		std::stringstream ss(string(theSSChain.ss, theSSChain.numberOfSS));
		std::vector<string> ssList;
		string token;
        
		while (std::getline(ss, token, ';')) {
			ssList.push_back(token);
		}
        
		int chosenSS = rand() % ssList.size();
		string IP = ssList[chosenSS].substr(0, ssList[chosenSS].find(","));
		string port = ssList[chosenSS].substr(ssList[chosenSS].find(",")+1);
		ssList.erase(ssList.begin() + chosenSS);
		string stringSSData;
        
		for (size_t i = 0; i < ssList.size(); i++) {
			stringSSData+= ssList[i] + ";";
		}
		client = std::unique_ptr<Awget>(new Awget(IP, port, theSSChain.url, stringSSData));
		fileName = client->client();
	}
    // "WGET CALL"
	else {

		string url = std::string(theSSChain.url, theSSChain.sizeOfUrl);
		fileName = url.substr(url.find("www"));
		if (fileName.find("/") == string::npos) {
			fileName = "index.html";
		} else {
			fileName = fileName.substr(fileName.find_last_of("/") +1);
		}
		string command = "wget -q " + url;
		cout << "issuing wget for file " << fileName << endl;
		system (command.c_str());

	}
	cout << "Relaying file..." << endl;
	serverSendAwgetFile(fileName);
	remove(fileName.c_str());
	cout << "File relayed!" << endl;
	close(clientFD);
}


// Server Recv packet
SSStruct AnonServerConn::serverRecSSStruct(){
	SSStruct ssMessage;
	int result;
	if ((result = recv(clientFD, (SSStruct*) &ssMessage, sizeof(SSStruct), MSG_WAITALL)) == -1) {
		cout << result << endl;
		cerr << "ServerConn Error: Server receive failed." << endl;
	}
	return ssMessage;
}

// Send AWGET File
void AnonServerConn::serverSendAwgetFile(string fileName) {
	off_t offset = 0;
	struct stat file_stat;
	unsigned short seqNum = 0;
	memset(&file_stat, 0, sizeof file_stat); 
	int fileDesc;
	
	fileDesc = open(fileName.c_str(), O_RDONLY);
	if (fileDesc < 0) {
		cout << "ServerConn Error: Failed to read file." << endl;
		return;
	}
	
	int result = fstat(fileDesc, &file_stat);
	if (result < 0) {
		cout << "Error: fstat failed." << endl;
	}
	
	ssize_t fileSize = file_stat.st_size;

	while (offset < fileSize) {
		WGETStruct toSend = createPacket(fileDesc, seqNum, offset, fileSize);
		ssize_t datasent = send(clientFD, (char *) &toSend, sizeof (WGETStruct), 0);
		if (datasent == 0) {
			cout << "Server Thread: client closed connection." << endl;
			return;
		} else if (datasent == -1) {
			cout << "Server Thread: error sending data." << endl;
		} else {
			seqNum++;
		}
	}	
}


// Create packet
WGETStruct AnonServerConn::createPacket(int fd, unsigned short seqNum, off_t& offset, off_t fileSize) {
	WGETStruct fileChunk;
	ssize_t read = pread(fd, fileChunk.chunk, 1400, offset);
	fileChunk.packetSize = (unsigned short) read;
	fileChunk.sequenceNum = seqNum;
	fileChunk.readMoreFlag = ((read + offset) >= fileSize ? 0 : 1);
	offset += read;
	return fileChunk;
}
