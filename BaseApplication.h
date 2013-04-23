/*
 * -----------------------------------------------------------------------------
 * Filename:    BaseApplication.h
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
#ifndef __BaseApplication_h_
#define __BaseApplication_h_

#include <Ogre.h>

class BaseApplication {

public:
    BaseApplication(void);
    virtual ~BaseApplication(void);
    virtual void go(void);
  
protected:
    virtual bool setup();
    virtual bool configure(void);
    virtual void chooseSceneManager(void);
    virtual void createCamera(void) = 0;
    virtual void createFrameListener(void) = 0;
    virtual void createScene(void) = 0; // Override me!
    virtual void destroyScene(void) = 0;
    virtual void createViewports(void) = 0;
    virtual void setupResources(void);
    virtual void createResourceListener(void);
    virtual void loadResources(void);
    
    
    Ogre::Root *mRoot;
    Ogre::Camera* mCamera;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;
};

#endif // #ifndef __BaseApplication_h_
