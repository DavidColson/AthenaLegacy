
typedef unsigned long long SoundID;

namespace AudioDevice
{
    void Initialize();

    SoundID PlaySound(const char* fileName, float volume);

    void PauseSound(SoundID sound);

    void UnPauseSound(SoundID sound);

    void Update();

    void Destroy();
}