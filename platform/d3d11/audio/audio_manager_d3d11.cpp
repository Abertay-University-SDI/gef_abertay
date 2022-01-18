#include <platform/d3d11/audio/audio_manager_d3d11.h>


namespace gef
{
	AudioManager* AudioManager::Create()
	{
		return new AudioManagerD3D11();
	}

	AudioManagerD3D11::AudioManagerD3D11() :
	musicFile(NULL),
	musicBuffer(NULL)
	{
		// Nothing to setup
		music.setLoop(true);
	}

	AudioManagerD3D11::~AudioManagerD3D11(void)
	{
		UnloadMusic();
		UnloadAllSamples();
	}

	Int32 AudioManagerD3D11::LoadSample(const char * strFileName, const Platform & platform)
	{
		// Create file and open the target sound file
		File* sampleFile = File::Create();
		if (sampleFile->Open(strFileName)) {
			// Get size of the file so we can read it properly
			Int32 fileSize;
			if (sampleFile->GetSize(fileSize)) {
				// Create a buffer thats the correct size and read the data into it
				UInt8* rawSample = new UInt8[fileSize];
				Int32 bytesRead = 0;
				if (sampleFile->Read(rawSample, fileSize, bytesRead)) {
					// Create new sound buffer and load in the data
					sf::SoundBuffer* newBuffer = new sf::SoundBuffer();
					if (newBuffer->loadFromMemory(rawSample, bytesRead)) {
						// Add the new buffer to the vector
						sampleBuffers_.push_back(newBuffer);

						// Create a new sound and assign it the buffer
						sf::Sound* newSound = new sf::Sound();
						newSound->setBuffer(*newBuffer);

						// Add new sound to the samples
						samples_.push_back(newSound);

						// Delete the old raw data
						delete[] rawSample;

						// Close the file and delete it
						sampleFile->Close();
						delete sampleFile;

						// Return the index of the new sounds
						return (int)(samples_.size() - 1);
					}
				}
				delete[] rawSample;
			}
			sampleFile->Close();
		}

		delete sampleFile;
		return -1;
	}

	Int32 AudioManagerD3D11::LoadMusic(const char * strFileName, const Platform & platform)
	{
		// If the music already has been loaded, unload it so it can be replaced
		if (musicFile && musicBuffer)
			UnloadMusic();

		// Create a file and open the target music file
		musicFile = gef::File::Create();
		if (musicFile->Open(strFileName)) 
		{
			// Get the file size so we can read the correct amount of data
			Int32 fileSize;
			if (musicFile->GetSize(fileSize))
			{
				// Create a buffer to store the music data
				musicBuffer = new UInt8[fileSize];
				Int32 bytesRead = 0;
				if (musicFile->Read(musicBuffer, fileSize, bytesRead)) 
				{
					// Pass data buffer to the music object so it can play
					if (music.openFromMemory(musicBuffer, bytesRead)) {
						return 0;
					}
				}
			}
		}

		// If any of the above checks fail, unload the music
		UnloadMusic();
		return -1;
	}

	Int32 AudioManagerD3D11::PlayMusic()
	{
		// Play the music...
		music.play();

		return 0;
	}

	Int32 AudioManagerD3D11::StopMusic()
	{
		// Stop the music...
		music.stop();

		return 0;
	}

	Int32 AudioManagerD3D11::PlaySample(const Int32 sample_index, const bool looping)
	{
		// Stop the request if the sample index does not exist
		if (sample_index < 0 || sample_index >(samples_.size() - 1))
			return -1;

		// Stop request if sample is NULL
		if (samples_[sample_index] == NULL)
			return -1;

		// Set looping and play on sample
		samples_[sample_index]->setLoop(looping);
		samples_[sample_index]->play();

		return sample_index;
	}

	Int32 AudioManagerD3D11::StopPlayingSampleVoice(const Int32 voice_index)
	{
		// Stop the request if the sample index does not exist
		if (voice_index < 0 || voice_index >(samples_.size() - 1))
			return -1;

		// Stop request if sample is NULL
		if (samples_[voice_index] == NULL)
			return -1;

		// Stop sample
		samples_[voice_index]->stop();

		return 0;
	}

	void AudioManagerD3D11::UnloadMusic()
	{
		// Stop the music
		music.stop();

		// Close and delete the music file
		if (musicFile) {
			musicFile->Close();
			delete musicFile;
			musicFile = NULL;
		}

		// delete the music buffer
		if (musicBuffer) {
			delete[] musicBuffer;
			musicBuffer = NULL;
		}
	}

	void AudioManagerD3D11::UnloadSample(Int32 sample_num)
	{
		// Stop the request if the sample index does not exist
		if (sample_num < 0 || sample_num >(samples_.size() - 1))
			return;

		// Delete the sample buffer
		if (sampleBuffers_[sample_num])
		{
			delete sampleBuffers_[sample_num];
			sampleBuffers_[sample_num] = NULL;
		}

		// Delete the sample itself
		if (samples_[sample_num])
		{
			delete samples_[sample_num];
			samples_[sample_num] = NULL;
		}
		
	}
	void AudioManagerD3D11::UnloadAllSamples()
	{
		// Remove all the samples
		for (int i = 0; i < samples_.size(); i++)
		{
			UnloadSample(i);
		}

		// Empty the vectors of all contents
		sampleBuffers_.clear();
		samples_.clear();
	}

	Int32 AudioManagerD3D11::WriteAudioOut(const short * pBuffer)
	{
		printf("No writting out audio for me...");

		return -1;
	}

	Int32 AudioManagerD3D11::SetSamplePitch(const Int32 voice_index, float pitch)
	{
		// Stop the request if the sample index does not exist
		if (voice_index < 0 || voice_index >(samples_.size() - 1))
			return -1;

		// Stop request if sample is NULL
		if (samples_[voice_index] == NULL)
			return -1;

		// Set the pitch of the sample
		samples_[voice_index]->setPitch(pitch);

		return 0;
	}

	Int32 AudioManagerD3D11::SetMusicPitch(float pitch)
	{
		// Set music pitch...
		music.setPitch(pitch);

		return 0;
	}

	Int32 AudioManagerD3D11::GetSampleVoiceVolumeInfo(const Int32 voice_index, VolumeInfo & volume_info)
	{
		// Stop the request if the sample index does not exist
		if (voice_index < 0 || voice_index >(samples_.size() - 1))
			return -1;

		// Stop request if sample is NULL
		if (samples_[voice_index] == NULL)
			return -1;

		// Assign the data to the argument
		volume_info.volume = samples_[voice_index]->getVolume();
		volume_info.pan = 0.f;

		return 0;
	}

	Int32 AudioManagerD3D11::SetSampleVoiceVolumeInfo(const Int32 voice_index, const VolumeInfo & volume_info)
	{
		// Stop the request if the sample index does not exist
		if (voice_index < 0 || voice_index >(samples_.size() - 1))
			return -1;

		// Stop request if sample is NULL
		if (samples_[voice_index] == NULL)
			return -1;

		// Assign new volume from the argument
		samples_[voice_index]->setVolume(volume_info.volume);

		return 0;
	}
	Int32 AudioManagerD3D11::GetMusicVolumeInfo(VolumeInfo & volume_info)
	{
		// Assign volume data to argument
		volume_info.volume = music.getVolume();
		volume_info.pan = 0.f;

		return 0;
	}

	Int32 AudioManagerD3D11::SetMusicVolumeInfo(const VolumeInfo & volume_info)
	{
		// Set music volume
		music.setVolume(volume_info.volume);

		return 0;
	}

	Int32 AudioManagerD3D11::SetMasterVolume(float volume)
	{
		// Set the master volume control of the listener
		sf::Listener::setGlobalVolume(volume);
		return 0;
	}

	Int32 AudioManagerD3D11::LockMusicMutex()
	{
		printf("Windows Audio Manager doesn't used Mutexes");

		return -1;
	}

	Int32 AudioManagerD3D11::UnlockMusicMutex()
	{
		printf("Windows Audio Manager doesn't used Mutexes");

		return -1;
	}

	bool AudioManagerD3D11::sample_voice_playing(const UInt32 voice_index)
	{
		// Stop the request if the sample index does not exist
		if (voice_index < 0 || voice_index >(samples_.size() - 1))
			return false;

		// Stop request if sample is NULL
		if (samples_[voice_index] == NULL)
			return false;

		// Check sample is playing
		return (samples_[voice_index]->getStatus() == sf::Sound::Playing);
	}

	bool AudioManagerD3D11::sample_voice_looping(const UInt32 voice_index)
	{
		// Stop the request if the sample index does not exist
		if (voice_index < 0 || voice_index >(samples_.size() - 1))
			return false;

		// Stop request if sample is NULL
		if (samples_[voice_index] == NULL)
			return false;

		// Check if sample is looping
		return samples_[voice_index]->getLoop();
	}

	void AudioManagerD3D11::SetMusicLoop(bool looping)
	{
		// Set music object to loop
		music.setLoop(looping);
	}

	bool AudioManagerD3D11::GetMusicLoop()
	{
		// Get value of music loop flag
		return music.getLoop();
	}
}