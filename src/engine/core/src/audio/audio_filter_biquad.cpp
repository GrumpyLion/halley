#include "halley/audio/audio_filter_biquad.h"

using namespace Halley;

AudioFilterBiquad::AudioFilterBiquad(std::shared_ptr<AudioSource> src)
	: src(std::move(src))
{
}

uint8_t AudioFilterBiquad::getNumberOfChannels() const
{
	return src->getNumberOfChannels();
}

bool AudioFilterBiquad::isReady() const
{
	return src->isReady();
}

bool AudioFilterBiquad::getAudioData(size_t numSamples, AudioMultiChannelSamples dst)
{
	// TODO
	return src->getAudioData(numSamples, dst);
}

size_t AudioFilterBiquad::getSamplesLeft() const
{
	return src->getSamplesLeft();
}

void AudioFilterBiquad::restart()
{
	src->restart();
}
