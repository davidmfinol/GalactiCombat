#include "Bullet.h"

const float Bullet::LOSS = -5.0f;
const float Bullet::GAIN = 5.0f;

//-------------------------------------------------------------------------------------
Bullet::Bullet (std::string name, Ogre::SceneNode* parentNode, Ogre::Entity* entity, SpaceShip* owner, int x, int y, int z)
: GameObject(name, parentNode, entity, x, y, z, 1, "Sphere5"), _hasHit(false), _owner(owner), _creationTime(std::time(0)), _exists(true)
{
    mNode->scale(Ogre::Vector3(5 * 0.01, 5 * 0.01, 5 * 0.01));
}
//-------------------------------------------------------------------------------------
Bullet::Bullet (std::string name, Ogre::SceneNode* parentNode, SpaceShip* owner, int x, int y, int z)
: GameObject(name, parentNode, "sphere.mesh", true, x, y, z, 1, "Sphere5"), _hasHit(false), _owner(owner), _creationTime(std::time(0)), _exists(true)
{
    mNode->scale(Ogre::Vector3(5 * 0.01, 5 * 0.01, 5 * 0.01));
}
//-------------------------------------------------------------------------------------
Bullet::~Bullet(void) 
{
}
//-------------------------------------------------------------------------------------
void Bullet::collidedWith(GameObject* other)
{
    if(other->getInternalType() == "SpaceShip") {
        _hasHit = true;
        if( (SpaceShip*)other != _owner ) {
            ((SpaceShip*)other)->adjustSize(LOSS);
            _owner->adjustSize(GAIN);
        }
    }
    if(other->getInternalType() == "Mineral") {
        _hasHit = true;
        ((Mineral*)other)->adjustRadius(LOSS);
        _owner->adjustEnergy(GAIN);
    }
}
//-------------------------------------------------------------------------------------
std::string Bullet::getInternalType(void) const
{
    return "Bullet";
}
//-------------------------------------------------------------------------------------
SpaceShip* Bullet::getOwner() const
{
    return _owner;
}
//-------------------------------------------------------------------------------------
bool Bullet::hasHit() const
{
    return _hasHit;
}
//-------------------------------------------------------------------------------------
bool Bullet::isLifeOver() const
{
    return std::time(0) >= (_creationTime + 3) || _hasHit || !_exists;
}
//-------------------------------------------------------------------------------------
void Bullet::setExist(bool b)
{
    _exists = b;
}