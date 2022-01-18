#ifndef _ABFW_AUDIO_MANAGER_D3D11_H
#define _ABFW_AUDIO_MANAGER_D3D11_H

#include <audio/audio_manager.h>
#include <vector>
#include <gef.h>
#include <external/SFML/Audio.hpp>
#include <system/file.h>

namespace gef
{

class AudioManagerD3D11 : public AudioManager
{
public:
	AudioManagerD3D11();
	~AudioManagerD3D11();

	Int32 LoadSample(const char *strFileName, const Platform& platform);
	Int32 LoadMusic(const char *strFileName, const Platform& platform);
	Int32 PlayMusic();
	Int32 StopMusic();
	Int32 PlaySample(const Int32 sample_index, const bool looping = false);
	Int32 StopPlayingSampleVoice(const Int32 voice_index);
	void UnloadMusic();
	void UnloadSample(Int32 sample_num);
	void UnloadAllSamples();


	Int32 WriteAudioOut( const short *pBuffer );
	Int32 SetSamplePitch(const Int32 voice_index, float pitch);
	Int32 SetMusicPitch(float pitch);
	Int32 GetSampleVoiceVolumeInfo(const Int32 voice_index, struct VolumeInfo& volume_info);
	Int32 SetSampleVoiceVolumeInfo(const Int32 voice_index, const struct VolumeInfo& volume_info);
	Int32 GetMusicVolumeInfo(struct VolumeInfo& volume_info);
	Int32 SetMusicVolumeInfo(const struct VolumeInfo& volume_info);
	Int32 SetMasterVolume(float volume);
	Int32 LockMusicMutex();
	Int32 UnlockMusicMutex();

	bool sample_voice_playing(const UInt32 voice_index);
	bool sample_voice_looping(const UInt32 voice_index);

	void SetMusicLoop(bool looping);
	bool GetMusicLoop();

private:
	std::vector<UInt8*> rawSampleData_;
	std::vector<sf::SoundBuffer*> sampleBuffers_;
	std::vector<sf::Sound*> samples_;
	sf::Music music;
	UInt8* musicBuffer;
	File* musicFile;
};

}

#endif // _ABFW_AUDIO_MANAGER_VITA_H

