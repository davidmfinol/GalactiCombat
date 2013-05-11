/*
 * -----------------------------------------------------------------------------
 * Filename:    GalactiCombat.cpp
 * -----------------------------------------------------------------------------
 */

#include "GalactiCombat.h"
    
const float GalactiCombat::MAX_SUB_STEPS = 6.0f;
const float GalactiCombat::TIME_STEP = 1.0f/60.0f;

//-------------------------------------------------------------------------------------
GalactiCombat::GalactiCombat(void) : minerals(MINERALS_AMOUNT), walls(6), spaceShips(0), bullets(0), isServer(false), startTime(0)
{
    physicsSimulator = new PhysicsSimulator(Mineral::MAX_RADIUS, SpaceShip::MAX_SIZE);
    mSoundMgr = new SoundManager();
    mGUIMgr = new GUIManager(SpaceShip::MIN_ENERGY, SpaceShip::MAX_ENERGY);
    mNetworkMgr = new NetworkManagerClient();
    mInputMgr = new InputManager(mNetworkMgr);
}
//-------------------------------------------------------------------------------------
GalactiCombat::~GalactiCombat(void)
{
    if(spaceShips.size() > 0 && spaceShips[0])
        delete spaceShips[0];
    delete mInputMgr;
    delete mNetworkMgr;
    delete mGUIMgr;
    delete mSoundMgr;
    delete physicsSimulator;
}
//-------------------------------------------------------------------------------------
void GalactiCombat::createCamera(void)
{
    // create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");
    // set its position, direction  
    mCamera->setPosition(Ogre::Vector3(0,150,300));
    mCamera->lookAt(Ogre::Vector3(0,100,0));
    // set the near clip distance
    mCamera->setNearClipDistance(5);
    mCamera->setFarClipDistance(500);
}
//-------------------------------------------------------------------------------------
void GalactiCombat::createViewports(void)
{
    // create one viewport, entire window
    mWindow->removeAllViewports();
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));
    // alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight())); 
}
//-------------------------------------------------------------------------------------
void GalactiCombat::destroyScene(void)
{
    while(spaceShips.size()>1) delete spaceShips.back(), spaceShips.pop_back();
    while(!walls.empty()) delete walls.back(), walls.pop_back();
    while(!minerals.empty()) delete minerals.back(), minerals.pop_back();
    mSceneMgr->destroyAllEntities();
    mSceneMgr->destroyAllCameras();
    mSceneMgr->destroyAllLights();
    mSceneMgr->clearScene();
}
//-------------------------------------------------------------------------------------
void GalactiCombat::createScene(void)
{
    // create player
    createPlayer();
    
    // create floating minerals
    createMinerals();
    
    // create walls
    createRoom();
    
    // create enemies
    spaceShips.push_back(new SpaceShip("EnemySpaceShip", new ComputerSpaceShipController(), 
                                       mSceneMgr->getRootSceneNode(), 500, 500, 500 ));
    physicsSimulator->addGameObject(spaceShips.back(), RESTITUTION, true, false);
    spaceShips.push_back(new SpaceShip("EnemySpaceShip2", new ComputerSpaceShipController(), 
                                       mSceneMgr->getRootSceneNode(), 1000, 1000, 1000 ));
    physicsSimulator->addGameObject(spaceShips.back(), RESTITUTION, true, false);
    
    // set the ambiance for the game
    this->setLighting();
    mSoundMgr->playMusic("media/sounds/Level1_destination.wav");   
}
//-------------------------------------------------------------------------------------
void GalactiCombat::createPlayer(void)
{
    // create player
    spaceShips.resize(1);
    spaceShips[0] = new SpaceShip("PlayerSpaceShip", dynamic_cast<ISpaceShipController*>(mInputMgr), 
                                  mSceneMgr->getRootSceneNode(), 200, 200, 200);
    physicsSimulator->addGameObject(spaceShips[0], RESTITUTION, true, false);
    
    // set camera to player
    spaceShips[0]->attachCamera(mCamera);
    mInputMgr->setPlayerCamera(spaceShips[0]->getSceneNode(), mCamera->getParentSceneNode());
}
//-------------------------------------------------------------------------------------
void GalactiCombat::createMinerals(void)
{
    int radius, i, pos_x, pos_y, pos_z, vel_x, vel_y, vel_z;
    std::srand (std::time(NULL));
    for (i = 0; i < minerals.size(); i++) {
        std::ostringstream o;
        o << "Mineral" << i;
        pos_x = (std::rand() % (ROOM_SIZE/2 - 250)) * (std::rand() % 2 == 0 ? 1 : -1); 
        pos_z = (std::rand() % (ROOM_SIZE/2 - 250)) * (std::rand() % 2 == 0 ? 1 : -1); 
        pos_y = (std::rand() % (ROOM_SIZE - 500)) + 250; 
        radius = (std::rand() % (Mineral::MAX_RADIUS/2 - Mineral::MIN_RADIUS+ 1)) + Mineral::MIN_RADIUS; 
        vel_x = (std::rand() % 10) * (std::rand() % 2 == 0 ? 1 : -1); 
        vel_y = (std::rand() % 5) * (std::rand() % 2 == 0 ? 1 : -1); 
        vel_z = (std::rand() % 10) * (std::rand() % 2 == 0 ? 1 : -1); 
        minerals[i] = new Mineral(o.str(), mSceneMgr->getRootSceneNode(), pos_x, pos_y, pos_z, radius);
        adjustMineralMaterial(minerals[i]);
        physicsSimulator->addGameObject(minerals[i], RESTITUTION);
        physicsSimulator->setGameObjectVelocity(minerals[i], Ogre::Vector3(vel_x, vel_y, vel_z));
    }
}
//-------------------------------------------------------------------------------------
void GalactiCombat::createRoom(void)
{
    // create planes
    Ogre::Plane ground(Ogre::Vector3::UNIT_Y, 0);
    Ogre::Plane ceiling(Ogre::Vector3::NEGATIVE_UNIT_Y, 0);
    Ogre::Plane front(Ogre::Vector3::NEGATIVE_UNIT_X, 0);
    Ogre::Plane back(Ogre::Vector3::UNIT_X, 0);
    Ogre::Plane left(Ogre::Vector3::UNIT_Z, 0);
    Ogre::Plane right(Ogre::Vector3::NEGATIVE_UNIT_Z, 0);
    
    // create ground
    Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  ground, ROOM_SIZE, ROOM_SIZE, 20, 20, true, 1, ROOM_SIZE/300, ROOM_SIZE/300, Ogre::Vector3::UNIT_Z); 
    Ogre::Entity* entGround = mSceneMgr->createEntity("GroundEntity", "ground");
    entGround->setMaterialName("Examples/WaterStream");
    entGround->setCastShadows(false);
    walls[0] = new GameObject ("ground", mSceneMgr->getRootSceneNode(), entGround, 0, 0, 0, 0, "UNIT_Y");
    physicsSimulator->addGameObject(walls[0]);
    
    // create ceiling
    Ogre::MeshManager::getSingleton().createPlane("ceiling", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  ceiling, ROOM_SIZE, ROOM_SIZE, 20, 20, true, 1, ROOM_SIZE/300, ROOM_SIZE/300, Ogre::Vector3::UNIT_Z); 
    Ogre::Entity* entCeiling = mSceneMgr->createEntity("CeilingEntity", "ceiling");
    entCeiling->setMaterialName("Examples/WaterStream");
    entCeiling->setCastShadows(false);
    walls[1] = new GameObject ("ceiling", mSceneMgr->getRootSceneNode(), entCeiling, 0, ROOM_SIZE, 0, 0, "NEGATIVE_UNIT_Y");
    physicsSimulator->addGameObject(walls[1]);
    
    // create front wall
    Ogre::MeshManager::getSingleton().createPlane("front", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  front, ROOM_SIZE, ROOM_SIZE, 20, 20, true, 1, ROOM_SIZE/300, ROOM_SIZE/300, Ogre::Vector3::UNIT_Z); 
    Ogre::Entity* entFront = mSceneMgr->createEntity("frontEntity", "front");
    entFront->setMaterialName("Examples/WaterStream");
    entFront->setCastShadows(false);
    walls[2] = new GameObject ("front", mSceneMgr->getRootSceneNode(), entFront, ROOM_SIZE/2, ROOM_SIZE/2, 0, 0, "NEGATIVE_UNIT_X");
    physicsSimulator->addGameObject(walls[2]);
    
    // create back wall
    Ogre::MeshManager::getSingleton().createPlane("back", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  back, ROOM_SIZE, ROOM_SIZE, 20, 20, true, 1, ROOM_SIZE/300, ROOM_SIZE/300, Ogre::Vector3::UNIT_Z); 
    Ogre::Entity* entBack = mSceneMgr->createEntity("backEntity", "back");
    entBack->setMaterialName("Examples/WaterStream");
    entBack->setCastShadows(false);
    walls[3] = new GameObject ("back", mSceneMgr->getRootSceneNode(), entBack, -ROOM_SIZE/2, ROOM_SIZE/2, 0, 0, "UNIT_X");
    physicsSimulator->addGameObject(walls[3]);
    
    // create left wall
    Ogre::MeshManager::getSingleton().createPlane("left", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  left, ROOM_SIZE, ROOM_SIZE, 20, 20, true, 1, ROOM_SIZE/300, ROOM_SIZE/300, Ogre::Vector3::UNIT_X); 
    Ogre::Entity* entLeft = mSceneMgr->createEntity("leftEntity", "left");
    entLeft->setMaterialName("Examples/WaterStream");
    entLeft->setCastShadows(false);
    walls[4] = new GameObject ("left", mSceneMgr->getRootSceneNode(), entLeft, 0, ROOM_SIZE/2, -ROOM_SIZE/2, 0, "UNIT_Z");
    physicsSimulator->addGameObject(walls[4]);
    
    // create right wall
    Ogre::MeshManager::getSingleton().createPlane("right", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  right, ROOM_SIZE, ROOM_SIZE, 20, 20, true, 1, ROOM_SIZE/300, ROOM_SIZE/300, Ogre::Vector3::UNIT_X); 
    Ogre::Entity* entRight = mSceneMgr->createEntity("rightEntity", "right");
    entRight->setMaterialName("Examples/WaterStream");
    entRight->setCastShadows(false);
    walls[5] = new GameObject ("right", mSceneMgr->getRootSceneNode(), entRight, 0, ROOM_SIZE/2, ROOM_SIZE/2, 0, "NEGATIVE_UNIT_Z");
    physicsSimulator->addGameObject(walls[5]);
}
//-------------------------------------------------------------------------------------
void GalactiCombat::setLighting(void)
{
    // set light attributes
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.4, 0.4, 0.4));
    mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
    
    // draw the skybox
    mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox2");
}
//-------------------------------------------------------------------------------------
void GalactiCombat::loopBackgroundMusic(void)
{
    std::time_t currentTime = std::time(0);
    std::time_t diff = currentTime - startTime;
    if (diff != 0 && diff % 130 == 0) {
        mSoundMgr->playMusic("media/sounds/Level1_destination.wav");
        startTime = currentTime;
    }
}
//-------------------------------------------------------------------------------------
void GalactiCombat::createFrameListener(void)
{
    mInputMgr->inputSetup(mWindow, mGUIMgr);
    mGUIMgr->GUIsetup(mNetworkMgr, mSoundMgr, mWindow, mInputMgr->getMouse());
    mGUIMgr->displayWelcomeMsg();
    mRoot->addFrameListener(this);
}
//-------------------------------------------------------------------------------------
bool GalactiCombat::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    static std::time_t startTime;
    static std::time_t prevTime;
    // Handle essential events
    if (mWindow->isClosed()) return false;
    if (mGUIMgr->isShutDown()) return false;
    mInputMgr->getMouse()->capture();
    mInputMgr->getKeyboard()->capture();
    mGUIMgr->getTrayMgr()->frameRenderingQueued(evt);
    
    // Only update the lobby list if the player is in the lobby
    if (mGUIMgr->isInLobby()) { //FIXME: HANDLE LOBBY LOGIC IN A SEPERATE METHOD
        if (!mGUIMgr->lobbyCountingDown()) {
            char* request = const_cast<char*>("15LIST_REQUEST");
            char *list = NULL;
            if(NetworkUtil::TCPSend(mNetworkMgr->getSocket(), request) && NetworkUtil::TCPReceive(mNetworkMgr->getSocket(), &list)) {
                mGUIMgr->setLobbyList(list);
                std::string allReady(list);
                if (allReady.find("All players ready") != std::string::npos) {
                    mGUIMgr->startCountingDown();
                    this->destroyScene();
                    this->createRoom();
                    this->createCamera();
                    this->createViewports();
                    this->setLighting();
                    this->createPlayer();
                    startTime = prevTime = std::time(0);
                }
            }
        }
        else {
            std::time_t currentTime = std::time(0);
            if (currentTime != prevTime) {
                mGUIMgr->updateLobbyList(6 - (currentTime - startTime));
                prevTime = currentTime;
            }
        }
    }

    // Only run the game loop after the welcome menu and before the game ends
    if (!mGUIMgr->isWelcomeState() && !mGUIMgr->isGameOver()) {
        
        // physics/main gameloop
        if(!mNetworkMgr->isOnline())
            gameLoop((float)evt.timeSinceLastFrame);
        if(mNetworkMgr->isOnline())
            updateFromServer();
        
        // Update GUI
        mGUIMgr->setTimeLabel(getCurrentTime()); // FIXME: TIME FROM SERVER
        mGUIMgr->informEnergy(spaceShips[0]->getEnergy());// FIXME: ENERGY FROM SERVER
        
        // Background music
        this->loopBackgroundMusic();
    }
    return true;
}
//-------------------------------------------------------------------------------------
void GalactiCombat::updateFromServer(void)
{
    // Contact server
//    mNetworkMgr->receiveData();
    mNetworkMgr->requestGameState(mSceneMgr, minerals, spaceShips, bullets);

    // Update visual components
    this->updateMinerals();
    this->updateSpaceShips();
    this->updateBullets();
}
//-------------------------------------------------------------------------------------
void GalactiCombat::gameLoop(float elapsedTime)
{
    // Update the ships
    for (int i = 0; i < spaceShips.size(); ++i) {
        // Orientation is handled by Ogre
        physicsSimulator->setGameObjectOrientation(spaceShips[i], spaceShips[i]->getSceneNode()->getOrientation());
        
        // Handle input
        if(spaceShips[i]->getEnergy() > 0) {
            Ogre::Quaternion orientation = physicsSimulator->getGameObjectOrientation(spaceShips[i]);
            Ogre::Vector3 velocity = physicsSimulator->getGameObjectVelocity(spaceShips[i]);
            if(spaceShips[i]->getController()->left()) {
                velocity -= orientation.xAxis()*elapsedTime*SpaceShip::ACCELERATION;
                spaceShips[i]->adjustEnergy(elapsedTime*SpaceShip::ENERGY_CONSUMPTION);
            }
            if(spaceShips[i]->getController()->right()) {
                velocity += orientation.xAxis()*elapsedTime*SpaceShip::ACCELERATION;
                spaceShips[i]->adjustEnergy(elapsedTime*SpaceShip::ENERGY_CONSUMPTION);
            }
            if(spaceShips[i]->getController()->up()) {
                velocity += orientation.yAxis()*elapsedTime*SpaceShip::ACCELERATION;
                spaceShips[i]->adjustEnergy(elapsedTime*SpaceShip::ENERGY_CONSUMPTION);
            }
            if(spaceShips[i]->getController()->down()) {
                velocity -= orientation.yAxis()*elapsedTime*SpaceShip::ACCELERATION;
                spaceShips[i]->adjustEnergy(elapsedTime*SpaceShip::ENERGY_CONSUMPTION);
            }
            if(spaceShips[i]->getController()->forward()) {
                velocity -= orientation.zAxis()*elapsedTime*SpaceShip::ACCELERATION;
                spaceShips[i]->adjustEnergy(elapsedTime*SpaceShip::ENERGY_CONSUMPTION);
            }
            if(spaceShips[i]->getController()->back()) {
                velocity += orientation.zAxis()*elapsedTime*SpaceShip::ACCELERATION;
                spaceShips[i]->adjustEnergy(elapsedTime*SpaceShip::ENERGY_CONSUMPTION);
            }
            physicsSimulator->setGameObjectVelocity(spaceShips[i], velocity);
            
            if(spaceShips[i]->getController()->shoot() && spaceShips[i]->canShoot()) {
                spaceShips[i]->shootBullet();
                this->createBullet(spaceShips[i]);
            }
        }
    }
    
    // Step the physics simulator
    physicsSimulator->stepSimulation(elapsedTime, MAX_SUB_STEPS, TIME_STEP);
    
    // Update after physics
    this->updateMinerals();
    this->updateSpaceShips();
    this->updateBullets();
}
//-------------------------------------------------------------------------------------
void GalactiCombat::createBullet(SpaceShip* ship)
{
        Ogre::Vector3 pos = physicsSimulator->getGameObjectPosition(ship);
        Ogre::Vector3 velocity = physicsSimulator->getGameObjectVelocity(ship);
        Ogre::Quaternion orientation = physicsSimulator->getGameObjectOrientation(ship);
        pos += orientation*Ogre::Vector3(0, 0, -2*ship->getSize());
        velocity += orientation*Ogre::Vector3(0, 0, -5000);
        
        static int bulletID = 0;
        std::string bulletName("Bullet");
        char idChar[4];
        sprintf(idChar, "%d", bulletID);
        bulletName += idChar;
        if(!isServer)
            bullets.push_back(new Bullet(bulletName, mSceneMgr->getRootSceneNode(), ship, pos.x, pos.y, pos.z));
        else
            bullets.push_back(new Bullet(bulletName, mSceneMgr->getRootSceneNode(), NULL, ship, pos.x, pos.y, pos.z));
        physicsSimulator->addGameObject(bullets.back());
        physicsSimulator->setGameObjectOrientation(bullets.back(), orientation);
        physicsSimulator->setGameObjectVelocity(bullets.back(), velocity);
        bulletID++;
}
//-------------------------------------------------------------------------------------
void GalactiCombat::updateSpaceShips(void)
{
    for (int i = 0; i < spaceShips.size(); ++i) {
        double diff = spaceShips[i]->getSizeDifference();
        if(diff!=0) {
            physicsSimulator->removeGameObject(spaceShips[i]);
            spaceShips[i]->adjustSize(diff);
            physicsSimulator->addGameObject(spaceShips[i], RESTITUTION, true, false);
        }
    }
    //TODO: SOUNDEFFECTS!
    //if(!isServer) {
    //if (camera) mSoundMgr->playSound("media/sounds/bell.wav");
    //if (camera) mSoundMgr->playSound("media/sounds/bounce.wav");}
}
//-------------------------------------------------------------------------------------
void GalactiCombat::updateMinerals(void)
{
    for (int i = 0; i < minerals.size(); ++i) {
        double diff = minerals[i]->getRadiusDifference();
        if(diff!=0) {
            physicsSimulator->removeGameObject(minerals[i]);
            minerals[i]->adjustRadius(diff);
            physicsSimulator->addGameObject(minerals[i], RESTITUTION);
        }
        if(!isServer)
            adjustMineralMaterial(minerals[i]);
    }
}
//-------------------------------------------------------------------------------------
void GalactiCombat::adjustMineralMaterial(Mineral* mineral)
{
    if (mineral->getRadius() < spaceShips[0]->getSize())
        mineral->getEntity()->setMaterialName("Examples/SphereBlue");
    else if (mineral->getRadius() > spaceShips[0]->getSize())
        mineral->getEntity()->setMaterialName("Examples/SphereMappedRustySteel");
    else
        mineral->getEntity()->setMaterialName("Examples/SphereYellow");
}
//-------------------------------------------------------------------------------------
void GalactiCombat::updateBullets(void)
{
    for(std::list<Bullet*>::iterator it = bullets.begin(); it != bullets.end(); ++it) {
        if( (*it)->isLifeOver() ) {
            physicsSimulator->removeGameObject(*it);
            physicsSimulator->deleteGameObject(*it);
            delete *it;
            it = bullets.erase(it);
        }
    }
}
//-------------------------------------------------------------------------------------
void GalactiCombat::crazyEnergyInjection(void)
{
    int i, vel_x, vel_y, vel_z;
    
    for (i = 0; i < minerals.size(); i++) {
        vel_x = ((std::rand() % 500) + 500) * (std::rand() % 2 == 0 ? 1 : -1); 
        vel_y = ((std::rand() % 500) + 500);
        vel_z = ((std::rand() % 500) + 500) * (std::rand() % 2 == 0 ? 1 : -1); 
        physicsSimulator->setGameObjectVelocity(minerals[i], Ogre::Vector3(vel_x, vel_y, vel_z));
    }
}
//-------------------------------------------------------------------------------------
std::string GalactiCombat::getCurrentTime(void)
{
    std::ostringstream o;
    static std::time_t startTime = std::time(0);
    static std::time_t prevTime = startTime;
    std::time_t currentTime = std::time(0);
    static int min = 0;
    static int sec = 59;
    if (mGUIMgr->resetTimer()) {
        startTime = std::time(0);
        prevTime = startTime;
        min = 0;
        sec = 59;
        mGUIMgr->resetTimerDone();
    }
    if (min != 0 && sec <= 10) {
        if (sec == 0) {
            crazyEnergyInjection();
        }
        mGUIMgr->countDown(sec, INJECT_CODE);
    }
    if (min == 0 && sec <= 10) {
        if (sec == 0) {
            mGUIMgr->gameOver(spaceShips[0]->getSize());
        }
        mGUIMgr->countDown(sec, OVER_CODE);
    }
    if (currentTime != prevTime) {
        prevTime = currentTime;
        if (sec == 0) {
            min--;
            sec = 59;
        }
        else {
            sec--;
        }
    }
    if (sec <= 9)
        o << min << ":0" << sec;
    else
        o << min << ":" << sec;
    return o.str();
}
