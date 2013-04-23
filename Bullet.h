#ifndef __Bullet_h_
#define __Bullet_h_

#include "GameObject.h"

class Bullet : public GameObject {
    
public:
    Bullet (std::string name, Ogre::SceneNode* parentNode, int x, int y, int z);
    virtual ~Bullet(void);        
    virtual void collidedWith(GameObject*);
    virtual std::string getInternalType(void) const;
};

#endif //#ifndef __Bullet_h_
