#ifndef __Mineral_h_
#define __Mineral_h_

#include "GameObject.h"
#include "SoundManager.h"

class Mineral : public GameObject {
    
public:
    Mineral (std::string name, SoundManager* sound, Ogre::SceneNode* parentNode, Ogre::Entity* entity,
        int x = 100, int y = 100, int z = 100, double r = 1);
    Mineral (std::string name, SoundManager* sound, Ogre::SceneNode* parentNode,
        int x = 100, int y = 100, int z = 100, double r = 1);
    virtual ~Mineral(void);
    
    // Minerals have radiuses
    double getRadius(void) const;
    double getRadiusDifference(void) const;
    void adjustRadius(double);
    
    // Physics collaboration
    virtual int getMass(void) const;
    virtual void collidedWith(GameObject*);
    virtual std::string getInternalType(void) const;
    
    static const int MAX_RADIUS;
    static const int MIN_RADIUS;
    static const double RADIUS_INCREASE;
    static const double RADIUS_DECREASE;
    
protected:
    double radius;
    double radiusDifference;
    SoundManager* mSoundMgr;
    
private:
    void setupMineral(void);
};

#endif //#ifndef __Mineral_h_
