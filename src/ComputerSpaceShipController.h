#ifndef __ComputerSpaceShipController_h_
#define __ComputerSpaceShipController_h_

#include <OgreSceneManager.h>
#include "ISpaceShipController.h"

class ComputerSpaceShipController : public ISpaceShipController {
    
public: 
    //ComputerSpaceShipController(Ogre::SceneManager* sceneMgr);
    //virtual ~ComputerSpaceShipController(void);
    
    // ISpaceShipController
    bool left() const;
    bool right() const;
    bool forward() const;
    bool back() const;
    bool up() const;
    bool down() const;
    bool shoot() const;
    
protected:
    Ogre::SceneManager* scene; //TODO: MAYBE USE GAMEOBJECTS_LIST (& physicSimulator) INSTEAD OF SCENE TO IDENTIFY SURROUNDINGS
    
};

#endif //#ifndef __ComputerSpaceShipController_h_