#ifndef __ClientSpaceShipController_h_
#define __ClientSpaceShipController_h_

#include "ISpaceShipController.h"

class ClientSpaceShipController : public ISpaceShipController {
    
public: 
    ClientSpaceShipController(void);
    virtual ~ClientSpaceShipController(void);
    
    //  The input char holds all the input sent by the client
    void injectInput(char input);
    
    // ISpaceShipController
    bool left() const;
    bool right() const;
    bool forward() const;
    bool back() const;
    bool up() const;
    bool down() const;
    bool shoot() const;
    
protected:
    char _input;
    
};

#endif //#ifndef __ClientSpaceShipController_h_