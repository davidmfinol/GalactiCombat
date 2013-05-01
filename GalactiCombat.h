/*
 * -----------------------------------------------------------------------------
 * Filename:    GalactiCombat.h
 * -----------------------------------------------------------------------------
 */
#ifndef __GalactiCombat_h_
#define __GalactiCombat_h_

#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <string>
#include <cstdlib>

#include "BaseApplication.h"
#include "GameObject.h"
#include "Mineral.h"
#include "PhysicsSimulator.h"
#include "GUIManager.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "NetworkManagerClient.h"
#include "SpaceShip.h"
#include "ComputerSpaceShipController.h"
#include "Bullet.h"

#define ROOM_SIZE 2500
#define ROOM_HIGH 2500
#define RESTITUTION 0.9
#define MINERALS_AMOUNT 10
#define INJECT_CODE 2
#define OVER_CODE 3

class GalactiCombat : public BaseApplication, public Ogre::FrameListener {
    
public:
    GalactiCombat(void);
    virtual ~GalactiCombat(void);
    
protected:
    // Setup and cleanup
    virtual void createViewports(void);
    virtual void createCamera(void);
    virtual void destroyScene(void);
    virtual void createScene(void);
    void createLights(void);
    void createRoom(void);
    void createMinerals(void);
    void loopBackgroundMusic(void);
    
    // Ogre::FrameListener
    virtual void createFrameListener(void);
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
    
    // Game logic
    void updateFromServer(void);
    void gameLoop(float elapsedTime);
    void createBullet(SpaceShip* ship);
    void updateMinerals(void);
    void adjustMineralMaterial(Mineral* mineral);
    void updateSpaceShips(void);
    void adjustSpaceShipMaterial(SpaceShip* ship);
    void updateBullets(void);
    void crazyEnergyInjection(void);
    std::string getCurrentTime(void);
    
    // Game objects
    std::vector<GameObject*> walls;
    std::vector<Mineral*> minerals;
    std::vector<SpaceShip*> spaceShips;
    std::vector<Bullet*> bullets;
    
    // For music
    std::time_t startTime;
    // For networking
    bool isServer;
    
    // Game Engine Components
    PhysicsSimulator* physicsSimulator;
    SoundManager* mSoundMgr;
    GUIManager* mGUIMgr;
    InputManager* mInputMgr;
    NetworkManagerClient* mNetworkMgr;
};

#endif // #ifndef __GalactiCombat_h_
