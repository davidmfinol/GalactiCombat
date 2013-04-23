#include "Bullet.h"

//-------------------------------------------------------------------------------------
Bullet::Bullet(std::string name, Ogre::SceneNode* parentNode, int x, int y, int z) 
: GameObject(name, parentNode, "sphere.mesh", true, x, y, z, 1, "Sphere5")
{
}
//-------------------------------------------------------------------------------------
Bullet::~Bullet(void) 
{
}
//-------------------------------------------------------------------------------------
void Bullet::collidedWith(GameObject* other)
{
}
//-------------------------------------------------------------------------------------
std::string Bullet::getInternalType(void) const
{
    return "Bullet";
}