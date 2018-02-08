For Visual Studio ***2015*** to build the project, 3 environment variables 
must be defined: $(OGRE_HOME), $(BULLET_HOME), and $(SDL_HOME).

-----------------------------------------------------------
$(OGRE_HOME):
Follow the setup instructions on the Ogre website.

http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Setting%20Up%20An%20Application%20-%20Visual%20Studio
-----------------------------------------------------------
$(BULLET_HOME):
Make sure that this points to the base directory of
the Bullet SDK.

Bullet download: https://forums.ogre3d.org/viewtopic.php?t=69274
-----------------------------------------------------------
$(SDL_HOME):
Make sure that this points to the base directory of
the SDL library. Additionally, make sure that the include 
directory contains SDL.h, SDL_audio.h, and SDL_net.h and that
lib/x86/ contains SDL_net.dll and SDL_net.lib. 

SDL 1.2 download: http://www.libsdl.org/
SDL_net download: http://www.libsdl.org/projects/SDL_net

============================================================
Put copies of SDL dlls and libs in $(OGRE_HOME)\bin\${Configuration}.
Also, copy the resources.cfg,plugins.cfg, and the media/ folder to $(OGRE_HOME)\bin\${Configuration}.
Visual Studio is set to compile to that folder. Run the .exe from there.
