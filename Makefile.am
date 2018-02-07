noinst_HEADERS = src/BaseApplication.h src/GalactiCombat.h src/GameObject.h src/OgreMotionState.h src/PhysicsSimulator.h src/GUIManager.h src/GalactiCombatServer.h src/NetworkManagerClient.h src/InputManager.h src/SoundManager.h src/NetworkUtil.h src/Bullet.h src/SpaceShip.h src/ISpaceShipController.h src/ComputerSpaceShipController.h src/Mineral.h src/ClientSpaceShipController.h


bin_PROGRAMS = GalactiCombat GalactiCombatServer

GalactiCombat_CPPFLAGS = -I$(top_srcdir)
GalactiCombat_SOURCES = src/BaseApplication.cpp src/GalactiCombat.cpp src/GameObject.cpp src/OgreMotionState.cpp src/PhysicsSimulator.cpp src/GUIManager.cpp src/NetworkManagerClient.cpp src/InputManager.cpp src/SoundManager.cpp src/GalactiCombatMain.cpp src/Bullet.cpp src/SpaceShip.cpp src/ComputerSpaceShipController.cpp src/Mineral.cpp src/ClientSpaceShipController.cpp
GalactiCombat_CXXFLAGS = $(OGRE_CFLAGS) $(OIS_CFLAGS) $(bullet_CFLAGS) $(sdl_CFLAGS) $(CEGUI_CFLAGS) $(CEGUI_OGRE_CFLAGS) $(SDL_net_CFLAGS)
GalactiCombat_LDADD = $(OGRE_LIBS) $(OIS_LIBS) $(bullet_LIBS) $(sdl_LIBS) $(CEGUI_LIBS) $(CEGUI_OGRE_LIBS) $(SDL_net_LIBS)

GalactiCombatServer_CPPFLAGS = -I$(top_srcdir)
GalactiCombatServer_SOURCES = src/BaseApplication.cpp src/GalactiCombat.cpp src/GameObject.cpp src/OgreMotionState.cpp src/PhysicsSimulator.cpp src/GUIManager.cpp src/GalactiCombatServer.cpp src/InputManager.cpp src/SoundManager.cpp src/NetworkManagerClient.cpp src/Bullet.cpp src/SpaceShip.cpp src/ComputerSpaceShipController.cpp src/Mineral.cpp src/ClientSpaceShipController.cpp
GalactiCombatServer_CXXFLAGS = $(OGRE_CFLAGS) $(OIS_CFLAGS) $(bullet_CFLAGS) $(sdl_CFLAGS) $(CEGUI_CFLAGS) $(CEGUI_OGRE_CFLAGS) $(SDL_net_CFLAGS)
GalactiCombatServer_LDADD = $(OGRE_LIBS) $(OIS_LIBS) $(bullet_LIBS) $(sdl_LIBS) $(CEGUI_LIBS) $(CEGUI_OGRE_LIBS) $(SDL_net_LIBS)

EXTRA_DIST = buildit makeit
AUTOMAKE_OPTIONS = foreign
