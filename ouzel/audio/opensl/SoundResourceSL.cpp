// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "core/CompileConfig.h"

#if OUZEL_SUPPORTS_OPENSL

#include "SoundResourceSL.h"
#include "AudioSL.h"
#include "audio/SoundData.h"
#include "audio/Stream.h"
#include "core/Engine.h"
#include "utils/Log.h"

static void playerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void* context)
{
    ouzel::audio::SoundResourceSL* soundResourceSL = reinterpret_cast<ouzel::audio::SoundResourceSL*>(context);

    soundResourceSL->enqueue(bufferQueue);
}

namespace ouzel
{
    namespace audio
    {
        static const uint32_t BUFFER_SIZE = 32 * 4096;

        SoundResourceSL::SoundResourceSL()
        {
        }

        SoundResourceSL::~SoundResourceSL()
        {
            if (playerObject)
            {
                (*playerObject)->Destroy(playerObject);
            }
        }

        bool SoundResourceSL::update()
        {
            AudioSL* audioSL = static_cast<AudioSL*>(sharedEngine->getAudio());

            if (dirty & DIRTY_SOUND_DATA)
            {
                if (soundData)
                {
                    if (playerObject)
                    {
                        (*playerObject)->Destroy(playerObject);
                        playerObject = nullptr;
                        player = nullptr;
                        bufferQueue = nullptr;
                        playerVolume = nullptr;
                    }

                    SLDataLocator_AndroidSimpleBufferQueue location = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };

                    SLDataFormat_PCM format;
                    format.formatType = SL_DATAFORMAT_PCM;
                    format.numChannels = soundData->getChannels();
                    format.samplesPerSec = soundData->getSamplesPerSecond() * 1000; //mHz
                    format.bitsPerSample = 16;
                    format.containerSize = 16;

                    if (soundData->getChannels() == 2)
                    {
                        format.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
                    }
                    else if (soundData->getChannels() == 1)
                    {
                        format.channelMask = SL_SPEAKER_FRONT_CENTER;
                    }
                    else
                    {
                        Log(Log::Level::ERR) << "Invalid channel count";
                        return false;
                    }

                    format.endianness = SL_BYTEORDER_LITTLEENDIAN;

                    SLDataSource dataSource = { &location, &format };

                    SLDataLocator_OutputMix dataLocatorOut;
                    dataLocatorOut.locatorType = SL_DATALOCATOR_OUTPUTMIX;
                    dataLocatorOut.outputMix = audioSL->getOutputMix();

                    SLDataSink dataSink;
                    dataSink.pLocator = &dataLocatorOut;
                    dataSink.pFormat = NULL;

                    const SLuint32 playerIIDCount = 4;
                    const SLInterfaceID playerIIDs[] = { SL_IID_BUFFERQUEUE, SL_IID_PLAY, SL_IID_VOLUME, SL_IID_RATEPITCH };
                    const SLboolean playerReqs[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_FALSE };

                    if ((*audioSL->getEngine())->CreateAudioPlayer(audioSL->getEngine(), &playerObject, &dataSource, &dataSink, playerIIDCount, playerIIDs, playerReqs) != SL_RESULT_SUCCESS)
                    {
                        Log(Log::Level::ERR) << "Failed to create OpenSL player object";
                        return false;
                    }

                    if ((*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS)
                    {
                        Log(Log::Level::ERR) << "Failed to create OpenSL player object";
                        return false;
                    }

                    if ((*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &player) != SL_RESULT_SUCCESS)
                    {
                        Log(Log::Level::ERR) << "Failed to get OpenSL player interface";
                        return false;
                    }

                    if ((*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &bufferQueue) != SL_RESULT_SUCCESS)
                    {
                        Log(Log::Level::ERR) << "Failed to get OpenSL buffer queue interface";
                        return false;
                    }

                    if ((*playerObject)->GetInterface(playerObject, SL_IID_VOLUME, &playerVolume) != SL_RESULT_SUCCESS)
                    {
                        Log(Log::Level::ERR) << "Failed to get OpenSL volume interface";
                        return false;
                    }

                    if ((*playerObject)->GetInterface(playerObject, SL_IID_RATEPITCH, &playerPitch) != SL_RESULT_SUCCESS)
                    {
                        Log(Log::Level::WARN) << "Failed to get OpenSL pitch interface";
                    }

                    if ((*bufferQueue)->Clear(bufferQueue) != SL_RESULT_SUCCESS)
                    {
                        Log(Log::Level::ERR) << "Failed to clear OpenSL buffer";
                        return false;
                    }

                    if ((*bufferQueue)->RegisterCallback(bufferQueue, playerCallback, this) != SL_RESULT_SUCCESS)
                    {
                        Log(Log::Level::ERR) << "Failed to register OpenSL buffer queue callback";
                        return false;
                    }

                    data = soundData->getData(stream.get(), BUFFER_SIZE);

                    if ((*bufferQueue)->Enqueue(bufferQueue, data.data(), data.size()) != SL_RESULT_SUCCESS)
                    {
                        Log(Log::Level::ERR) << "Failed to enqueue OpenSL data";
                        return false;
                    }

                    channels = soundData->getChannels();
                }
                else
                {
                    if (playerObject)
                    {
                        (*playerObject)->Destroy(playerObject);
                        playerObject = nullptr;
                        player = nullptr;
                        bufferQueue = nullptr;
                        playerVolume = nullptr;
                    }

                    channels = 0;
                }
            }

            if (dirty & DIRTY_PITCH)
            {
                if (playerPitch && soundData)
                {
                    SLpermille frequency = static_cast<SLpermille>(soundData->getSamplesPerSecond() * pitch);

                    if ((*playerPitch)->SetPitch(playerPitch, frequency) != SL_RESULT_SUCCESS)
                    {
                        Log(Log::Level::ERR) << "Failed to set pitch";
                        return false;
                    }
                }
            }

            if (dirty & DIRTY_GAIN)
            {
                if (playerVolume)
                {
                    if ((*playerVolume)->SetVolumeLevel(playerVolume, gain) != SL_RESULT_SUCCESS)
                    {
                        Log(Log::Level::ERR) << "Failed to set volume";
                        return false;
                    }
                }
            }

            if (dirty & DIRTY_PLAY_STATE)
            {
                if (player)
                {
                    if (shouldPlay)
                    {
                        if (reset)
                        {
                            if ((*player)->SetPlayState(player, SL_PLAYSTATE_STOPPED) != SL_RESULT_SUCCESS)
                            {
                                Log(Log::Level::ERR) << "Failed to stop sound";
                                return false;
                            }
                        }

                        if ((*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING) != SL_RESULT_SUCCESS)
                        {
                            Log(Log::Level::ERR) << "Failed to play sound";
                            return false;
                        }
                    }
                    else
                    {
                        if ((*player)->SetPlayState(player, reset ? SL_PLAYSTATE_STOPPED : SL_PLAYSTATE_PAUSED) != SL_RESULT_SUCCESS)
                        {
                            Log(Log::Level::ERR) << "Failed to " << (reset ? "stop" : "pause") << " sound";
                            return false;
                        }
                    }
                }

                reset = false;
            }

            dirty = 0;

            if (playerVolume && channels == 1)
            {
                // TODO: update position
                /*if ((*playerVolume)->SetStereoPosition(playerVolume, pan) != SL_RESULT_SUCCESS)
                {
                    Log(Log::Level::ERR) << "Failed to set pan";
                }*/
            }

            return true;
        }

        void SoundResourceSL::enqueue(SLAndroidSimpleBufferQueueItf bufferQueue)
        {
            if (stream)
            {
                data = soundData->getData(stream.get(), BUFFER_SIZE);

                // if stream has ended but we should repeat it
                if (repeat && data.empty())
                {
                    stream->reset();
                    data = soundData->getData(stream.get(), BUFFER_SIZE);
                }

                if (!data.empty())
                {
                    if ((*bufferQueue)->Enqueue(bufferQueue, data.data(), data.size()) != SL_RESULT_SUCCESS)
                    {
                        ouzel::Log(ouzel::Log::Level::ERR) << "Failed to enqueue OpenSL data";
                    }
                }
                else
                {
                    if ((*player)->SetPlayState(player, SL_PLAYSTATE_STOPPED) != SL_RESULT_SUCCESS)
                    {
                        ouzel::Log(ouzel::Log::Level::ERR) << "Failed to stop sound";
                    }
                }
            }
            else
            {
                if (repeat && !data.empty())
                {
                    if ((*bufferQueue)->Enqueue(bufferQueue, data.data(), data.size()) != SL_RESULT_SUCCESS)
                    {
                        ouzel::Log(ouzel::Log::Level::ERR) << "Failed to enqueue OpenSL data";
                    }
                }
                else
                {
                    if ((*player)->SetPlayState(player, SL_PLAYSTATE_STOPPED) != SL_RESULT_SUCCESS)
                    {
                        ouzel::Log(ouzel::Log::Level::ERR) << "Failed to stop sound";
                    }
                }
            }
        }
    } // namespace audio
} // namespace ouzel

#endif