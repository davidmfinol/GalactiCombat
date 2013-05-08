Three environment variables must be defined for VS2010 to build
the project: $(OGRE_HOME), $(BULLET_HOME), and $(SDL_HOME).

-----------------------------------------------------------
$(OGRE_HOME):
Follow the setup instructions on the Ogre website.

http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Setting%20Up%20An%20Application%20-%20Visual%20Studio
-----------------------------------------------------------
$(BULLET_HOME):
Make sure that this points to the base directory of
the Bullet SDK.

Bullet download: http://code.google.com/p/bullet/
-----------------------------------------------------------
$(SDL_HOME):
Make sure that this points to the base directory of
the SDL library. Additionally, make sure that the include 
directory contains SDL.h, SDL_audio.h, and SDL_net.h.

SDL download: http://www.libsdl.org/
SDL_net download: http://www.libsdl.org/projects/SDL_net/