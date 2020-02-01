#include "AudioDevice/AudioDevice.h"

#include <SDL.h>

#include "Log.h"

// Defines the supported audio format (based on wav files)
#define AUDIO_FORMAT AUDIO_S16LSB
#define AUDIO_FREQUENCY 44100
#define AUDIO_CHANNELS 1 // Number of concurrent audio channels
#define AUDIO_SAMPLES 4096 // Amount of audio data in the buffer at once (always power of 2)
#define AUDIO_MAX_SOUNDS 16 // Maximum amount of concurrent sounds

struct Sound
{
    SoundID id{ SoundID(-1) };
    bool paused{ false };
    bool active{ false };
    bool loop{ false };
    uint8_t* buffer{ nullptr };
    uint8_t* currentBufferPosition{ nullptr };

    float volume{ 1.0f };
    uint32_t length{ 0 };
    uint32_t remainingLength{ 0 };
    SDL_AudioSpec spec;
};

struct AudioCallbackData
{
    // @todo: this does make more sense as a linked list
    Sound currentSounds[AUDIO_MAX_SOUNDS];
};

namespace
{
    float globalVolume = 1.0f;
    SDL_AudioDeviceID device;
    uint32_t currentNumSounds{ 0 };
    AudioCallbackData callbackData; // Do not write without locking audio callback thread
}

void AudioCallback(void *userdata, Uint8 *stream, int requestedLength)
{
    AudioCallbackData* data = (AudioCallbackData*)userdata;
    memset(stream, 0, requestedLength);    

    for (uint32_t i = 0; i < AUDIO_MAX_SOUNDS; i++)
    {
        Sound* sound = &data->currentSounds[i];
        if (!sound->active || sound->paused)
            continue;

        int length = requestedLength > (int)sound->remainingLength ? sound->remainingLength : requestedLength;

        // We're dealing with signed 16 bit audio here, so cast up
        int16_t* dest = (int16_t*)stream;
        int16_t* src = (int16_t*)sound->currentBufferPosition;

        // 1 << x will be 2^x, so we're getting the highest number you can get with a signed 16 bit number
        int max = ((1 << (16 - 1)) - 1);
        int min = -(1 << (16 - 1));

        loopContinue:
        // Mix into the destination stream
        int len = length / 2; // Length is assuming 8 bit, so divide by 2
        while (len--)
        {
            // volume adjust and mix
            int srcSample = *src;
            srcSample = int(srcSample * globalVolume * sound->volume);
            int destSample = (*dest + srcSample);         

            // clipping
            if (destSample > max)
                destSample = max;
            else if (destSample < min)
                destSample = min;

            *dest = destSample;
            dest++;
            src++;
        }

        sound->currentBufferPosition += length;
        sound->remainingLength -= length;   

        if (sound->remainingLength == 0)
        {
            if (sound->loop == false)
            {
                // Free sound
                SDL_FreeWAV(sound->buffer);
                *sound = Sound();
                currentNumSounds--;
                continue;
            }
            else
            {
                sound->remainingLength = sound->length;
                sound->currentBufferPosition = sound->buffer;

                length = requestedLength - length;
                src = (int16_t*)sound->currentBufferPosition;
                goto loopContinue;
            }
        }
    }
}

void AudioDevice::Initialize()
{
    Log::Print(Log::EAudio, "Audio Initialization, listing available %i devices", SDL_GetNumAudioDevices(0));
    for (int i = 0; i < SDL_GetNumAudioDevices(0); i++)
    {
        Log::Print(Log::EAudio, "   - Device %i - %s", i,  SDL_GetAudioDeviceName(i, 0));
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
        Log::Print(Log::EAudio, "Successfully opened audio device - %s",  SDL_GetAudioDeviceName(0, 0));
    }
    else
    {
        Log::Print(Log::EAudio, "Failed to open audio device - %s",  SDL_GetAudioDeviceName(0, 0));        
    }

    SDL_PauseAudioDevice(device, 0);
}

SoundID AudioDevice::PlaySound(const char* fileName, float volume, bool loop)
{
    if (currentNumSounds < AUDIO_MAX_SOUNDS)
    {
        Sound tempNewSound;

         // @Improvement: check if the imported data actually has the right format
        if (SDL_LoadWAV(fileName, &(tempNewSound.spec), &(tempNewSound.buffer), &(tempNewSound.length)) == nullptr)
        {
            Log::Print(Log::EErr, "%s", SDL_GetError());
            return SoundID(-1);
        }
        tempNewSound.currentBufferPosition = tempNewSound.buffer; // Start at the beginning of the buffer
        tempNewSound.remainingLength = tempNewSound.length;
        tempNewSound.active = true;
        tempNewSound.loop = loop;
        tempNewSound.volume = volume;

        SDL_LockAudioDevice(device);
        // Find the next free slot in the memory pool of sounds
        SoundID id = -1;
        for (int i = 0; i < AUDIO_MAX_SOUNDS; i++)
        {
            if (callbackData.currentSounds[i].active == false)
            {
                // cast to 32bit and increment to get new version, then build new id
                uint32_t version = ((uint32_t)callbackData.currentSounds[currentNumSounds].id) + 1;
                tempNewSound.id = ((SoundID)i << 32) | ((SoundID)version);
                callbackData.currentSounds[currentNumSounds] = tempNewSound;
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
    SDL_LockAudioDevice(device);
    uint32_t index = sound >> 32;
    if (callbackData.currentSounds[index].id == sound)
    {
        callbackData.currentSounds[index].paused = false;
    }
    SDL_UnlockAudioDevice(device);
}

void AudioDevice::Update()
{
}

void AudioDevice::Destroy()
{
    SDL_PauseAudio(1);
    SDL_CloseAudioDevice(device);
}

