#ifndef __GalactiCombatServer_h_
#define __GalactiCombatServer_h_

#include <iostream>
#include <string>
#include <vector>
#include "GalactiCombat.h"
#include "NetworkUtil.h"
#include "ClientSpaceShipController.h"

#define PAUSE 0
#define PLAY 1

struct Client{
    TCPsocket sock;
    int channel;
    std::string name;
    SpaceShip* ship;
    bool ready;
};
typedef struct Client Client; // allows Client struct to be used in C

class GalactiCombatServer : public GalactiCombat {
    
public:
    GalactiCombatServer(bool v = false);
    virtual ~GalactiCombatServer();
    void startServer(long portNo);
    
    // Handle clients and sockets
    SDLNet_SocketSet createSockSet();
    std::vector<Client*> getClients();
    int findClientByName(std::string name);
    int findClientBySocket(TCPsocket sock);
    int findClientByChannel(int channel);
    Client* addClient(TCPsocket sock, int channel, std::string name);
    void removeClient(int clientIndex);
    void sendToAll(char *buf, bool TCP);
    
    // Server-side replacements of GalactiCombat's createScene() methods
    void createServerMinerals();
    void createServerRoom();
    
protected:
    std::vector<Client*> clients;       //all clients connected
    TCPsocket TCPServerSock;            //server socket for TCP messages
    UDPsocket UDPServerSock;            //server socket for UDP messages
    
    int state;                          //whether the server is currently running the game
//	std::string scoreboard;             //of the format "Name,Score;"
    bool verbose;                       //whether to output print statements

private:
    void serverLoop();
    void listenForConnections();
    void receiveData(int clientIndex);
    void receiveConnectionPacket(int clientIndex, Packet& incoming);
    void receiveStatePacket(int clientIndex, Packet& incoming);
    void receivePlayerInputPacket(int clientIndex, Packet& incoming);
    void receivePlayerRotationPacket(int clientIndex, Packet& incoming);
    void receiveReadyPacket(int clientIndex, Packet& incoming);
    void receiveWallsPacket(int clientIndex, Packet& incoming);
    void receiveScorePacket(int clientIndex, Packet& incoming);
    
};
#endif //#ifndef __GalactiCombatServer_h_
