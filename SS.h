#pragma once
#include <string>
#include <tuple>
#include <thread>
#include <vector>
#include "Awget.h"
using std::string;
using std::tuple;
using std::vector;
	
class SS {
	
public:
	SS(string port);
	~SS();
	int server();
    
private:
	std::vector<std::thread> serverConns;
	string portnumber;
	string finalIPAddress;
	int socketFileDescriptor;
	char tempChar[INET6_ADDRSTRLEN];
	void * get_in_addr(struct sockaddr *sa);
	void waitingForConnection();
};
