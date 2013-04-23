/*
 * -----------------------------------------------------------------------------
 * Filename:    BaseApplication.cpp
 * -----------------------------------------------------------------------------
 * 
 * This source file is part of the
 *   ___                 __    __ _ _    _ 
 *  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 * //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
 * / \_// (_| | | |  __/  \  /\  /| |   <| |
 * \___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
 *      |___/                              
 *      Tutorial Framework
 *      http://www.ogre3d.org/tikiwiki/
 * -----------------------------------------------------------------------------
 */
#include "BaseApplication.h"

//-------------------------------------------------------------------------------------
BaseApplication::BaseApplication(void)
: mRoot(0),
mCamera(0),
mSceneMgr(0),
mWindow(0),
mResourcesCfg(Ogre::StringUtil::BLANK),
mPluginsCfg(Ogre::StringUtil::BLANK)
{
}

//-------------------------------------------------------------------------------------
BaseApplication::~BaseApplication(void)
{
    delete mRoot;
}

//-------------------------------------------------------------------------------------
bool BaseApplication::configure(void)
{
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "GalactiCombat");
        
        return true;
    }
    else
    {
        return false;
    }
}
//-------------------------------------------------------------------------------------
void BaseApplication::chooseSceneManager(void)
{
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
}
//-------------------------------------------------------------------------------------
void BaseApplication::setupResources(void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);
    
    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
    
    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}
//-------------------------------------------------------------------------------------
void BaseApplication::createResourceListener(void)
{
    
}
//-------------------------------------------------------------------------------------
void BaseApplication::loadResources(void)
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
//-------------------------------------------------------------------------------------
void BaseApplication::go(void)
{
    #ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
    #else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
    #endif
    
    if (!setup())
        return;
    
    mRoot->startRendering();
    
    // clean up
    destroyScene();
}
//-------------------------------------------------------------------------------------
bool BaseApplication::setup(void)
{
    mRoot = new Ogre::Root(mPluginsCfg);
    
    setupResources();
    
    bool carryOn = configure();
    if (!carryOn) return false;
    
    chooseSceneManager();
    createCamera();
    createViewports();
    
    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
    
    // Create any resource listeners (for loading screens)
    createResourceListener();
    // Load resources
    loadResources();
    
    // Create the scene
    createScene();
    
    createFrameListener();
    
    return true;
};
