#ifndef __SpaceShip_h_
#define __SpaceShip_h_

#include <OgreCamera.h>

#include "GameObject.h"
#include "ISpaceShipController.h"
#include "Mineral.h"

class SpaceShip : public GameObject {
    
public:
    SpaceShip (std::string name, ISpaceShipController* controller, Ogre::SceneNode* parentNode, Ogre::Entity* entity, int x = 100, int y = 100, int z = 100, double s = 30, Ogre::Camera* cam = 0);
    SpaceShip (std::string name, ISpaceShipController* controller, Ogre::SceneNode* parentNode, int x = 100, int y = 100, int z = 100, double s = 30, Ogre::Camera* cam = 0);
    virtual ~SpaceShip(void);
    
    // The controller is used to make decisions
    ISpaceShipController* getController(void) const;
    
    // SpaceShip has energy
    double getEnergy(void) const;
    void adjustEnergy(double);
    
    // SpaceShip size varies
    double getSize(void) const;
    double getSizeDifference(void) const;
    void adjustSize(double);
    
    // Phyics collaboration
    virtual int getMass(void) const;
    virtual void collidedWith(GameObject*);
    virtual std::string getInternalType(void) const;
    
    static const double ACCELERATION;
    static const double ENERGY_CONSUMPTION;
    static const double ENERGY_MINING;
    static const double STARTING_ENERGY;
    static const double MIN_ENERGY;
    static const double MAX_ENERGY;
    
protected:
    Ogre::Camera* camera;
    ISpaceShipController* brain;
    double size;
    double sizeDifference;
    double energy;
    
private:
    void setupSpaceShip(void);
    
};

#endif //#ifndef __SpaceShip_h_