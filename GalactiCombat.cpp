/*
 * -----------------------------------------------------------------------------
 * Filename:    GalactiCombat.cpp
 * -----------------------------------------------------------------------------
 */

#include "GalactiCombat.h"

//-------------------------------------------------------------------------------------
GalactiCombat::GalactiCombat(void) : minerals(MINERALS_AMOUNT), spaceShips(0), walls(6), isServer(false)
{
    physicsSimulator = new PhysicsSimulator(Mineral::MIN_RADIUS, Mineral::MAX_RADIUS);
    mSoundMgr = new SoundManager();
    mGUIMgr = new GUIManager(SpaceShip::MIN_ENERGY, SpaceShip::MAX_ENERGY);
    mNetworkMgr = new NetworkManagerClient();
    mInputMgr = new InputManager(mNetworkMgr);
    startTime = std::time(0);
}
//-------------------------------------------------------------------------------------
GalactiCombat::~GalactiCombat(void)
{
    delete mNetworkMgr;
    delete mInputMgr;
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
    mCamera->setPosition(Ogre::Vector3(0,100,400));
    mCamera->lookAt(Ogre::Vector3(0,0,0));
    // set the near clip distance
    mCamera->setNearClipDistance(5);
    mCamera->setFarClipDistance(500);
}
//-------------------------------------------------------------------------------------
void GalactiCombat::createViewports(void)
{
    // create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));
    // alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));    
}
//-------------------------------------------------------------------------------------
void GalactiCombat::destroyScene(void)
{
    while(!spaceShips.empty()) delete spaceShips.back(), spaceShips.pop_back();
    while(!minerals.empty()) delete minerals.back(), minerals.pop_back();
    while(!walls.empty()) delete walls.back(), walls.pop_back();
    mSceneMgr->destroyAllEntities();
    mSceneMgr->clearScene();
}
//-------------------------------------------------------------------------------------
void GalactiCombat::createScene(void)
{
    // create player
    spaceShips.resize(1);
    spaceShips[0] = new SpaceShip("PlayerSpaceShip", mSoundMgr, 
                         dynamic_cast<ISpaceShipController*>(mInputMgr), mSceneMgr->getRootSceneNode()->createChildSceneNode("PlayerNode", Ogre::Vector3(100, 100, 100)), 30, mCamera);
    physicsSimulator->addGameObject(spaceShips[0], RESTITUTION, true, true);
    // create floating minerals
    createMinerals();
    // create enemy
    spaceShips.push_back(new SpaceShip("EnemySpaceShip", mSoundMgr, 
                                        new ComputerSpaceShipController(), mSceneMgr->getRootSceneNode()->createChildSceneNode("EnemyNode", Ogre::Vector3(500, 500, 500)) ));
    physicsSimulator->addGameObject(spaceShips.back(), RESTITUTION, true, true);
    // create walls
    createRoom();
    
    // Lights!
    createLights();
    // Camera!
    mInputMgr->setPlayerCamera(spaceShips[0]->getSceneNode()->getParentSceneNode(), mCamera->getParentSceneNode());
    // Music!
    mSoundMgr->playMusic("media/sounds/Level1_destination.wav");
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
        pos_y = (std::rand() % (ROOM_HIGH - 500)) + 250; 
        radius = (std::rand() % (Mineral::MAX_RADIUS/2 - Mineral::MIN_RADIUS+ 1)) + Mineral::MIN_RADIUS; 
        vel_x = (std::rand() % 10) * (std::rand() % 2 == 0 ? 1 : -1); 
        vel_y = (std::rand() % 5) * (std::rand() % 2 == 0 ? 1 : -1); 
        vel_z = (std::rand() % 10) * (std::rand() % 2 == 0 ? 1 : -1); 
        minerals[i] = new Mineral(o.str(), mSoundMgr, mSceneMgr->getRootSceneNode(), pos_x, pos_y, pos_z, radius);
        adjustMineralMaterial(minerals[i]);
        physicsSimulator->addGameObject(minerals[i], RESTITUTION);
        physicsSimulator->setGameObjectVelocity(minerals[i], Ogre::Vector3(vel_x, vel_y, vel_z));
    }
}
//-------------------------------------------------------------------------------------
void GalactiCombat::createLights(void)
{
    // set scene lights' attributes
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.4, 0.4, 0.4));
    mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
    // draw the skybox
    mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox2");
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
    walls[1] = new GameObject ("ceiling", mSceneMgr->getRootSceneNode(), entCeiling, 0, ROOM_HIGH, 0, 0, "NEGATIVE_UNIT_Y");
    physicsSimulator->addGameObject(walls[1]);
    
    // create front wall
    Ogre::MeshManager::getSingleton().createPlane("front", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  front, ROOM_HIGH, ROOM_SIZE, 20, 20, true, 1, ROOM_HIGH/300, ROOM_SIZE/300, Ogre::Vector3::UNIT_Z); 
    Ogre::Entity* entFront = mSceneMgr->createEntity("frontEntity", "front");
    entFront->setMaterialName("Examples/WaterStream");
    entFront->setCastShadows(false);
    walls[2] = new GameObject ("front", mSceneMgr->getRootSceneNode(), entFront, ROOM_SIZE/2, ROOM_HIGH/2, 0, 0, "NEGATIVE_UNIT_X");
    physicsSimulator->addGameObject(walls[2]);
    
    // create back wall
    Ogre::MeshManager::getSingleton().createPlane("back", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  back, ROOM_HIGH, ROOM_SIZE, 20, 20, true, 1, ROOM_HIGH/300, ROOM_HIGH/300, Ogre::Vector3::UNIT_Z); 
    Ogre::Entity* entBack = mSceneMgr->createEntity("backEntity", "back");
    entBack->setMaterialName("Examples/WaterStream");
    entBack->setCastShadows(false);
    walls[3] = new GameObject ("back", mSceneMgr->getRootSceneNode(), entBack, -ROOM_SIZE/2, ROOM_HIGH/2, 0, 0, "UNIT_X");
    physicsSimulator->addGameObject(walls[3]);
    
    // create left wall
    Ogre::MeshManager::getSingleton().createPlane("left", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  left, ROOM_HIGH, ROOM_SIZE, 20, 20, true, 1, ROOM_HIGH/300, ROOM_SIZE/300, Ogre::Vector3::UNIT_X); 
    Ogre::Entity* entLeft = mSceneMgr->createEntity("leftEntity", "left");
    entLeft->setMaterialName("Examples/WaterStream");
    entLeft->setCastShadows(false);
    walls[4] = new GameObject ("left", mSceneMgr->getRootSceneNode(), entLeft, 0, ROOM_HIGH/2, -ROOM_SIZE/2, 0, "UNIT_Z");
    physicsSimulator->addGameObject(walls[4]);
    
    // create right wall
    Ogre::MeshManager::getSingleton().createPlane("right", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  right, ROOM_HIGH, ROOM_SIZE, 20, 20, true, 1, ROOM_HIGH/300, ROOM_SIZE/300, Ogre::Vector3::UNIT_X); 
    Ogre::Entity* entRight = mSceneMgr->createEntity("rightEntity", "right");
    entRight->setMaterialName("Examples/WaterStream");
    entRight->setCastShadows(false);
    walls[5] = new GameObject ("right", mSceneMgr->getRootSceneNode(), entRight, 0, ROOM_HIGH/2, ROOM_SIZE/2, 0, "NEGATIVE_UNIT_Z");
    physicsSimulator->addGameObject(walls[5]);
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
    if (mGUIMgr->isInLobby()) {
        if (!mGUIMgr->lobbyCountingDown()) {
            char* request = const_cast<char*>("15LIST_REQUEST");
            char *list = NULL;
            if(TCPSend(mNetworkMgr->getSocket(), request) && TCPReceive(mNetworkMgr->getSocket(), &list)) {
                mGUIMgr->setLobbyList(list);
                std::string allReady(list);
                if (allReady.find("All players ready") != std::string::npos) {
                    mGUIMgr->startCountingDown();
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
        
        if(mNetworkMgr->isOnline())
            updateFromServer();
        else
            gameLoop((float)evt.timeSinceLastFrame);
        
        // Update GUI
        mGUIMgr->setTimeLabel(getCurrentTime()); // FIXME: TIME FROM SERVER
        // TODO: mGUIMgr->informSize(spaceShips[0]->getSize());
        mGUIMgr->informEnergy(spaceShips[0]->getEnergy());// FIXME: ENERGY FROM SERVER
        // Background music
        this->loopBackgroundMusic();
    }
    return true;
}
//-------------------------------------------------------------------------------------
void GalactiCombat::updateFromServer(void)
{
    //mNetworkMgr->receiveData(mSceneMgr, mSoundMgr, minerals,spaceShips,walls);
    //vector<Mineral*> newMinerals = mInputMgr->receiveMinerals();
    //vector<SpaceShip*> newSpaceShips = mInputMgr->receiveSpaceShips();
    //this->updateSpaceShips(newSpaceShips);
    //this->updateMineralMaterial(newMinerals);
}
//-------------------------------------------------------------------------------------
void GalactiCombat::gameLoop(float elapsedTime)
{
    // Update the physics for the ships
    for (int i = 0; i < spaceShips.size(); ++i) {
        //Ogre::Quaternion orientation = physicsSimulator->getGameObjectOrientation(spaceShips[i], true); //FIXME: USE THIS LINE INSTEAD OF TWO BELOW
        Ogre::Quaternion orientation = spaceShips[i]->getSceneNode()->getParentSceneNode()->getOrientation();
        //Ogre::Quaternion orientation = physicsSimulator->getGameObjectOrientation(spaceShips[i]);
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
        //if(spaceShips[i]->getController()->shoot()) 
        physicsSimulator->setGameObjectVelocity(spaceShips[i], velocity);
    }
    
    // Step the physics simulator
    physicsSimulator->stepSimulation(elapsedTime);
    
    // Update objects after the results of the physics step
    this->updateSpaceShips();
    this->updateMinerals();
}
//-------------------------------------------------------------------------------------
void GalactiCombat::updateSpaceShips(void)
{
    //TODO:
    //if(!isServer)
    //adjustSpaceShipMaterial()
}
//-------------------------------------------------------------------------------------
void GalactiCombat::adjustSpaceShipMaterial(SpaceShip* ship)
{
    //TODO:
}
//-------------------------------------------------------------------------------------
void GalactiCombat::updateMinerals(void)
{
    // Update all the minerals
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
void GalactiCombat::crazyEnergyInjection(void)
{
    int i, vel_x, vel_y, vel_z;
    
    vel_x = ((std::rand() % 500) + 500) * (std::rand() % 2 == 0 ? 1 : -1); 
    vel_y = ((std::rand() % 500) + 500);
    vel_z = ((std::rand() % 500) + 500) * (std::rand() % 2 == 0 ? 1 : -1); 
    physicsSimulator->setGameObjectVelocity(spaceShips[0], Ogre::Vector3(vel_x, vel_y, vel_z));
    
    for (i = 0; i < minerals.size(); i++) {
        vel_x = ((std::rand() % 500) + 500) * (std::rand() % 2 == 0 ? 1 : -1); 
        vel_y = ((std::rand() % 500) + 500);
        vel_z = ((std::rand() % 500) + 500) * (std::rand() % 2 == 0 ? 1 : -1); 
        physicsSimulator->setGameObjectVelocity(minerals[i], Ogre::Vector3(vel_x, vel_y, vel_z));
    }
}
//-------------------------------------------------------------------------------------
void GalactiCombat::loopBackgroundMusic(void) {
    std::time_t currentTime = std::time(0);
    std::time_t diff = currentTime - startTime;
    if (diff != 0 && diff % 130 == 0) {
        mSoundMgr->playMusic("media/sounds/cautious-path.wav");
        startTime = currentTime;
    }
}
//-------------------------------------------------------------------------------------
std::string GalactiCombat::getCurrentTime(void) {
    std::ostringstream o;
    static std::time_t startTime = std::time(0);
    static std::time_t prevTime = startTime;
    std::time_t currentTime = std::time(0);
    static int min = 0;
    static int sec = 20;
    if (mGUIMgr->resetTimer()) {
        startTime = std::time(0);
        prevTime = startTime;
        min = 0;
        sec = 20;
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
