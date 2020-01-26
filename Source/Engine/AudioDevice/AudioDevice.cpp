#include "AudioDevice/AudioDevice.h"

#include <SDL.h>

#include "Log.h"

// Defines the supported audio format (based on wav files)
#define AUDIO_FORMAT AUDIO_S16LSB
#define AUDIO_FREQUENCY 48000
#define AUDIO_CHANNELS 2 // Number of concurrent audio channels
#define AUDIO_SAMPLES 4096 // Amount of audio data in the buffer at once (always power of 2)


namespace
{
    uint8_t* audioBufferPos;
    uint32_t audioBufferRemainingLength;
}

void AudioCallback(void *userdata, Uint8 *stream, int requestedLength)
{
    //Log::Print(Log::EMsg, "Audio Callback asking for %i data, audio buffer remaining %i", requestedLength, audioBufferRemainingLength);
    memset(stream, 0, requestedLength);

    if (audioBufferRemainingLength == 0)
    {
        return;
    }

    int length = requestedLength > (int)audioBufferRemainingLength ? audioBufferRemainingLength : requestedLength;

    // Copy the appropriate amount from the buffer into the stream
    memcpy(stream, audioBufferPos, length);

    audioBufferPos += length;
    audioBufferRemainingLength -= length;

    // We'll have a userdata to pass in a list of currently playing audio elements
    // We'll loop over them, and copy from their current position, the appropriate length, adding to the existing stream memory

}

void AudioDevice::Initialize()
{
    Log::Print(Log::EAudio, "Audio Initialization, listing available %i devices", SDL_GetNumAudioDevices(0));
    for (int i = 0; i < SDL_GetNumAudioDevices(0); i++)
    {
        Log::Print(Log::EAudio, "   - Device %i - %s", i,  SDL_GetAudioDeviceName(i, 0));
    }

    SDL_AudioDeviceID device;

    SDL_AudioSpec desiredWaveSpec, gotWaveSpec;
    desiredWaveSpec.callback = AudioCallback;
    desiredWaveSpec.userdata = nullptr;
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
    

    // @Improvement: check if the imported data actually has the right format
    SDL_AudioSpec testAudioSpec;
    uint32_t waveLength;
    uint8_t* waveBuffer;
    if (SDL_LoadWAV("Resources/Audio/Shoot.wav", &testAudioSpec, &waveBuffer, &waveLength) == nullptr)
    {
        Log::Print(Log::EErr, "%s", SDL_GetError());
        return;
    }
    audioBufferPos = waveBuffer; // Start at the beginning of the buffer
    audioBufferRemainingLength = waveLength;

    SDL_PauseAudioDevice(device, 0);
}

void AudioDevice::Update()
{
}

void AudioDevice::Destroy()
{
    SDL_PauseAudio(1);
    SDL_CloseAudio();
}

