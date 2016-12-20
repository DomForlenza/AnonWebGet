#include <iostream>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <time.h>
#include <memory>
#include <fstream>
#include "Awget.h"

#define PORT "65001"
#define BACKLOG 2

using namespace std;

struct sigaction sa;

Awget::Awget(string ip, string pn, string url, string ssData) :
ip(ip),
portnumber(pn) {
	fileName = url.substr(url.find("www"));
	if (fileName.find("/") == string::npos) {
		fileName = "index.html";
	} else {
		fileName = fileName.substr(fileName.find_last_of("/") +1);
	}
	createPacket(url, ssData);
}

// Destructor
Awget::~Awget() {
    if (socketFileDescriptor > 0) {
        close(socketFileDescriptor);
    }
}

// Client method
string Awget::client() {
	int result;
	struct addrinfo hints;
	struct addrinfo *serverinfo;
	struct addrinfo *p;
	char tempChar[INET6_ADDRSTRLEN];
	memset(&hints, 0, sizeof hints);	// clears memory
	hints.ai_family = AF_UNSPEC;		// type of IP v4 or v6
	hints.ai_socktype = SOCK_STREAM;	// stream socket not datagram
	//hints.ai_flags = AI_PASSIVE; 		// use my current local host IP
	
	if ((result = getaddrinfo(ip.c_str(), portnumber.c_str(), &hints, &serverinfo)) != 0) {
		cerr << "getaddrinfo: " << gai_strerror(result) << endl;
		return "Error";
	}
	
	for (p = serverinfo; p != NULL; p = p->ai_next) {
		if ((socketFileDescriptor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			cerr << "Client Error: Failed to make socket." << endl;
			continue;
		}

		if (connect(socketFileDescriptor, p->ai_addr, p->ai_addrlen) == -1) {
			close(socketFileDescriptor);
			cerr << "Client Error: Failed to bind socket." << endl;
			continue;
		}
		break;
	}
	
	if (p == NULL)  {
		cerr << "Client Error: Failed to bind, reached null at end of struct." << endl;
		exit(1);
	}
	
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), tempChar, sizeof tempChar);
	
	freeaddrinfo(serverinfo); // Free the serverinfo memory
	char hostname[128];
	gethostname(hostname, sizeof hostname);
	hostent * ipRecord = gethostbyname(hostname);
	in_addr * ipAddress = (in_addr * )ipRecord->h_addr;
	string finalIPAddress = inet_ntoa(* ipAddress);
	
	std::stringstream ss(string(ssURLPacket.ss, ssURLPacket.numberOfSS));
	string token;
	std::cout << "Request: " <<  string(ssURLPacket.url, ssURLPacket.sizeOfUrl) << std::endl;
	std::cout << "Chainlist is " << std::endl;
	while (std::getline(ss, token, ';'))
	{
		std::cout << token << std::endl;
	}
	cout << "next SS is: " << ip << ", " << portnumber << endl;
	clientSendSSStruct();

	cout << "waiting for file... \"" + fileName + "\"" << endl;
	clientReceiveWGETPacket();

    return fileName;
}


// Get sockaddr, IPv4 or IPv6:
void * Awget::get_in_addr(struct sockaddr *sa){

if (sa->sa_family == AF_INET) {
	return &(((struct sockaddr_in*)sa)->sin_addr);
}
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Client receive AWGET packet
void Awget::clientReceiveWGETPacket(){
	ofstream outputFile(fileName);
    
	if (outputFile) {
		string fileData;
		WGETStruct fileChunk;
        
		if (recv(socketFileDescriptor, (WGETStruct *) &fileChunk, sizeof(WGETStruct), MSG_WAITALL) > 0) {
			do {
				outputFile << string(fileChunk.chunk, fileChunk.packetSize);
				if (fileChunk.readMoreFlag == 0)
					break;
			} while (recv(socketFileDescriptor, (WGETStruct *) &fileChunk, sizeof(WGETStruct), MSG_WAITALL) > 0);
		}
	}
}

// Client send SS packet
void Awget::clientSendSSStruct() {
	int result = send(socketFileDescriptor, (char *)& ssURLPacket, sizeof(SSStruct) , 0);
	if (result < 0) {
		cout << "Error sending message.\n" << endl;
	}
	else if (result == 0) {
		cout << "Server closed connection.\n" << endl;
	}
}

// Create packet
void Awget::createPacket(string url, string ssData){
	// Send SSnum + URL to next SS
	// Pack
	ssURLPacket.sizeOfUrl = (unsigned short) url.size();
	ssURLPacket.numberOfSS = (unsigned short) ssData.size();
	snprintf(ssURLPacket.url,1000, "%s",url.c_str());
	snprintf(ssURLPacket.ss,1000, "%s",ssData.c_str());
}