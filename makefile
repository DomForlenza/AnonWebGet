#name of compiler
CC=g++

#Object files to become linked and create an executable
AWGETOBJS=Awget.o awgetexe.o
SSOBJECTS=AnonServerConn.o SS.o Awget.o
#Include gdb information in your executable
DEBUG=-g

#Flags of g++ / Warning flags are only Wall. (CFLAGS=Compile flags, Lflags=Link flags), turns all of them on
CFLAGS=-Wall -I. -c -std=c++11 $(DEBUG) -O2
LFLAGS=-Wall $(DEBUG) -pthread

#What the executable file will be named
#puts all into an executable file
all: Awget SS
	$(RM) *.o ~*

Awget: $(AWGETOBJS)
	$(CC) $(LFLAGS) $(AWGETOBJS) -o awget
	
SS: $(SSOBJECTS)
	$(CC) $(LFLAGS) $(SSOBJECTS) -o ss
	
	
#This is the .exe name...........^ Do not make this the name of any of the files that are being loaded in!!
#Files to make into object files	
Awget.o: Awget.cpp
	$(CC) $(CFLAGS) $< 
#This is the .exe name...........^ Do not make this the name of any of the files that are being loaded in!!
#Files to make into object files	
AnonServerConn.o: AnonServerConn.cpp
	$(CC) $(CFLAGS) $< 
#This is the .exe name...........^ Do not make this the name of any of the files that are being loaded in!!
#Files to make into object files	
awgetexe.o: awgetexe.cpp
	$(CC) $(CFLAGS) $< 
#This is the .exe name...........^ Do not make this the name of any of the files that are being loaded in!!
#Files to make into object files	
SS.o: SS.cpp
	$(CC) $(CFLAGS) $< 

#Clean function for executable file
clean:
	rm -f $(AWGETOBJS) $(SSOBJECTS) *~ awget ss
#Never put *.pgm as one of your remove flags (this will remove every .pgm file in the directory)
