#ifndef __ISpaceShipController_h_
#define __ISpaceShipController_h_

class ISpaceShipController {
    
public: 
    virtual bool left() const = 0;
    virtual bool right() const = 0;
    virtual bool forward() const = 0;
    virtual bool back() const = 0;
    virtual bool up() const = 0;
    virtual bool down() const = 0;
    virtual bool shoot() const = 0;
    
};

#endif //#ifndef __ISpaceShipController_h_