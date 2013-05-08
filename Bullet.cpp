#include "Bullet.h"

const float Bullet::LOSS = -10.0f;
const float Bullet::GAIN = 20.0f;

//-------------------------------------------------------------------------------------
Bullet::Bullet (std::string name, Ogre::SceneNode* parentNode, Ogre::Entity* entity, SpaceShip* owner, int x, int y, int z)
: GameObject(name, parentNode, entity, x, y, z, 1, "Sphere5"), _hasHit(false), _owner(owner)
{
    mNode->scale(Ogre::Vector3(5 * 0.01, 5 * 0.01, 5 * 0.01));
}
//-------------------------------------------------------------------------------------
Bullet::Bullet (std::string name, Ogre::SceneNode* parentNode, SpaceShip* owner, int x, int y, int z)
: GameObject(name, parentNode, "sphere.mesh", true, x, y, z, 1, "Sphere5"), _hasHit(false), _owner(owner)
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
    if(other->getInternalType() == "SpaceShip")
    {
        _hasHit = true;
        ((SpaceShip*)other)->adjustSize(LOSS);
        _owner->adjustSize(GAIN);
    }
}
//-------------------------------------------------------------------------------------
std::string Bullet::getInternalType(void) const
{
    return "Bullet";
}
//-------------------------------------------------------------------------------------
bool Bullet::hasHit() const
{
    return _hasHit;
}
