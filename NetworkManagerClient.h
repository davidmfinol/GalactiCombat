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
    int TCPConnect(char *host, char *name);
    TCPsocket& getSocket();
    bool isOnline();
    void resetReadyState();
    void quit();
    
    // Game Logic Transaction
    void sendPlayerInput(ISpaceShipController* controller);
    void receiveData(Ogre::SceneManager*, SoundManager*, std::vector<Mineral*>&, std::vector<SpaceShip*>&, std::vector<GameObject*>&);
    
protected:
    TCPsocket serverSock;
    std::string mName;
    bool connected;
};
#endif //#ifndef __NetworkManagerClient_h_
