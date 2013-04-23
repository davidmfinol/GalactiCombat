#ifndef __GameObject_h_
#define __GameObject_h_

#include <string>
#include <time.h>
#include <OgreSceneManager.h>
#include <OgreEntity.h>

class GameObject {
    
public:
    GameObject (std::string name, Ogre::SceneNode* parentNode, Ogre::Entity* entity, int x = 0, int y = 0, int z = 0, int mass = 0, std::string shapeName = "Box");
    GameObject (std::string name, Ogre::SceneNode* parentNode, std::string mesh, bool shadow = false, int x = 0, int y = 0, int z = 0, int mass = 0, std::string shapeName = "Box");
    virtual ~GameObject (void);

    // Getters
    std::string getName(void) const;
    Ogre::Entity* getEntity(void) const;
    Ogre::SceneNode* getSceneNode(void) const;
    virtual int getMass(void) const;
    virtual std::string getShapeName(void) const;
    
    // Physics collaboration
    virtual void collidedWith(GameObject* other);
    virtual std::string getInternalType(void) const;
    
protected:
    std::string mName;
    Ogre::Entity* mEntity;
    Ogre::SceneNode* mNode;
    int mMass;
    std::string mShapeName;
    std::time_t mCollisionTimeStamp;
};

#endif // #ifndef __GameObject_h_
