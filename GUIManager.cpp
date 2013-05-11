#include "GUIManager.h"

GUIManager::GUIManager(double min, double max) : initialState(true), finalState(false), multiPlayerMenu(false), mShutDown(false), inLobby(false), countdown(false),
isReady(false), minEnergy(min), maxEnergy(max), musicVol(0.5), soundVol(0.5), countDownLabel(0), mTrayMgr(0), menuLabel(0), musicButton(0), quitButton(0),
gameoverBox(0), timerLabel(0), gameoverQuitButton(0), energyIndicator(0), welcomeBox(0), lobbyReadyButton(0), lobbyQuitButton(0),
startSingle(0), startMulti(0), inputLabel(0), multiLabel(0), musicControl(0), soundControl(0), lobbyPanel(0)
{
    userInputs = "";
}

GUIManager::~GUIManager(void)
{
	//FIXME: Segfault
    if(welcomeBox) delete welcomeBox;
    if(gameoverBox) delete gameoverBox;
    if(menuLabel) delete menuLabel;
    if(timerLabel) delete timerLabel;
    if(countDownLabel) delete countDownLabel;
    if(multiLabel) delete multiLabel;
    if(inputLabel) delete inputLabel;
    if(startSingle) delete startSingle;
    if(startMulti) delete startMulti;
    if(submitButton) delete submitButton;
    if(musicButton) delete musicButton;
    if(quitButton) delete quitButton;
    if(gameoverQuitButton) delete gameoverQuitButton;
    if(energyIndicator) delete energyIndicator;
    if(musicControl) delete musicControl;
    if(soundControl) delete soundControl;
    if(lobbyPanel) delete lobbyPanel;
    if(lobbyReadyButton) delete lobbyReadyButton;
    if(lobbyQuitButton) delete lobbyQuitButton;
    if(mTrayMgr) delete mTrayMgr;
    
}

void GUIManager::GUIsetup(NetworkManagerClient* network, SoundManager* sound, Ogre::RenderWindow* window, OIS::Mouse* mouse)
{
    // Create the tray manager
    mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", window, mouse, this);
    // load the fonts
    Ogre::FontManager::getSingleton().getByName("SdkTrays/Caption")->load();
    Ogre::FontManager::getSingleton().getByName("SdkTrays/Value")->load();
    // create some essential GUI components
    timerLabel = mTrayMgr->createLabel(OgreBites::TL_TOP, "timer_label", "0:00", 200);
    energyIndicator = mTrayMgr->createProgressBar(OgreBites::TL_BOTTOM,  "energy_indicator", "Energy", 500, 300);
    // set managers
    mSoundMgr = sound;
    mNetworkMgr = network;
}

OgreBites::SdkTrayManager* GUIManager::getTrayMgr(void)
{
    return mTrayMgr;
}

void GUIManager::displayWelcomeMsg(void)
{
    std::string msg = "Welcome to GalactiCombat! In this game, you are controlling a ";
    msg += "spaceship. You can accelerate your ship in 3D space with WASD, left-shift, and space, and ";
    msg += " you rotate the camera with your mouse. The goal of this game is to absorb ";
    msg += "mass by shooting other ships, and eventually be the biggest ship in subspace.";
    msg += "Plan your movements carefully and shots carefully, because both moving and shooting will consume ";
    msg += "\"energy\", and you need energy to do those actions.\n\n\nQuick Notes:\n\n";
    msg += "1) Collide into minerals smaller than you to gain energy.\n\n"; 
    msg += "2). Yellow minerals are the same size as you, red ones are larger, and blue ones ";
    msg += "are smaller.\n\n";
    msg += "3). Getting shot by a bullet OR colliding into colliding into object bigger than you";
    msg += " will cause you to decrease in size.\n\n";
    welcomeBox = mTrayMgr->createTextBox(OgreBites::TL_CENTER, "welcome_widget", "Welcome!", 400, 300);
    welcomeBox->setText(msg);
    startSingle = mTrayMgr->createButton(OgreBites::TL_CENTER, "start_single", "Start Single-Player Game", 300);
    startMulti = mTrayMgr->createButton(OgreBites::TL_CENTER, "start_multi", "Start Multi-Player Game", 300);
}

bool GUIManager::isWelcomeState(void)
{
    return initialState;
}

void GUIManager::passWelcomeState(void)
{
    mTrayMgr->destroyWidget(welcomeBox); welcomeBox = 0;
    mTrayMgr->destroyWidget(startSingle); startSingle = 0;
    mTrayMgr->destroyWidget(startMulti); startMulti = 0;
    mTrayMgr->hideCursor();
    initialState = false;
}

bool GUIManager::isMultiPlayerMenu(void)
{
    return multiPlayerMenu;
}

bool GUIManager::lobbyCountingDown(void)
{
    return countdown;
}

void GUIManager::setMultiplayerInput(std::string s)
{
    inputLabel->setCaption(s);
    userInputs = inputLabel->getCaption();
}

std::string GUIManager::getMultiplayerInput()
{
    return userInputs;
}

void GUIManager::informEnergy(double energy)
{
    std::ostringstream o;
    o << std::setprecision(4) << energy;
    energyIndicator->setProgress((energy - minEnergy) / (maxEnergy - minEnergy));
    energyIndicator->setComment(o.str());
}

void GUIManager::setTimeLabel(std::string timeString)
{
    timerLabel->setCaption(timeString);
}

void GUIManager::countDown(int sec, int type) {
    std::ostringstream o;
    if (type == OVER_CODE)
        o << "Game is ending in " << sec << " seconds!";
    else
        o << "Be careful! The Crazy Energy Injection is coming in " << sec << " seconds!";
    if (!countDownLabel)
        countDownLabel = mTrayMgr->createLabel(OgreBites::TL_CENTER, "count_down_label", o.str(), 800);
    else 
        countDownLabel->setCaption(o.str());
    if (sec == 0 && countDownLabel != 0) {
        mTrayMgr->destroyWidget(countDownLabel); countDownLabel = 0;
    }
}

void GUIManager::toggleMainMenu(void)
{
    if (finalState) return;
    if (isMainMenuOpened()) // Close the menu
    {
        mTrayMgr->destroyWidget(menuLabel); menuLabel = 0;
        mTrayMgr->destroyWidget(quitButton); quitButton = 0;
        mTrayMgr->destroyWidget(musicButton); musicButton = 0;
        mTrayMgr->destroyWidget(musicControl); musicControl = 0;
        mTrayMgr->destroyWidget(soundControl); soundControl = 0;
        mTrayMgr->hideCursor();
    }
    else // Open the menu
    {
        menuLabel = mTrayMgr->createLabel(OgreBites::TL_CENTER, "menu_label", "MENU", 400);
        musicButton = mTrayMgr->createButton(OgreBites::TL_CENTER, "music_button", "Turn On/Off Music", 400);
        musicControl = mTrayMgr->createLongSlider(OgreBites::TL_CENTER, "music_control", "Music Volume", 400, 160, 90, 0, 1, 100);
        soundControl = mTrayMgr->createLongSlider(OgreBites::TL_CENTER, "sound_control", "Sound Volume", 400, 160, 90, 0, 1, 100);
        musicControl->setValue(musicVol);
        soundControl->setValue(soundVol);
        //mSoundMgr->adjustVolume(0, musicVol);
        //mSoundMgr->adjustVolume(1, soundVol);
        quitButton = mTrayMgr->createButton(OgreBites::TL_CENTER, "quit_button", "Quit Game", 400);
        mTrayMgr->showCursor();
    }
}

bool GUIManager::isMainMenuOpened(void)
{
    if (!menuLabel) return false;
    return true;
}

void GUIManager::gameOver(double size) {
	std::cout<<"Entering gameOver."<<std::endl;
    if (isMainMenuOpened()) // Close the menu
    {
        mTrayMgr->destroyWidget(menuLabel); menuLabel = 0;
        mTrayMgr->destroyWidget(quitButton); quitButton = 0;
        mTrayMgr->destroyWidget(musicButton); musicButton = 0;
        mTrayMgr->destroyWidget(musicControl); musicControl = 0;
        mTrayMgr->destroyWidget(soundControl); soundControl = 0;
    }
    mTrayMgr->showCursor();
    
    if (mNetworkMgr->isOnline()) {
        countdown = false;
        inLobby = true;
        initialState = true;    
        multiPlayerMenu = true;
        isReady = false;
        notifyResetTimer = true;
        mNetworkMgr->resetReadyState();
        lobbyPanel = mTrayMgr->createTextBox(OgreBites::TL_CENTER, "lobby_panel", "Game Lobby", 500, 300);
        lobbyReadyButton = mTrayMgr->createButton(OgreBites::TL_CENTER, "lobby_ready_button", "Ready", 200);
        lobbyQuitButton = mTrayMgr->createButton(OgreBites::TL_CENTER, "lobby_quit_button", "Quit", 200);
		/*
		///////////////////////// Add the online scores
        std::multimap<double, std::string, std::greater<double> > topScores;
        std::map<std::string, double> playerScores;
        mNetworkMgr->sendPlayerScore(size);
        std::string scores = mNetworkMgr->getPlayerScores();
		std::stringstream o;
		std::cout<<"Starting while loop."<<std::endl;
        while(scores != "") {
			std::cout<<"Inside while loop."<<std::endl;
            size_t pos = scores.find(",");
            if(pos == -1) 
                break;
            std::string name = scores.substr(0, pos);
            scores = scores.substr(pos + 1); 
            pos = scores.find(";");
            if(pos == -1) 
                break;
            double score = atof(scores.substr(0, pos).c_str());
            topScores.insert(std::pair<double, std::string>(score, name));
            if(score > playerScores[name])
                playerScores[name] = score;
            scores = scores.substr(pos + 1); 
        	o << "Scoreboard:\n";
        //for(std::map<std::string, double>::iterator it = playerScores.begin(); it != playerScores.end(); ++it) 
        //    o << (it->first) << ": " << (it->second) << "\n";
        	for(std::multimap<double, std::string>::iterator it = topScores.begin(); it != topScores.end(); ++it) 
            	o << (it->first) << ": " << (it->second) << "\n";
    	}
		std::cout<<"Ending while loop."<<std::endl;
        mTrayMgr->showOkDialog("Scoreboard", o.str());
		/////////////////////////////////////////////////////////////
		*/
    }
    else {
        finalState = true;
        gameoverBox = mTrayMgr->createTextBox(OgreBites::TL_CENTER, "gameover_box", "Final Summary", 400, 300);
        std::ostringstream o;
        o << std::fixed << std::setprecision(2);
        o << "Well done\n\nYour final size is: " << size << "\n\nHope you had fun!\n\n\n";
        nextLevelButton = mTrayMgr->createButton(OgreBites::TL_CENTER, "next_level_button", "Restart", 200);
        gameoverBox->setText(o.str());
        gameoverQuitButton = mTrayMgr->createButton(OgreBites::TL_CENTER, "gameover_quit_button", "Quit Game", 200);
    }
	std::cout<<"Exiting gameOver."<<std::endl;
}

bool GUIManager::isGameOver(void)
{
    return finalState;
}

bool GUIManager::isConnectionFail(void)
{
    return connectionFail;
}

bool GUIManager::isShutDown(void)
{
    return mShutDown;
}

bool GUIManager::isInLobby(void) 
{
    return inLobby;
}

void GUIManager::passWelcomeStateAfterFailure(bool fromEsc)
{
    initialState = false;    
    multiPlayerMenu = false;
    mTrayMgr->hideCursor();
    if (fromEsc) mTrayMgr->closeDialog();
}

void GUIManager::okDialogClosed(const Ogre::DisplayString& message)
{    
    if (message.substr(0, 10) == "Connection") {
        passWelcomeStateAfterFailure(false);
    }
    else if (message.substr(0, 10) == "Scoreboard") {
    
    }
}

void GUIManager::lobby(void)
{
    lobbyPanel = mTrayMgr->createTextBox(OgreBites::TL_CENTER, "lobby_panel", "Game Lobby", 500, 300);
    lobbyReadyButton = mTrayMgr->createButton(OgreBites::TL_CENTER, "lobby_ready_button", "Ready", 200);
    lobbyQuitButton = mTrayMgr->createButton(OgreBites::TL_CENTER, "lobby_quit_button", "Quit", 200);
    inLobby = true;
}

void GUIManager::updateLobbyList(int i) 
{
    if (!i) {
        mTrayMgr->destroyWidget(lobbyPanel); lobbyPanel = 0;
        inLobby = false;
        initialState = false;    
        multiPlayerMenu = false;
        mTrayMgr->hideCursor();   
        char* request = const_cast<char*>("15START");
        NetworkUtil::TCPSend(mNetworkMgr->getSocket(), request);
    }
    else {
        std::stringstream ss;
        ss << lobbyPanel->getText() << "\n" << i << ".................";
        lobbyPanel->setText(ss.str());
    }
}


void GUIManager::startCountingDown(void)
{
    mTrayMgr->destroyWidget(lobbyReadyButton); lobbyReadyButton = 0;
    mTrayMgr->destroyWidget(lobbyQuitButton); lobbyQuitButton = 0; 
    countdown = true;
}

void GUIManager::readyPressed(void)
{
    if (!isReady) {
        char* request = const_cast<char*>("15READY");
        NetworkUtil::TCPSend(mNetworkMgr->getSocket(), request);
        lobbyReadyButton->setCaption("Cancel");
    }
    else {
        char* request = const_cast<char*>("15PREPARING");
        NetworkUtil::TCPSend(mNetworkMgr->getSocket(), request);
        lobbyReadyButton->setCaption("Ready");        
    }
    isReady = !isReady;
}

void GUIManager::setLobbyList(char* content)
{
    lobbyPanel->setText(content);
}

void GUIManager::submit(void)
{
    mTrayMgr->destroyWidget(multiLabel); multiLabel = 0;
    mTrayMgr->destroyWidget(inputLabel); inputLabel = 0;
    mTrayMgr->destroyWidget(submitButton); submitButton = 0;
    
    std::string host = userInputs.substr(0, userInputs.find(",", 0));
    std::string clientName = userInputs.substr(userInputs.find(",", 0) + 1);
//    int port = 5172;
    try {
        mNetworkMgr->connect(const_cast<char*>(host.c_str()), const_cast<char*>(clientName.c_str()));
        lobby();
    }
    catch (std::string exception) {
        if (exception == "fail_to_connect") {
            std::string msg = "Connection to the server has not been established successfully. ";
            msg += "Please verify that you have run ./GalactiCombatServer on the server. ";
            msg += "And verify that you did not mistype anything.\n\n";
            msg += "Clicking 'OK' now will start the single-player game.";
            mTrayMgr->showOkDialog("Fail to connect to the server", msg);
            connectionFail = true;
        }
    }
}

void GUIManager::buttonHit(OgreBites::Button *b)
{  
    static bool soundEffect = true;  
    std::string buttonName = b->getName();
    if (buttonName == "quit_button" || buttonName == "gameover_quit_button" || buttonName == "lobby_quit_button")
    {
        mShutDown = true;
        if(mNetworkMgr->isOnline())
        {
            mNetworkMgr->quit();
        }
    }
    else if (buttonName == "lobby_ready_button")
    {
        readyPressed();
    }
    else if (buttonName == "start_single")
    {
        passWelcomeState();
    } 
    else if (buttonName == "music_button")
    {
        SDL_PauseAudio(soundEffect);
        soundEffect = !soundEffect;
    }
    else if (buttonName == "next_level_button")
    {
        initialState = true;
        finalState = false;
        mTrayMgr->destroyWidget(gameoverQuitButton); gameoverQuitButton = 0;
        mTrayMgr->destroyWidget(nextLevelButton); nextLevelButton = 0;
        mTrayMgr->destroyWidget(gameoverBox); gameoverBox = 0;
        notifyResetTimer = true;
        displayWelcomeMsg();
    } 
    else if (buttonName == "start_multi")
    {
        mTrayMgr->destroyWidget(welcomeBox); welcomeBox = 0;
        mTrayMgr->destroyWidget(startSingle); startSingle = 0;
        mTrayMgr->destroyWidget(startMulti); startMulti = 0;
        multiLabel = mTrayMgr->createLabel(OgreBites::TL_CENTER, "multi_label", "Please type <hostname,clientName>", 800);
        inputLabel = mTrayMgr->createLabel(OgreBites::TL_CENTER, "input_label", "", 800);
        submitButton = mTrayMgr->createButton(OgreBites::TL_CENTER, "submit_button", "Start", 400);
        multiPlayerMenu = true;
    } 
    else if (buttonName == "submit_button")
    {
        submit();
    }
}

void GUIManager::sliderMoved(OgreBites::Slider *s)
{  
    std::string sliderName = s->getName();
    if (sliderName == "music_control") {
        musicVol = s->getValue();
        mSoundMgr->adjustVolume(0, musicVol);
    }
    else if (sliderName == "sound_control") {
        soundVol = s->getValue();
        mSoundMgr->adjustVolume(1, soundVol);
    }
}
