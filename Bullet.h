#ifndef __Bullet_h_
#define __Bullet_h_

#include "GameObject.h"
#include "SpaceShip.h"

class Bullet : public GameObject {
    
public:
    Bullet (std::string name, Ogre::SceneNode* parentNode, Ogre::Entity* entity, SpaceShip* owner, int x, int y, int z);
    Bullet (std::string name, Ogre::SceneNode* parentNode, SpaceShip* owner, int x, int y, int z);
    virtual ~Bullet(void);        
    virtual void collidedWith(GameObject*);
    virtual std::string getInternalType(void) const;
    bool hasHit() const;
    SpaceShip* getOwner() { return _owner; }
    
    // Constants
    static const float LOSS;
    static const float GAIN;
    
protected:
    bool _hasHit;
    SpaceShip* _owner;
};

#endif //#ifndef __Bullet_h_
