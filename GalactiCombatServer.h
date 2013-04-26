#ifndef __GalactiCombatServer_h_
#define __GalactiCombatServer_h_

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
    ClientSpaceShipController* inputController;
    bool ready;
};
typedef struct Client Client; // allows Client struct to be used in C

class GalactiCombatServer : public GalactiCombat {
    
public:
    GalactiCombatServer();
    virtual ~GalactiCombatServer();
    void startServer(long portNo);
    
    // Handle clients and sockets
    SDLNet_SocketSet createSockSet();
    std::vector<Client*> getClients();
    Client* findClientByName(std::string name);
    Client* findClientBySocket(TCPsocket sock);
    Client* addClient(TCPsocket sock, int channel, std::string name);
    void removeClient(int i);
    void sendToAll(char *buf, bool TCP);
    
    // Server-side replacements of GalactiCombat's createScene() methods
    void createServerMinerals();
    void createServerRoom();
    
protected:
    std::vector<Client*> clients;       //all clients connected
    TCPsocket TCPServerSock;            //server socket for TCP messages
    UDPsocket UDPServerSock;            //server socket for UDP messages
    
    int state;                          //whether the server is currently running the game
    std::time_t lastFrameTime;          //time of the last occurence of gameLoop()
    std::string scoreboard;             //of the format "Name,Score;"

private:
    void serverLoop();
    void listenForConnections();
    void receiveData(const Packet& incoming, int i);
    void sendMineral(Mineral* mineral, int i);
    void sendSpaceShip(SpaceShip* spaceShip, int i);
    void movePlayer(int i, std::string direction);
    
};
#endif //#ifndef __GalactiCombatServer_h_
