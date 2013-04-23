#include "InputManager.h"

const Ogre::Real InputManager::CAMERA_YAW = -0.15;
const Ogre::Real InputManager::CAMERA_PITCH = -0.10;

//-------------------------------------------------------------------------------------
InputManager::InputManager(NetworkManagerClient* network): mNetworkMgr(network), mWindow(0), mGUIMgr(0), mInputManager(0), mMouse(0), mKeyboard(0), 
mLeft(false), mRight(false), mForward(false), mBack(false), mUp(false), mDown(false), mShoot(false),
mCameraHorizontal(0), mCameraVertical(0)
{
	//TODO: WAT
    keyCodesToCharacters[OIS::KC_Q] = "Q";
    keyCodesToCharacters[OIS::KC_W] = "W";
    keyCodesToCharacters[OIS::KC_E] = "E";
    keyCodesToCharacters[OIS::KC_R] = "R";
    keyCodesToCharacters[OIS::KC_T] = "T";
    keyCodesToCharacters[OIS::KC_Y] = "Y";
    keyCodesToCharacters[OIS::KC_U] = "U";
    keyCodesToCharacters[OIS::KC_I] = "I";
    keyCodesToCharacters[OIS::KC_O] = "O";
    keyCodesToCharacters[OIS::KC_P] = "P";
    keyCodesToCharacters[OIS::KC_A] = "A";
    keyCodesToCharacters[OIS::KC_S] = "S";
    keyCodesToCharacters[OIS::KC_D] = "D";
    keyCodesToCharacters[OIS::KC_F] = "F";
    keyCodesToCharacters[OIS::KC_G] = "G";
    keyCodesToCharacters[OIS::KC_H] = "H";
    keyCodesToCharacters[OIS::KC_I] = "I";
    keyCodesToCharacters[OIS::KC_J] = "J";
    keyCodesToCharacters[OIS::KC_K] = "K";
    keyCodesToCharacters[OIS::KC_L] = "L";
    keyCodesToCharacters[OIS::KC_Z] = "Z";
    keyCodesToCharacters[OIS::KC_X] = "X";
    keyCodesToCharacters[OIS::KC_C] = "C";
    keyCodesToCharacters[OIS::KC_V] = "V";
    keyCodesToCharacters[OIS::KC_B] = "B";
    keyCodesToCharacters[OIS::KC_N] = "N";
    keyCodesToCharacters[OIS::KC_M] = "M";
    keyCodesToCharacters[OIS::KC_1] = "1";
    keyCodesToCharacters[OIS::KC_2] = "2";
    keyCodesToCharacters[OIS::KC_3] = "3";
    keyCodesToCharacters[OIS::KC_4] = "4";
    keyCodesToCharacters[OIS::KC_5] = "5";
    keyCodesToCharacters[OIS::KC_6] = "6";
    keyCodesToCharacters[OIS::KC_7] = "7";
    keyCodesToCharacters[OIS::KC_8] = "8";
    keyCodesToCharacters[OIS::KC_9] = "9";
    keyCodesToCharacters[OIS::KC_0] = "0";
    keyCodesToCharacters[OIS::KC_PERIOD] = ".";
    keyCodesToCharacters[OIS::KC_COMMA] = ",";
    keyCodesToCharacters[OIS::KC_MINUS] = "-";
    keyCodesToCharacters[OIS::KC_NUMPAD1] = "1";
    keyCodesToCharacters[OIS::KC_NUMPAD2] = "2";
    keyCodesToCharacters[OIS::KC_NUMPAD3] = "3";
    keyCodesToCharacters[OIS::KC_NUMPAD4] = "4";
    keyCodesToCharacters[OIS::KC_NUMPAD5] = "5";
    keyCodesToCharacters[OIS::KC_NUMPAD6] = "6";
    keyCodesToCharacters[OIS::KC_NUMPAD7] = "7";
    keyCodesToCharacters[OIS::KC_NUMPAD8] = "8";
    keyCodesToCharacters[OIS::KC_NUMPAD9] = "9";
    keyCodesToCharacters[OIS::KC_NUMPAD0] = "0";
    keyCodesToCharacters[OIS::KC_SUBTRACT] = "-";
    keyCodesToCharacters[OIS::KC_DECIMAL] = ".";
}
//-------------------------------------------------------------------------------------
InputManager::~InputManager(void)
{
    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    //TODO: delete mInputManager, mKeyboard, mMouse?
}
//-------------------------------------------------------------------------------------
void InputManager::inputSetup(Ogre::RenderWindow* window, GUIManager* GUIMgr)
{
    mWindow = window;
    mGUIMgr = GUIMgr;
    
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;
    
    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
    
    mInputManager = OIS::InputManager::createInputSystem( pl );
    
    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));
    
    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);
    
    //Set initial mouse clipping size
    windowResized(mWindow);
    
    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);
}  
//-------------------------------------------------------------------------------------
void InputManager::setPlayerCamera(Ogre::SceneNode* horizontal, Ogre::SceneNode* vertical)
{
    mCameraHorizontal = horizontal;
    mCameraVertical = vertical;
}
//-------------------------------------------------------------------------------------
bool InputManager::left() const
{
    return mLeft;
}
//-------------------------------------------------------------------------------------
bool InputManager::right() const
{
    return mRight;
}
//-------------------------------------------------------------------------------------
bool InputManager::forward() const
{
    return mForward;
}
//-------------------------------------------------------------------------------------
bool InputManager::back() const
{
    return mBack;
}
//-------------------------------------------------------------------------------------
bool InputManager::up() const
{
    return mUp;
}
//-------------------------------------------------------------------------------------
bool InputManager::down() const
{
    return mDown;
}
//-------------------------------------------------------------------------------------
bool InputManager::shoot() const
{
    return mShoot;
}
//-------------------------------------------------------------------------------------
OIS::Mouse* InputManager::getMouse() const
{
    return mMouse;
}
//-------------------------------------------------------------------------------------
OIS::Keyboard* InputManager::getKeyboard() const
{
    return mKeyboard;
}
//-------------------------------------------------------------------------------------
// Ogre::WindowEventListener
// Adjust mouse clipping area
void InputManager::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}
//-------------------------------------------------------------------------------------
// Ogre::WindowEventListener
// Unattach OIS before window shutdown (very important under Linux)
void InputManager::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}
//-------------------------------------------------------------------------------------
// OIS::KeyListener
bool InputManager::keyPressed(const OIS::KeyEvent& arg)
{
    if (mGUIMgr->isInLobby()) return true;
    switch (arg.key)
    {
        case OIS::KC_ESCAPE: 
            // Toggles the in-game menu
            if (!mGUIMgr->isWelcomeState())
                mGUIMgr->toggleMainMenu();
            // Esc pressed before game started
            else {
                if (mGUIMgr->isMultiPlayerMenu())
                    if (!mGUIMgr->isConnectionFail())
                        mGUIMgr->setMultiplayerInput("");
                    else
                        mGUIMgr->passWelcomeStateAfterFailure(true);
                else
                    mGUIMgr->passWelcomeState();
            }
            break;
        case OIS::KC_W: 
            mForward = true; break;
        case OIS::KC_A: 
            mLeft = true; break;
        case OIS::KC_S: 
            mBack = true; break;
        case OIS::KC_D: 
            mRight = true; break;
        case OIS::KC_LSHIFT: 
            mDown = true; break;
        case OIS::KC_SPACE: 
            mUp = true; break;
        default:
            break;
    }
    
    if(mNetworkMgr->isOnline())
        mNetworkMgr->sendPlayerInput(this);
    
    return true;
}
//-------------------------------------------------------------------------------------
// OIS::KeyListener
bool InputManager::keyReleased( const OIS::KeyEvent &arg )
{
    if (mGUIMgr->isInLobby()) return true;
    switch (arg.key)
    {
        case OIS::KC_W: 
            mForward = false; break;
        case OIS::KC_A: 
            mLeft = false; break;
        case OIS::KC_S: 
            mBack = false; break;
        case OIS::KC_D: 
            mRight = false; break;
        case OIS::KC_LSHIFT: 
            mDown = false; break;
        case OIS::KC_SPACE: 
            mUp = false; break;
        default:
            break;
    }
    
    if(mGUIMgr->isMultiPlayerMenu()) {
        if (arg.key == OIS::KC_RETURN) {
            if (!mGUIMgr->isConnectionFail()) {
                mGUIMgr->submit();
                return true;
            }
            else {
                mGUIMgr->passWelcomeStateAfterFailure(true);
                return true;
            }
        }
        std::string newString = mGUIMgr->getMultiplayerInput();
        if(arg.key == OIS::KC_BACK) {
            newString = newString.substr(0, newString.size() - 1);
        }
        else {
            std::ostringstream o;
            o << newString << keyCodesToCharacters[arg.key];
            newString = o.str();
        }
        mGUIMgr->setMultiplayerInput(newString);
    }
    
    if(mNetworkMgr->isOnline())
        mNetworkMgr->sendPlayerInput(this);
    
    return true;
}
//-------------------------------------------------------------------------------------
// OIS::MouseListener
bool InputManager::mouseMoved( const OIS::MouseEvent &arg )
{
    mGUIMgr->getTrayMgr()->injectMouseMove(arg);
    
    if (!(mGUIMgr->isMainMenuOpened() || mGUIMgr->isWelcomeState() || mGUIMgr->isGameOver()))
    {
        if(mCameraHorizontal) 
            mCameraHorizontal->yaw(Ogre::Degree(CAMERA_YAW * arg.state.X.rel));
        if(mCameraVertical) 
            mCameraVertical->pitch(Ogre::Degree(CAMERA_PITCH * arg.state.Y.rel), Ogre::Node::TS_LOCAL);
    }
    
    return true;
}
//-------------------------------------------------------------------------------------
// OIS::MouseListener
bool InputManager::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    mShoot = true;
    mGUIMgr->getTrayMgr()->injectMouseDown(arg, id);
    switch (id)
    {
        case OIS::MB_Left: 
            mShoot = true; break;
        default:
            break;
    }
    
    if(mNetworkMgr->isOnline())
        mNetworkMgr->sendPlayerInput(this);
    
    return true;
}
//-------------------------------------------------------------------------------------
// OIS::MouseListener
bool InputManager::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    mShoot = false;
    mGUIMgr->getTrayMgr()->injectMouseUp(arg, id);
    switch (id)
    {
        case OIS::MB_Left: 
            mShoot = false; break;
        default:
            break;
    }
    
    if(mNetworkMgr->isOnline())
        mNetworkMgr->sendPlayerInput(this);
    
    return true;
}
