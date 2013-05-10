#include "GameObject.h"

//-------------------------------------------------------------------------------------
GameObject::GameObject(std::string name, Ogre::SceneNode* parentNode, Ogre::Entity* entity, int x, int y, int z, int mass, std::string shapeName) 
: mName(name), mEntity(entity), mNode(0) , mMass(mass), mShapeName(shapeName), mCollisionTimeStamp(0)
{
    mNode = parentNode->createChildSceneNode(name + "Node", Ogre::Vector3(x, y, z));
    if(mEntity)
        mNode->attachObject(mEntity);
}
//-------------------------------------------------------------------------------------
GameObject::GameObject(std::string name, Ogre::SceneNode* parentNode, std::string mesh, bool shadow, int x, int y, int z, int mass, std::string shapeName) 
: mName(name), mNode(0), mMass(mass), mShapeName(shapeName), mCollisionTimeStamp(0)
{
    // create entity
    mEntity = parentNode->getCreator()->createEntity(name + "Entity", mesh);
    mEntity->setCastShadows(shadow);
    // create node
    mNode = parentNode->createChildSceneNode(name + "Node", Ogre::Vector3(x, y, z));
    if(mEntity)
        mNode->attachObject(mEntity);
}
//-------------------------------------------------------------------------------------
GameObject::~GameObject(void) 
{
	/*
    if(mEntity)
    {
        mNode->detachObject(mEntity);
        mNode->getCreator()->destroyEntity(mEntity);
    }
    mNode->getCreator()->destroySceneNode(mNode);
	*/
}
//-------------------------------------------------------------------------------------
std::string GameObject::getName(void) const
{
    return mName;
}
//-------------------------------------------------------------------------------------
Ogre::Entity* GameObject::getEntity(void) const
{
    return mEntity;
}
//-------------------------------------------------------------------------------------
Ogre::SceneNode* GameObject::getSceneNode(void) const
{
    return mNode;
}
//-------------------------------------------------------------------------------------
int GameObject::getMass(void) const
{
    return mMass;
}
//-------------------------------------------------------------------------------------
std::string GameObject::getShapeName(void) const
{
    return mShapeName;
}
//-------------------------------------------------------------------------------------
void GameObject::collidedWith(GameObject* other)
{
}
//-------------------------------------------------------------------------------------
std::string GameObject::getInternalType(void) const
{
    return "GameObject";
}
