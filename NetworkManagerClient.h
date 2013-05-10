#ifndef __NetworkManagerClient_h_
#define __NetworkManagerClient_h_

#ifndef WIN32
#include <unistd.h>
#include <sys/time.h>

#else
#include <windows.h>
#include <time.h>

#endif

#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <vector>
#include <OgreSceneManager.h>
#include <OgreQuaternion.h>

#include "NetworkUtil.h"
#include "GameObject.h"
#include "Mineral.h"
#include "SpaceShip.h"
#include "Bullet.h"
#include "ISpaceShipController.h"
#include "ClientSpaceShipController.h"

#define MAXLEN (10*1024) //10 KB

class NetworkManagerClient {
    
public:
    NetworkManagerClient();
    virtual ~NetworkManagerClient();
    
    // Connection Data
    int connect(char *host, char *name);
    TCPsocket& getSocket();
    bool isOnline();
    void resetReadyState();
    void quit();
    
    // Game Logic Transaction
    void sendPlayerInput(ISpaceShipController* controller);
    void sendPlayerRotation(const Ogre::Quaternion& rotation);
	void receiveData();
    void requestGameState(Ogre::SceneManager*, std::vector<Mineral*>&, std::vector<SpaceShip*>&, std::list<Bullet*>&);
    
protected:
    TCPsocket TCPServerSock;
    UDPsocket UDPServerSock;
    IPaddress serverIP;
    std::string mName;
    bool connected;
	SDLNet_SocketSet set;
};
#endif //#ifndef __NetworkManagerClient_h_
