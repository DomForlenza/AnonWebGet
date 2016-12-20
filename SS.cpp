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
#include <signal.h>
#include <cstring>
#include <memory>
#include "SS.h"
#include "AnonServerConn.h"

#define PORT "65001"
#define BACKLOG 5

using namespace std;


SS::SS(string port):
portnumber(port) {}

// Destructor
SS::~SS(){
	close(socketFileDescriptor);
	for (size_t i = 0; i < serverConns.size(); i++) {
		//serverConns[i].~thread();
	}
}

// "Server" method
int SS::server() {
		int result;
		int one = 1;
		struct addrinfo *serverinfo;
		struct addrinfo hints;
		struct addrinfo *p;
		memset(&hints, 0, sizeof hints);	// clears memory
		hints.ai_family = AF_UNSPEC;		// type of IP v4 or v6
		hints.ai_socktype = SOCK_STREAM;	// stream socket not datagram
		hints.ai_flags = AI_PASSIVE; 		// use my current local host IP
		
		char hostname[1024];
		gethostname(hostname, 1024);
		hostent * ipRecord = gethostbyname(hostname);
		in_addr * ipAddress = (in_addr * )ipRecord->h_addr;
		finalIPAddress = inet_ntoa(* ipAddress);
		
		if ((result = getaddrinfo(finalIPAddress.c_str(), portnumber.c_str(), &hints, &serverinfo)) != 0) {
			cerr << "getaddrinfo: " << gai_strerror(result) << endl;
			return 1;
		}
		
		for (p = serverinfo; p != NULL; p = p->ai_next) {
			if ((socketFileDescriptor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
				cerr << "Server Error: Failed to make socket." << endl;
				continue;
			}

			if (setsockopt(socketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
				cerr << "Server Error: Failed to setSocketOptions." << endl;
				exit(1);
			}

			if (bind(socketFileDescriptor, p->ai_addr, p->ai_addrlen) == -1) {
				close(socketFileDescriptor);
				cerr << "Server Error: Failed to bind." << endl;
				continue;
			}
			break;
		}
		
		freeaddrinfo(serverinfo); // Frees the serverinfo memory
		
		if (p == NULL) {
			cerr << "Server Error: Failed to bind, reached null at end of struct." << endl;
			exit(1);
		}
		
		if (listen(socketFileDescriptor, BACKLOG) == -1) {
			cerr << "Server Error: Listen failed." << endl;
			exit(1);
		}

		waitingForConnection();
		return 0;
	}

// Wait for connections
void SS::waitingForConnection(){
	while(true) {  // Main accept() loop
		struct sockaddr_storage theirAddress;
		socklen_t socketSize = sizeof theirAddress;
		int new_fd = accept(socketFileDescriptor, (struct sockaddr *)&theirAddress, &socketSize);
		if (new_fd == -1) {
			cerr << "Server Error: Accept failed." << endl;
			continue;
		} else if (new_fd == 0) {
			cerr << "Server Error: client closed conn failed." << endl;
			continue;
		}
		
		inet_ntop(theirAddress.ss_family, get_in_addr((struct sockaddr *)&theirAddress), tempChar, sizeof tempChar);
		std::cout << "ss " << finalIPAddress << ", " << portnumber << endl;
		serverConns.push_back(std::thread(&AnonServerConn::GetFile, AnonServerConn(), new_fd)); // Start!
	}
}

// IPv4 or IPv6?
void * SS::get_in_addr(struct sockaddr *sa){
	
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

std::unique_ptr<SS> theServer = NULL;

// Sig handler
void sig_handler(int signal){
	if (theServer != NULL)
		theServer->~SS();
	exit(1);
}

// Usage message
string usage(){
    string tempstring = "HELP:\n SS: ./SS [-p port] (port argument is optional)\n";
    return tempstring;
}

// Main
int main(int argc, char* argv[]) {
    string port = "63756";
    
    // Argument checking
    if (argc > 3) {
        cerr << usage() << endl;
       exit(0);
    }
    else if (argc == 2) {
        cerr << usage() << endl;
        exit(0);
    }
    
    if (argc == 3) {
		if (strcmp( "-p", argv[1])!=0) {
			cerr <<"error" << endl;
			cerr << usage() << endl;
			exit(0);
		}
		port = argv[2];
	}

    std::cout << "Server started." << std::endl;
    theServer = std::unique_ptr<SS> (new SS(port));
    theServer->server();
    
    return 0;
}
