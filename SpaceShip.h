#ifndef __SpaceShip_h_
#define __SpaceShip_h_

#include <OgreCamera.h>
#include <OgreMath.h>

#include "GameObject.h"
#include "ISpaceShipController.h"
#include "Mineral.h"

class SpaceShip : public GameObject {
    
public:
    SpaceShip (std::string name, ISpaceShipController* controller, Ogre::SceneNode* parentNode, Ogre::Entity* entity, int x = 100, int y = 100, int z = 100, double s = 300);
    SpaceShip (std::string name, ISpaceShipController* controller, Ogre::SceneNode* parentNode, int x = 100, int y = 100, int z = 100, double s = 300);
    virtual ~SpaceShip(void);
    
    // Some helper functions
    void reset(void);
    void attachCamera(Ogre::Camera* camera);
    
    // The controller is used to make decisions
    ISpaceShipController* getController(void) const;
    
    // SpaceShip has energy
    double getEnergy(void) const;
    void adjustEnergy(double e);
    void setEnergy(double e);
    
    // SpaceShip size varies
    double getSize(void) const;
    double getSizeDifference(void) const;
    void adjustSize(double s);
    
    // Phyics collaboration
    virtual int getMass(void) const;
    virtual void collidedWith(GameObject* other);
    virtual std::string getInternalType(void) const;

    // Bullet control
    bool canShoot(void);
    void shootBullet(void);
    
    // Constants
    static const double ACCELERATION;
    static const double ENERGY_CONSUMPTION;
    static const double BULLET_COST;
    static const double ENERGY_GAIN;
    static const double SIZE_GAIN;
    static const double SIZE_LOSS;
    static const double RADIUS_LOSS;
    static const double STARTING_ENERGY;
    static const double MIN_ENERGY;
    static const double MAX_ENERGY;
    static const int MAX_SIZE;
    static const int MIN_SIZE;
    
protected:
    ISpaceShipController* brain;
    double size;
    double sizeDifference;
    double energy;
    std::time_t lastShotTime;
    
private:
    void setupSpaceShip(void);
    
};

#endif //#ifndef __SpaceShip_h_
