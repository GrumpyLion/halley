#pragma once
#include "halley/audio/audio_source.h"
#include "halley/audio/resampler.h"
#include "halley/audio/audio_buffer.h"

namespace Halley
{
	class AudioFilterResample final : public AudioSource
	{
	public:
		AudioFilterResample(std::shared_ptr<AudioSource> source, int fromHz, int toHz, AudioBufferPool& pool);

		uint8_t getNumberOfChannels() const override;
		bool isReady() const override;
		bool getAudioData(size_t numSamples, AudioMultiChannelSamples dst) override;
		size_t getSamplesLeft() const override;
		void restart() override;

		void setFromHz(int fromHz);

	private:
		AudioBufferPool& pool;
		std::shared_ptr<AudioSource> source;
		Vector<std::unique_ptr<AudioResampler>> resamplers;
		int fromHz;
		int toHz;

		struct LeftOverData
		{
			std::array<AudioSample, 8> samples;
			size_t n = 0;
		};
		std::array<LeftOverData, AudioConfig::maxChannels> leftoverSamples;
	};
}
