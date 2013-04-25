#ifndef __GalactiCombatServer_h_
#define __GalactiCombatServer_h_

#include <vector>
#include "GalactiCombat.h"
#include "NetworkUtil.h"
#include "ClientSpaceShipController.h"

#define PAUSE 0
#define PLAY 1

typedef struct {
    TCPsocket sock;
	int channel;
    std::string name;
    ClientSpaceShipController* inputController;
    bool ready;
} Client;

class GalactiCombatServer : public GalactiCombat {
    
public:
    GalactiCombatServer();
    virtual ~GalactiCombatServer();
    
    void startServer(long portNo);
    std::vector<Client*> getClients();
    Client* findClientByName(std::string name);
    Client* findClientBySocket(TCPsocket sock);
    
protected:
    std::vector<Client*> clients;        // all clients connected
    TCPsocket TCPServerSock;            //server socket for TCP messages
	UDPsocket UDPServerSock;			//server socket for UDP messages
    std::string scoreboard;             //of the format "Name,Score;"
    int state;
    double lastFrameTime;
    
    /***** Member functions ******/
    void movePlayer(int i, std::string direction);
    void createServerMinerals();
    void createServerRoom();
    void serverLoop();
    void listenForConnections();
    void receiveData(const Packet& incoming, int i);
    void sendMineral(Mineral* mineral);
    void sendSpaceShip(SpaceShip* spaceShip);
    //--------------------------------------------------------------------------
    /*
     *        addClient(TCPsocket, char*):
     *        This method adds a client to the list of clients and constructs a new Ball instance
     *        for them to control.
     *        This was copied from add_client() from tcpmultiserver.c in the SDLNet example code.
     * 
     *        sock: the socket that corresponds to this client
     *        name: the name associated with this client
     * 
     *        returns: a pointer the Client object associated with this client
     */
    Client *addClient(TCPsocket sock, int channel, std::string name);
    //--------------------------------------------------------------------------
    /*
     *        removeClient(int):
     *        This method removes a client from the list of clients.
     * 
     *        -i: the index number of the client to be removed
     */
    void removeClient(int i);
    //--------------------------------------------------------------------------
    /*
     *        sendToAll(char*, bool):
     *        This method takes the data stored in the address given in the first argument
     *        and sends it to all the clients that are connected.
     * 
     *        buf: the data to be sent to everyone
     *        TCP: a boolean that answers the question "Do you want to send this using TCP?"
     *                At the time this comment was written, UDPSend has not been written, so 
     *                always set this to true.
     */
    void sendToAll(char *buf, bool TCP);
    //--------------------------------------------------------------------------
    /*
     *          createSocketSet():
     *          This method returns an SDLNet_SocketSet containing the server
     *          socket and all the client sockets.
     */
    SDLNet_SocketSet createSockSet();
    //--------------------------------------------------------------------------
    
    
};
#endif //#ifndef __GalactiCombatServer_h_
