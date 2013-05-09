#include "SpaceShip.h"

const double SpaceShip::ACCELERATION = 200;
const double SpaceShip::ENERGY_CONSUMPTION = -1;
const double SpaceShip::ENERGY_MINING = 10.0;
const double SpaceShip::STARTING_ENERGY = 50;
const double SpaceShip::MIN_ENERGY = 0;
const double SpaceShip::MAX_ENERGY = 100;
const int SpaceShip::MAX_SIZE = 500;
const int SpaceShip::MIN_SIZE = 100;

//-------------------------------------------------------------------------------------
SpaceShip::SpaceShip (std::string name, ISpaceShipController* controller, Ogre::SceneNode* parentNode, Ogre::Entity* entity, int x, int y, int z, double s)
: creation_time(0), brain(controller), size(s), sizeDifference(0), energy(STARTING_ENERGY), GameObject(name, parentNode, entity, x, y, z, s)
{
    setupSpaceShip();
}
//-------------------------------------------------------------------------------------
SpaceShip::SpaceShip (std::string name, ISpaceShipController* controller, Ogre::SceneNode* parentNode, int x, int y, int z, double s)
: creation_time(0), brain(controller), size(s), sizeDifference(0), energy(STARTING_ENERGY), GameObject(name, parentNode, "SpaceShip.mesh", true, x, y, z, s)
{
    setupSpaceShip();
}
//-------------------------------------------------------------------------------------
SpaceShip::~SpaceShip(void) 
{
}
//-------------------------------------------------------------------------------------
void SpaceShip::setupSpaceShip(void)
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
bool SpaceShip::bulletFlying()
{
    return creation_time;
}
//-------------------------------------------------------------------------------------
bool SpaceShip::isLifeOver()
{
    return std::time(0) >= (creation_time + 3);
}
//-------------------------------------------------------------------------------------
void SpaceShip::bulletCreated()
{
    creation_time = std::time(0);
}
//-------------------------------------------------------------------------------------
void SpaceShip::bulletDestroyed()
{
    creation_time = 0;
}
//-------------------------------------------------------------------------------------
void SpaceShip::adjustEnergy(double e) 
{
    energy += e;
    energy = energy > MAX_ENERGY ? MAX_ENERGY : energy;
    energy = energy < MIN_ENERGY ? MIN_ENERGY : energy;
}
//-------------------------------------------------------------------------------------
double SpaceShip::getSize(void) const
{
    return size;
}
//-------------------------------------------------------------------------------------
double SpaceShip::getSizeDifference(void) const
{
    return sizeDifference;
}
//-------------------------------------------------------------------------------------
void SpaceShip::adjustSize(double s) 
{
    /*
    // scale the spaceship to the new size
    double originalSize = 100 / size;
    //size += r;
    //size = size < minSpaceShipSize ? minSpaceShipSize : size;
    //size = size > maxSpaceShipSize ? maxSpaceShipSize : size;
    double reScale = size * 0.01;
    mNode->scale(Ogre::Vector3(originalSize * reScale, originalSize * reScale, originalSize * reScale));
    
    // reset size/shape
    std::ostringstream m;
    m << "Sphere" << (int)size;
    mShapeName = m.str();
    sizeDifference = 0;
    */
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
            else if(((Mineral*)other)->getRadius() > size) { // mineral is too large; take damage
                //sizeDifference = RADIUS_DECREASE;
            }
            mCollisionTimeStamp = currentTimeStamp;
        }
    }
}
//-------------------------------------------------------------------------------------
std::string SpaceShip::getInternalType(void) const
{
    return "SpaceShip";
}
