#ifndef __OgreMotionState_h_
#define __OgreMotionState_h_

#include <OgreSceneManager.h>
#include <OgreVector3.h>
#include "btBulletDynamicsCommon.h"

class OgreMotionState : public btMotionState {
    
public:
    OgreMotionState(const btTransform& initialpos, Ogre::SceneNode* visibleObj, bool allowRotation = true);
    virtual ~OgreMotionState(void);
    virtual void getWorldTransform(btTransform& worldTrans) const;
    virtual void setWorldTransform(const btTransform& worldTrans);
    void setPosition(const btVector3& newPos);
    void setOrientation(const btQuaternion& newRot);
    bool allowsRotation(void) const;
    
protected:
    btTransform mPhysicalPos;
    Ogre::SceneNode* mVisibleObj;
    bool mAllowRotation;
};

#endif // #ifndef __OgreMotionState_h_
