noinst_HEADERS = BaseApplication.h GalactiCombat.h GameObject.h OgreMotionState.h PhysicsSimulator.h GUIManager.h GalactiCombatServer.h NetworkManagerClient.h InputManager.h SoundManager.h NetworkUtil.h Bullet.h SpaceShip.h ISpaceShipController.h ComputerSpaceShipController.h Mineral.h ClientSpaceShipController.h


bin_PROGRAMS = GalactiCombat GalactiCombatServer

GalactiCombat_CPPFLAGS = -I$(top_srcdir)
GalactiCombat_SOURCES = BaseApplication.cpp GalactiCombat.cpp GameObject.cpp OgreMotionState.cpp PhysicsSimulator.cpp GUIManager.cpp NetworkManagerClient.cpp InputManager.cpp SoundManager.cpp GalactiCombatMain.cpp Bullet.cpp SpaceShip.cpp ComputerSpaceShipController.cpp Mineral.cpp ClientSpaceShipController.cpp
GalactiCombat_CXXFLAGS = $(OGRE_CFLAGS) $(OIS_CFLAGS) $(bullet_CFLAGS) $(sdl_CFLAGS) $(CEGUI_CFLAGS) $(CEGUI_OGRE_CFLAGS) $(SDL_net_CFLAGS)
GalactiCombat_LDADD = $(OGRE_LIBS) $(OIS_LIBS) $(bullet_LIBS) $(sdl_LIBS) $(CEGUI_LIBS) $(CEGUI_OGRE_LIBS) $(SDL_net_LIBS)

GalactiCombatServer_CPPFLAGS = -I$(top_srcdir)
GalactiCombatServer_SOURCES = BaseApplication.cpp GalactiCombat.cpp GameObject.cpp OgreMotionState.cpp PhysicsSimulator.cpp GUIManager.cpp GalactiCombatServer.cpp InputManager.cpp SoundManager.cpp NetworkManagerClient.cpp Bullet.cpp SpaceShip.cpp ComputerSpaceShipController.cpp Mineral.cpp ClientSpaceShipController.cpp
GalactiCombatServer_CXXFLAGS = $(OGRE_CFLAGS) $(OIS_CFLAGS) $(bullet_CFLAGS) $(sdl_CFLAGS) $(CEGUI_CFLAGS) $(CEGUI_OGRE_CFLAGS) $(SDL_net_CFLAGS)
GalactiCombatServer_LDADD = $(OGRE_LIBS) $(OIS_LIBS) $(bullet_LIBS) $(sdl_LIBS) $(CEGUI_LIBS) $(CEGUI_OGRE_LIBS) $(SDL_net_LIBS)

EXTRA_DIST = buildit makeit
AUTOMAKE_OPTIONS = foreign
