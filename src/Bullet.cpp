#include "Bullet.h"

const float Bullet::SIZE_LOSS = -5.0f;
const float Bullet::SIZE_GAIN = 5.0f;
const float Bullet::RADIUS_LOSS = -2.0f;
const float Bullet::ENERGY_GAIN = 10.0f;

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
            ((SpaceShip*)other)->adjustSize(SIZE_LOSS);
            _owner->adjustSize(SIZE_GAIN);
        }
    }
    if(other->getInternalType() == "Mineral") {
        _hasHit = true;
        ((Mineral*)other)->adjustRadius(RADIUS_LOSS);
        _owner->adjustEnergy(ENERGY_GAIN);
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