#include "AudioDevice.h"

#include <SDL.h>
#include <vector>

#include "Log.h"

// Defines the supported audio format (based on wav files)
#define AUDIO_FORMAT AUDIO_S16LSB
#define AUDIO_FREQUENCY 44100
#define AUDIO_CHANNELS 2 // Number of concurrent audio channels
#define AUDIO_SAMPLES 4096 // Amount of audio data in the buffer at once (always power of 2)
#define AUDIO_MAX_SOUNDS 16 // Maximum amount of concurrent sounds

struct PlayingSound
{
    SoundID id{ SoundID(-1) };
    bool paused{ false };
    bool active{ false };
    bool loop{ false };
    uint8_t* currentBufferPosition{ nullptr };

    float volume{ 1.0f };
    uint32_t remainingLength{ 0 };
    LoadedSound* loadedSound;
};

struct AudioCallbackData
{
    // @todo: this does make more sense as a linked list
    PlayingSound currentSounds[AUDIO_MAX_SOUNDS];
};

namespace
{
    float globalVolume = 1.0f;
    SDL_AudioDeviceID device;
    uint32_t currentNumSounds{ 0 };
    AudioCallbackData callbackData; // Do not write without locking audio callback thread
}

void AudioCallback(void *userdata, Uint8 *stream, int nRequestedBytes)
{
    AudioCallbackData* data = (AudioCallbackData*)userdata;
    memset(stream, 0, nRequestedBytes);    

    for (uint32_t i = 0; i < AUDIO_MAX_SOUNDS; i++)
    {
        PlayingSound* sound = &(data->currentSounds[i]);
        if (!sound->active || sound->paused)
            continue;

        int nBytesToReturn = nRequestedBytes > (int)sound->remainingLength ? sound->remainingLength : nRequestedBytes;

        // We're dealing with signed 16 bit audio here, so cast up
        int16_t* dest = (int16_t*)stream;
        int16_t* src = (int16_t*)sound->currentBufferPosition;

        loopContinue:
        // Mix into the destination stream
        int nBytesToUse = sound->loadedSound->spec.channels == 1 ? nBytesToReturn / 2 : nBytesToReturn; // For single channel audio we're using every sample twice
        int nSamplesToMix = nBytesToUse / 2;
        while (nSamplesToMix--)
        {
            // volume adjust and mix
            int16_t srcSample = *src;
            srcSample = int16_t(srcSample * globalVolume * sound->volume);
            int16_t destSample = (*dest + srcSample);         

            // clipping
            if (destSample > INT16_MAX)
                destSample = INT16_MAX;
            else if (destSample < INT16_MIN)
                destSample = INT16_MIN;

            *dest = destSample;
            dest++;
            if (sound->loadedSound->spec.channels == 1)
            {
                *dest = destSample;
                dest++;
            }
            src++;
        }

        sound->currentBufferPosition += nBytesToUse;
        sound->remainingLength -= nBytesToReturn;   

        if (sound->remainingLength <= 0)
        {
            if (sound->loop == false)
            {
                *sound = PlayingSound();
                currentNumSounds--;
                continue;
            }
            else
            {
                sound->remainingLength = sound->loadedSound->length;
                sound->currentBufferPosition = sound->loadedSound->buffer;

                nBytesToReturn = nRequestedBytes - nBytesToReturn;
                src = (int16_t*)sound->currentBufferPosition;
                goto loopContinue;
            }
        }
    }
}

void AudioDevice::Initialize()
{
    Log::Info("Audio Initialization, listing available %i devices", SDL_GetNumAudioDevices(0));
    for (int i = 0; i < SDL_GetNumAudioDevices(0); i++)
    {
        Log::Info("   - Device %i - %s", i,  SDL_GetAudioDeviceName(i, 0));
    }

    SDL_AudioSpec desiredWaveSpec, gotWaveSpec;
    desiredWaveSpec.callback = AudioCallback;
    desiredWaveSpec.userdata = &callbackData;
    desiredWaveSpec.channels = AUDIO_CHANNELS;
    desiredWaveSpec.format = AUDIO_FORMAT;
    desiredWaveSpec.freq = AUDIO_FREQUENCY;
    desiredWaveSpec.samples = AUDIO_SAMPLES;
    device = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, 0), 0, &desiredWaveSpec, &gotWaveSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (device != 0)
    {
        Log::Info("Successfully opened audio device - %s",  SDL_GetAudioDeviceName(0, 0));
    }
    else
    {
        Log::Info("Failed to open audio device - %s",  SDL_GetAudioDeviceName(0, 0));        
    }

    SDL_PauseAudioDevice(device, 0);
}

LoadedSoundPtr AudioDevice::LoadSound(const char* fileName)
{
    LoadedSoundPtr newSound = eastl::make_shared<LoadedSound>();

    if (SDL_LoadWAV(fileName, &(newSound->spec), &(newSound->buffer), &(newSound->length)) == nullptr)
    {
        Log::Warn("%s", SDL_GetError());
    }

    if (newSound->spec.channels == 1) // doubling our actual buffer length since we reuse the samples for stereo
        newSound->length *= 2;

    // @TODO Ensure the loaded audio file is of the correct format and give errors otherwise
    return newSound;
}

SoundID AudioDevice::PlaySound(LoadedSoundPtr sound, float volume, bool loop)
{
    if (currentNumSounds < AUDIO_MAX_SOUNDS)
    {
        PlayingSound tempNewSound;
        tempNewSound.loadedSound = sound.get();

        tempNewSound.currentBufferPosition = tempNewSound.loadedSound->buffer; // Start at the beginning of the buffer
        tempNewSound.remainingLength = tempNewSound.loadedSound->length;

        tempNewSound.active = true;
        tempNewSound.loop = loop;
        tempNewSound.volume = volume;

        SDL_LockAudioDevice(device);
        // Find the next free slot in the memory pool of sounds
        for (int i = 0; i < AUDIO_MAX_SOUNDS; i++)
        {
            if (callbackData.currentSounds[i].active == false)
            {
                // cast to 32bit and increment to get new version, then build new id
                uint32_t version = ((uint32_t)callbackData.currentSounds[i].id) + 1;
                tempNewSound.id = ((SoundID)i << 32) | ((SoundID)version);
                callbackData.currentSounds[i] = tempNewSound;
                break;
            }
        }
        SDL_UnlockAudioDevice(device);
        currentNumSounds++;
        return tempNewSound.id;
    }
    return SoundID(-1);
}

void AudioDevice::PauseSound(SoundID sound)
{
    if (sound == SoundID(-1))
        return;

    SDL_LockAudioDevice(device);
    uint32_t index = sound >> 32;
    if (callbackData.currentSounds[index].id == sound)
    {
        callbackData.currentSounds[index].paused = true;
    }
    SDL_UnlockAudioDevice(device);
}

void AudioDevice::UnPauseSound(SoundID sound)
{
    if (sound == SoundID(-1))
        return;
        
    SDL_LockAudioDevice(device);
    uint32_t index = sound >> 32;
    if (callbackData.currentSounds[index].id == sound)
    {
        callbackData.currentSounds[index].paused = false;
    }
    SDL_UnlockAudioDevice(device);
}

void AudioDevice::StopSound(SoundID sound)
{
     if (sound == SoundID(-1))
        return;

    SDL_LockAudioDevice(device);
    uint32_t index = sound >> 32;
    if (callbackData.currentSounds[index].id == sound)
    {
        callbackData.currentSounds[index] = PlayingSound();
    }
    SDL_UnlockAudioDevice(device);
    currentNumSounds = 0;
}

void AudioDevice::Destroy()
{
    SDL_PauseAudio(1);
    SDL_CloseAudioDevice(device);
}
