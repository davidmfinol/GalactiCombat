#include "Mineral.h"

const int Mineral::MAX_RADIUS = 100;
const int Mineral::MIN_RADIUS = 10;
const double Mineral::RADIUS_INCREASE = 5.0;
const double Mineral::RADIUS_DECREASE = -5.0;

//-------------------------------------------------------------------------------------
Mineral::Mineral(std::string name, Ogre::SceneNode* parentNode, Ogre::Entity* entity,
    int x, int y, int z, double r)
: radius(r), radiusDifference(0), GameObject(name, parentNode, entity, x, y, z)
{
    setupMineral();
}
//-------------------------------------------------------------------------------------
Mineral::Mineral(std::string name, Ogre::SceneNode* parentNode,
    int x, int y, int z, double r)
: radius(r), radiusDifference(0), GameObject(name, parentNode, "sphere.mesh", true, x, y, z)
{
    setupMineral();
}
//-------------------------------------------------------------------------------------
Mineral::~Mineral(void) 
{
}
//-------------------------------------------------------------------------------------
void Mineral::setupMineral(void)
{
    // scale the mineral
    mNode->scale(Ogre::Vector3(radius * 0.01, radius * 0.01, radius * 0.01));
    
    // define shape name
    std::ostringstream m;
    m << "Sphere" << (int)radius;
    mShapeName = m.str();
}
//-------------------------------------------------------------------------------------
double Mineral::getRadius(void) const
{
    return radius;
}
//-------------------------------------------------------------------------------------
double Mineral::getRadiusDifference(void) const
{
    return radiusDifference;
}
//-------------------------------------------------------------------------------------
void Mineral::adjustRadius(double r) 
{
    // scale the mineral to the new radius
    double originalSize = 100 / radius;
    radius += r;
    radius = radius < MIN_RADIUS ? MIN_RADIUS : radius;
    radius = radius > MAX_RADIUS ? MAX_RADIUS : radius;
    double reScale = radius * 0.01;
    mNode->scale(Ogre::Vector3(originalSize * reScale, originalSize * reScale, originalSize * reScale));
    
    // reset radius/shape
    std::ostringstream m;
    m << "Sphere" << (int)radius;
    mShapeName = m.str();
    radiusDifference = 0;
}
//-------------------------------------------------------------------------------------
int Mineral::getMass(void) const
{
    return radius/5;
}
//-------------------------------------------------------------------------------------
void Mineral::collidedWith(GameObject* other)
{
    if("Mineral" == other->getInternalType()) {
        std::time_t currentTimeStamp = std::time(0);
        if (currentTimeStamp != mCollisionTimeStamp) { 
            if(((Mineral*)other)->getRadius() < radius)
                radiusDifference = RADIUS_INCREASE;
            else if(((Mineral*)other)->getRadius() > radius)
                radiusDifference = RADIUS_DECREASE;
            mCollisionTimeStamp = currentTimeStamp;
        }
    }
}
//-------------------------------------------------------------------------------------
std::string Mineral::getInternalType(void) const
{
    return "Mineral";
}