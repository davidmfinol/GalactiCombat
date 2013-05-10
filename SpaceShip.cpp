#include "SpaceShip.h"

const double SpaceShip::ACCELERATION = 200;
const double SpaceShip::ENERGY_CONSUMPTION = -1;
const double SpaceShip::BULLET_COST = -10;
const double SpaceShip::ENERGY_MINING = 10.0;
const double SpaceShip::STARTING_ENERGY = 50;
const double SpaceShip::MIN_ENERGY = 0;
const double SpaceShip::MAX_ENERGY = 100;
const int SpaceShip::MAX_SIZE = 500;
const int SpaceShip::MIN_SIZE = 100;

//-------------------------------------------------------------------------------------
SpaceShip::SpaceShip (std::string name, ISpaceShipController* controller, Ogre::SceneNode* parentNode, Ogre::Entity* entity, int x, int y, int z, double s)
: brain(controller), size(s), sizeDifference(0), energy(STARTING_ENERGY), lastShotTime(0), GameObject(name, parentNode, entity, x, y, z, s)
{
    setupSpaceShip();
}
//-------------------------------------------------------------------------------------
SpaceShip::SpaceShip (std::string name, ISpaceShipController* controller, Ogre::SceneNode* parentNode, int x, int y, int z, double s)
: brain(controller), size(s), sizeDifference(0), energy(STARTING_ENERGY), lastShotTime(0), GameObject(name, parentNode, "SpaceShip.mesh", true, x, y, z, s)
{
    setupSpaceShip();
}
//-------------------------------------------------------------------------------------
SpaceShip::~SpaceShip() 
{
}
//-------------------------------------------------------------------------------------
void SpaceShip::setupSpaceShip()
{
    // scale the spaceship
    mNode->scale(Ogre::Vector3(size * 0.1, size * 0.1, size * 0.1));
    
    // define shape name
    std::ostringstream m;
    m << "SpaceShip" << (int)size;
    mShapeName = m.str();
}
void SpaceShip::attachCamera(Ogre::Camera* camera)
{
    //Ogre::SceneNode* camPitchNode = mNode->createChildSceneNode("CameraPitchNode"); // Traditional Camera
    Ogre::SceneNode* camPitchNode = mNode; // Rolling Camera
    camPitchNode->attachObject(camera);
    // create point light
    Ogre::Light* pointLight = mNode->getCreator()->createLight(mName + "pointLight");
    pointLight->setType(Ogre::Light::LT_POINT);
    pointLight->setPosition(Ogre::Vector3(1, 1, 1));
    pointLight->setDiffuseColour(0.4, 0.6, 0.8);
    pointLight->setSpecularColour(0.4, 0.6, 0.8);
    // create spot light
    Ogre::Light* spotLight = mNode->getCreator()->createLight(mName + "spotLight");
    spotLight->setType(Ogre::Light::LT_SPOTLIGHT);
    spotLight->setDiffuseColour(1.0, 1.0, 1.0);
    spotLight->setSpecularColour(1.0, 1.0, 1.0);
    spotLight->setDirection(0, 0, -1);
    spotLight->setPosition(Ogre::Vector3(0, -10, 100));
    spotLight->setSpotlightRange(Ogre::Degree(35), Ogre::Degree(50));
    // create special light effects on the spaceship
    camPitchNode->attachObject(pointLight);
    camPitchNode->attachObject(spotLight);
}
//-------------------------------------------------------------------------------------
ISpaceShipController* SpaceShip::getController() const 
{
    return brain;
}
//-------------------------------------------------------------------------------------
double SpaceShip::getEnergy() const 
{
    return energy;
}
//-------------------------------------------------------------------------------------
bool SpaceShip::canShoot()
{
    return std::time(0) != lastShotTime;
}
//-------------------------------------------------------------------------------------
void SpaceShip::shootBullet()
{
    lastShotTime = std::time(0);
    adjustEnergy(BULLET_COST);
}
//-------------------------------------------------------------------------------------
void SpaceShip::adjustEnergy(double e) 
{
    energy += e;
    energy = energy > MAX_ENERGY ? MAX_ENERGY : energy;
    energy = energy < MIN_ENERGY ? MIN_ENERGY : energy;
}
//-------------------------------------------------------------------------------------
double SpaceShip::getSize() const
{
    return size;
}
//-------------------------------------------------------------------------------------
double SpaceShip::getSizeDifference() const
{
    return sizeDifference;
}
//-------------------------------------------------------------------------------------
void SpaceShip::adjustSize(double s) 
{
    // scale the spaceship to the new size
    double originalSize = 10 / size;
    size += s;
    size = size < MIN_SIZE ? MIN_SIZE : size;
    size = size > MAX_SIZE ? MAX_SIZE : size;
    double reScale = size * 0.1;
    mNode->scale(Ogre::Vector3(originalSize * reScale, originalSize * reScale, originalSize * reScale));
    
    // reset size/shape
    std::ostringstream m;
    m << "SpaceShip" << (int)size;
    mShapeName = m.str();
    sizeDifference = 0;
}
//-------------------------------------------------------------------------------------
int SpaceShip::getMass() const 
{
    return size;
}
//-------------------------------------------------------------------------------------
void SpaceShip::collidedWith(GameObject* other)
{
    if("Mineral" == other->getInternalType()) {
        std::time_t currentTimeStamp = std::time(0);
        if (currentTimeStamp != mCollisionTimeStamp) { 
            if(((Mineral*)other)->getRadius() <= size) { // the mineral is smaller than us; we can mine it
                energy += ENERGY_MINING;
                energy = energy > MAX_ENERGY ? MAX_ENERGY : energy;
            }
            mCollisionTimeStamp = currentTimeStamp;
        }
    }
    
    if("SpaceShip" == other->getInternalType()) {
        std::time_t currentTimeStamp = std::time(0);
        if (currentTimeStamp != mCollisionTimeStamp) { // We are bullies: bigger gets bigger, smaller gets smaller
            if(((SpaceShip*)other)->getSize() < size)
                sizeDifference = 10;
            else if(((SpaceShip*)other)->getSize() > size)
                sizeDifference = -10;
            mCollisionTimeStamp = currentTimeStamp;
        }
    }
}
//-------------------------------------------------------------------------------------
std::string SpaceShip::getInternalType() const
{
    return "SpaceShip";
}
