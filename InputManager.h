#ifndef __InputManager_h_
#define __InputManager_h_

#include <sstream>
#include <OgreRenderWindow.h>
#include <OgreWindowEventUtilities.h>
#include <OgreSceneManager.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include "ISpaceShipController.h"
#include "GUIManager.h"

class InputManager : public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener, public ISpaceShipController {

public:
    InputManager(NetworkManagerClient*);
    virtual ~InputManager(void);
    void inputSetup(Ogre::RenderWindow*, GUIManager*);
    void setPlayerCamera(Ogre::SceneNode*, Ogre::SceneNode*);
    
    // Constants
    static const Ogre::Real CAMERA_YAW;
    static const Ogre::Real CAMERA_PITCH;
    
    // Getters
    OIS::Mouse* getMouse() const;
    OIS::Keyboard* getKeyboard() const;
    
    // ISpaceShipController
    bool left() const;
    bool right() const;
    bool forward() const;
    bool back() const;
    bool up() const;
    bool down() const;
    bool shoot() const;

protected:
    // Ogre::WindowEventListener
    // Adjust mouse clipping area
    virtual void windowResized(Ogre::RenderWindow*);
    // Unattach OIS before window shutdown (very important under Linux)
    virtual void windowClosed(Ogre::RenderWindow*);
    
    // OIS::KeyListener
    virtual bool keyPressed(const OIS::KeyEvent&);
    virtual bool keyReleased(const OIS::KeyEvent&);
    // OIS::MouseListener
    virtual bool mouseMoved(const OIS::MouseEvent&);
    virtual bool mousePressed(const OIS::MouseEvent&, OIS::MouseButtonID);
    virtual bool mouseReleased(const OIS::MouseEvent&, OIS::MouseButtonID);
    
    // Camera that this input manager rotates
    Ogre::SceneNode* mCameraHorizontal;
    Ogre::SceneNode* mCameraVertical;
    // Components that need to receive input
    Ogre::RenderWindow* mWindow;
    GUIManager* mGUIMgr;
    NetworkManagerClient* mNetworkMgr;
    // OIS Input devices
    OIS::InputManager* mInputManager;
    OIS::Mouse*    mMouse;
    OIS::Keyboard* mKeyboard;
    
    // Relevant input
    bool mLeft;
    bool mRight;
    bool mForward;
    bool mBack;
    bool mUp;
    bool mDown;
    bool mShoot;
    std::map<OIS::KeyCode, std::string> keyCodesToCharacters;
};
#endif //#ifndef __InputManager_h_
