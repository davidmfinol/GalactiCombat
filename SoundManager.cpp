#include "SoundManager.h"

/*
 * Most of the code here is ripped from http://olofson.net/examples.html.
 */

SDL_AudioSpec audioSpec;
int *volumes = new int[NUM_SOUNDS];

struct sample {
    Uint8 *data;
    Uint32 dpos;
    Uint32 dlen;
} sounds[NUM_SOUNDS];

SoundManager::SoundManager(void)
{
//	printf("Initializing sound manager.");
	for(int i = 0; i < NUM_SOUNDS; i++){
		volumes[i] = SDL_MIX_MAXVOLUME/2;
	}
    int status = this->open();
    if(status != 0){
        fprintf(stderr, "Could not start audio! %s", SDL_GetError());
    }
}

SoundManager::~SoundManager(void)
{
	delete volumes;
    this->close();
}

/*
 * 		mixAudio(void*, Uint8*, int):
 *		Audio mixer so that more than one sound can be played simultaneously
 */
void mixAudio(void *unused, Uint8 *stream, int len)
{
    int i;
    Uint32 amount;
    
    for ( i=0; i<NUM_SOUNDS; ++i ) {
        amount = (sounds[i].dlen-sounds[i].dpos);
        if ( amount > len ) {
            amount = len;
        }
        SDL_MixAudio(stream, &sounds[i].data[sounds[i].dpos], amount, volumes[i]);
        sounds[i].dpos += amount;
    }
}

/*
 * 		open():
 *		Initializes the audioSpec to the desired format and starts the audio
 * 		callback process.
 */

int SoundManager::open(void)
{
    SDL_AudioSpec as;
    
    memset(sounds, 0, sizeof(sounds));
    
    if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
        return -2;
    
    as.freq = 44100; //44.1 kHz
    as.format = AUDIO_S16SYS; //Signed 16-bit samples
    as.channels = 2; //stereo
    //sample size = (16-bit audio) * (2 channels) = 32 bits	
    as.samples = 1024; //1024 samples per buffer (buffer size of 2KB)
    as.callback = mixAudio;
        
    if(SDL_OpenAudio(&as, &audioSpec) < 0)
        return -3;
    
    if(audioSpec.format != AUDIO_S16SYS)
        return -4;
    
    SDL_PauseAudio(0);
    return 0;
        
}

/*
 * 		close():
 *		Stops the audio callback process, frees memory, and closes the audio device
 */
void SoundManager::close(void)
{
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    for(int i = 0; i < NUM_SOUNDS; i++)
        SDL_FreeWAV(sounds[i].data);
    memset(sounds, 0, sizeof(sounds));
}

/*
 *		adjustVolume(int, double):
 *		Adjusts the volume of track to the indicated level (percentage).
 *
 *		-track: either SOUNDS or MUSIC
 *		-level: a number between 0 and 1 representing the percentage of
 *				maximum volume
 */
void SoundManager::adjustVolume(int track, double level)
{
	if(level < 0 || level > 1.0){
		printf("SoundManager error. Invalid volume adjustment.\n");
		return;
	}
	if(track != SOUNDS && track != MUSIC){
		printf("SoundManager error. Invalid track.\n");
		return;
	}

	volumes[track] = (int)(level * SDL_MIX_MAXVOLUME);
}

/*
 * 		playSound(std::string):
 *		Plays a sound effect. The sound must be a 16-bit .wav file.
 *
 *		-fileName: String representation of the filename.
 */
void SoundManager::playSound(std::string fileName)
{
    char * file = const_cast<char*>(fileName.c_str());
    SDL_AudioSpec wave;
    Uint8 *data;
    Uint32 dlen;
    SDL_AudioCVT cvt;

    // Load the sound file and convert it to 16-bit stereo at 22kHz
    if ( SDL_LoadWAV(file, &wave, &data, &dlen) == NULL ) {
        fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
        return;
    }
    SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq, AUDIO_S16, 2, 22050);
    cvt.buf = (Uint8*)malloc(dlen*cvt.len_mult);
    memcpy(cvt.buf, data, dlen);
    cvt.len = dlen;
    SDL_ConvertAudio(&cvt);
    SDL_FreeWAV(data);
    
    //Put the sound data in the slot (it starts playing immediately)
    if ( sounds[SOUNDS].data ) {
        free(sounds[SOUNDS].data);
    }
    //SDL_LockAudio();
    sounds[SOUNDS].data = cvt.buf;
    sounds[SOUNDS].dlen = cvt.len_cvt;
    sounds[SOUNDS].dpos = 0;
    //SDL_UnlockAudio();
}

/*
 *		playMusic(std::string):
 *		Plays background music. The music must be a 16-bit .wav file.
 *
 *		-fileName: String representation of the filename.
 */
void SoundManager::playMusic(std::string fileName)
{
    char * file = const_cast<char*>(fileName.c_str());
    SDL_AudioSpec wave;
    Uint8 *data;
    Uint32 dlen;
    SDL_AudioCVT cvt;
    
    // Load the music file and convert it to 16-bit stereo at 44.1kHz
    if ( SDL_LoadWAV(file, &wave, &data, &dlen) == NULL ) {
        fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
        return;
    }
    SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq, AUDIO_S16, 2, 44100);
    cvt.buf = (Uint8*)malloc(dlen*cvt.len_mult);
    memcpy(cvt.buf, data, dlen);
    cvt.len = dlen;
    SDL_ConvertAudio(&cvt);
    SDL_FreeWAV(data);
    
    // Put the sound data in the slot (it starts playing immediately)
    if ( sounds[MUSIC].data ) {
        free(sounds[MUSIC].data);
    }
    //SDL_LockAudio();
    sounds[MUSIC].data = cvt.buf;
    sounds[MUSIC].dlen = cvt.len_cvt;
    sounds[MUSIC].dpos = 0;
    //SDL_UnlockAudio();
}
