#pragma once
#include "Awget.h"
#include <string>
#include <memory>

class AnonServerConn {
    
public:
	AnonServerConn();
	AnonServerConn(const AnonServerConn& other) {};
	~AnonServerConn();
	void GetFile(int fd);
    
private:
	int clientFD;
	SSStruct serverRecSSStruct();
	std::unique_ptr<Awget> client;
	void serverSendAwgetFile(std::string fileName);
	WGETStruct createPacket(int fd, unsigned short seqNum, off_t& offset, off_t fileSize);
};

