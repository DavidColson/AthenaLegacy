#include "AudioDevice/AudioDevice.h"

#include <SDL.h>

#include "Log.h"

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
}

void AudioDevice::Initialize()
{
    Uint64 start = SDL_GetPerformanceCounter();

    uint32_t waveLength;
    uint8_t* waveBuffer;
    SDL_AudioSpec waveSpec;
    if (SDL_LoadWAV("Resources/Audio/Shoot.wav", &waveSpec, &waveBuffer, &waveLength) == nullptr)
    {
        Log::Print(Log::EErr, "%s", SDL_GetError());
        return;
    }

    waveSpec.callback = AudioCallback;
    waveSpec.userdata = nullptr;

    audioBufferPos = waveBuffer; // Start at the beginning of the buffer
    audioBufferRemainingLength = waveLength;

    if (SDL_OpenAudio(&waveSpec, nullptr) < 0)
    {
        Log::Print(Log::EErr, "Error opening audio device: %s", SDL_GetError());
        return;
    }

    SDL_PauseAudio(0);

    double duration = double(SDL_GetPerformanceCounter() - start) / SDL_GetPerformanceFrequency();
    Log::Print(Log::EMsg, "Initialize time %f", duration);
}

void AudioDevice::Update()
{
}

void AudioDevice::Destroy()
{
    SDL_PauseAudio(1);
    SDL_CloseAudio();
}

