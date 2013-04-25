#ifndef __NetworkManagerClient_h_
#define __NetworkManagerClient_h_

#include <vector>
#include <OgreSceneManager.h>
#include "SoundManager.h"
#include "NetworkUtil.h"
#include "ISpaceShipController.h"
#include "ClientSpaceShipController.h"
#include "Mineral.h"
#include "GameObject.h"
#include "SpaceShip.h"

#define MAXLEN (10*1024) //10 KB

class NetworkManagerClient {
    
public:
    NetworkManagerClient();
    virtual ~NetworkManagerClient();
    
    // Connection Data
    int connect(char *host, char *name);
    bool isOnline();
    void resetReadyState();
    void quit();  
    TCPsocket& getSocket() {return this->TCPServerSock;}
    
    // Scores
    void sendPlayerScore(double score);
    std::string getPlayerScores();
    
    // Game Logic Transaction
    void sendPlayerInput(ISpaceShipController* controller);
    void receiveData(Ogre::SceneManager*, SoundManager*, std::vector<Mineral*>&, std::vector<SpaceShip*>&, std::vector<GameObject*>&);
    
protected:
    TCPsocket TCPServerSock;
	UDPsocket UDPServerSock;
	IPaddress serverIP;
    std::string mName;
    std::string scores;
    bool connected;
};
#endif //#ifndef __NetworkManagerClient_h_
