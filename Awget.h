#pragma once
#include <string>
#include <tuple>
#include <vector>
using std::string;
using std::tuple;
using std::vector;

// SS struct
struct SSStruct{
	unsigned short sizeOfUrl;
	unsigned short numberOfSS;
	char url[1000];
	char ss[1000];
};
	
// WGET struct
struct WGETStruct{
	unsigned short packetSize;
	unsigned short sequenceNum;
	unsigned short readMoreFlag;
	char chunk[1400];
};

class Awget {
    
public:
	Awget(string ip, string port, string url, string ssData);
	~Awget();
	string client();
    
private:
	string fileName;
	string ip;
	string portnumber;
	int socketFileDescriptor;
	SSStruct ssURLPacket;
		
	void createPacket(string url, string ssData);
	void *get_in_addr(struct sockaddr *sa);
	void clientSendSSStruct();
	void clientReceiveWGETPacket();
};