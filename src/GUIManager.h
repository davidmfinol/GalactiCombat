#ifndef __GUIManager_h_
#define __GUIManager_h_

#include <string>
#include <sstream>
#include <map>

#include <Ogre.h>
#include <OISKeyboard.h>
#include <OISMouse.h>
#include <SdkTrays.h>

#include "SoundManager.h"
#include "NetworkManagerClient.h"

#define INJECT_CODE 2
#define OVER_CODE 3

class GUIManager : public OgreBites::SdkTrayListener {
    
public:
    GUIManager(double, double);
    virtual ~GUIManager(void);
    void GUIsetup(NetworkManagerClient*, SoundManager*, Ogre::RenderWindow*, OIS::Mouse*, OIS::Keyboard*);
    OgreBites::SdkTrayManager* getTrayMgr(void);
    
    // Intro 
    void displayWelcomeMsg(void);
    bool isWelcomeState(void);
    void passWelcomeState(void);
    void passWelcomeStateAfterFailure(bool);
    
    // Multi-Player Menu
    bool isInLobby(void);
    bool isMultiPlayerMenu(void);
    bool isConnectionFail(void);
    std::string getMultiplayerInput(void);
    void setMultiplayerInput(std::string);
    void lobby(void);
    void readyPressed(void);
    void setLobbyList(char*);
    bool lobbyCountingDown(void);
    void startCountingDown(void);
    void updateLobbyList(int);
    
    // In-game GUI
    void informEnergy(double);
    void setTimeLabel(std::string);
    void countDown(int, int);
    
    // In-game Menu
    void toggleMainMenu(void);
    bool isMainMenuOpened(void);
    void submit(void);
    
    // Game Over
    void gameOver(double);
    bool isGameOver(void);
    bool isShutDown(void);

    // Timer
    bool resetTimer(void) {return notifyResetTimer;}
    void resetTimerDone(void) {notifyResetTimer = false;}
    
protected:
    // OgreBites::SdkTrayListener
    virtual void buttonHit(OgreBites::Button*);
    virtual void sliderMoved(OgreBites::Slider*);
    virtual void okDialogClosed(const Ogre::DisplayString&);
    OgreBites::SdkTrayManager* mTrayMgr;
	OgreBites::InputContext mInputContext;
    
    // Menu items
    OgreBites::TextBox* welcomeBox;
    OgreBites::TextBox* gameoverBox;
    OgreBites::Label* menuLabel;
    OgreBites::Label* timerLabel;
    OgreBites::Label* countDownLabel;
    OgreBites::Label* multiLabel;
    OgreBites::Label* inputLabel;
    OgreBites::Button* startSingle;
    OgreBites::Button* startMulti;
    OgreBites::Button* submitButton;
    OgreBites::Button* musicButton;
    OgreBites::Button* quitButton;
    OgreBites::Button* gameoverQuitButton;
    OgreBites::Button* lobbyReadyButton;
    OgreBites::Button* lobbyQuitButton;
    OgreBites::Button* nextLevelButton;
    OgreBites::ProgressBar* energyIndicator;
    OgreBites::Slider* musicControl;
    OgreBites::Slider* soundControl;
    OgreBites::TextBox* lobbyPanel;
    
    // States
    bool initialState;
    bool multiPlayerMenu;
    bool finalState;
    bool connectionFail;
    bool mShutDown;
    bool notifyResetTimer;
    bool inLobby;
    bool countdown;
    bool isReady;
    
    // Game Components
    std::string userInputs;
    SoundManager* mSoundMgr;
    NetworkManagerClient* mNetworkMgr;
    double musicVol;
    double soundVol;

    // Spaceship stats
    double minEnergy;
    double maxEnergy;
};

#endif //#ifndef __GUIManager_h_
