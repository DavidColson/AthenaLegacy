#include "AudioDevice/AudioDevice.h"

#include <SDL.h>

#include "Log.h"

// Defines the supported audio format (based on wav files)
#define AUDIO_FORMAT AUDIO_S16LSB
#define AUDIO_FREQUENCY 48000
#define AUDIO_CHANNELS 2 // Number of concurrent audio channels
#define AUDIO_SAMPLES 4096 // Amount of audio data in the buffer at once (always power of 2)
#define AUDIO_MAX_SOUNDS 16 // Maximum amount of concurrent sounds

struct Sound
{
    bool active{false};
    uint8_t* buffer{ nullptr };
    uint8_t* currentBufferPosition{ nullptr };

    float volume{ 1.0f };
    uint32_t length{ 0 };
    uint32_t remainingLength{ 0 };
    SDL_AudioSpec spec;
};

struct AudioCallbackData
{
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
        if (!sound->active)
            continue;

        if (sound->remainingLength == 0)
        {
            // Free sound
            SDL_FreeWAV(sound->buffer);
            sound->buffer = nullptr;
            sound->currentBufferPosition = nullptr;
            currentNumSounds--;
            sound->active = false;
            continue;
        }

        int length = requestedLength > (int)sound->remainingLength ? sound->remainingLength : requestedLength;

        // We're dealing with signed 16 bit audio here, so cast up
        int16_t* dest = (int16_t*)stream;
        int16_t* src = (int16_t*)sound->currentBufferPosition;

        // 1 << x will be 2^x, so we're getting the highest number you can get with a signed 16 bit number
        int max = ((1 << (16 - 1)) - 1);
        int min = -(1 << (16 - 1));

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

    PlayAudio("Resources/Audio/Shoot.wav", 1.0f);
    PlayAudio("Resources/Audio/Bluezone-Abyss-sound-004.wav", 1.0f);
    PlayAudio("Resources/Audio/Detunized_Urban-Crows_01.wav", 1.0f);
}

void AudioDevice::PlayAudio(const char* fileName, float volume)
{
    if (currentNumSounds < AUDIO_MAX_SOUNDS)
    {
        Sound tempNewSound;

         // @Improvement: check if the imported data actually has the right format
        if (SDL_LoadWAV(fileName, &(tempNewSound.spec), &(tempNewSound.buffer), &(tempNewSound.length)) == nullptr)
        {
            Log::Print(Log::EErr, "%s", SDL_GetError());
            return;
        }
        tempNewSound.currentBufferPosition = tempNewSound.buffer; // Start at the beginning of the buffer
        tempNewSound.remainingLength = tempNewSound.length;
        tempNewSound.active = true;
        tempNewSound.volume = volume;

        SDL_LockAudioDevice(device);
        // Find the next free slot in the memory pool of sounds
        for (int i = 0; i < AUDIO_MAX_SOUNDS; i++)
        {
            if (callbackData.currentSounds[i].active == false)
            {
                callbackData.currentSounds[currentNumSounds] = tempNewSound;
                break;
            }
        }
        SDL_UnlockAudioDevice(device);
        currentNumSounds++;
    }
}

void AudioDevice::Update()
{
}

void AudioDevice::Destroy()
{
    SDL_PauseAudio(1);
    SDL_CloseAudioDevice(device);
}

