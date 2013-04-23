#ifndef __SoundManager_h_
#define __SoundManager_h_

#include <string>
#include "SDL/SDL.h"
#include "SDL/SDL_audio.h"

#define NUM_SOUNDS 2

#define MUSIC 0
#define SOUNDS 1

class SoundManager {

public:
    SoundManager(void);
    virtual ~SoundManager(void);

	/*      playSound(std::string) and playMusic(std::string):
	 *
	 *      Plays a sound effect and background music, respectively.
	 *		Must be 16-bit .wav file.
	 *
	 *      -fileName: String representation of the filename.
	 */
    void playSound(std::string fileName);
    void playMusic(std::string fileName);

	/*      adjustVolume(int, double):
	 *      Adjusts the volume of track to the indicated level (percentage).
	 *
	 *      -track: either SOUNDS or MUSIC
	 *      -level: a number between 0 and 1 representing the percentage of
	 *              maximum volume
	 */
    void adjustVolume(int track, double level);

protected:

    int open(void);
    void close(void);
};
#endif //#ifndef __SoundManager_h_
