#include "audio_service.hpp"

#include <cstdlib>

#include "audio_util.hpp"

AudioService::AudioService(
    WasapiClient& wasapiClient,
    moodycamel::ReaderWriterQueue<std::string>* queue
)
    : wasapiClient(wasapiClient), queue(queue)
{
    unsigned long samplesPerSecond = wasapiClient.waveFormat.Format.nSamplesPerSec;
    secondsPerSample = 1.0 / (double)samplesPerSecond;

    bufferSizeBytes = wasapiClient.getBufferSizeBytes();
    sampleBuffer.init(bufferSizeBytes);

    bufferSizeFrames = wasapiClient.getBufferSizeFrames();
}

void AudioService::run() {
    sampleBuffer.zero();

    wasapiClient.writeBuffer(sampleBuffer.buffer, bufferSizeFrames);

    wasapiClient.startPlaying();

    bool quit = false;
    std::string message;

    // main loop:
    while (!quit) {
        WaitForSingleObject(wasapiClient.hEvent, INFINITE);

        // handle events from main thread
        trig = false;
        while (!quit && queue->try_dequeue(message)) {
            if (message == "quit") {
                std::cout << "audio thread: " << message << std::endl;
                quit = true;
                break;
            }

            if (message == "trig") {
                trig = true;
            }
        }

        unsigned numPaddingFrames = wasapiClient.getCurrentPadding();

        // recall that each elt of buffer stores 1 sample
        // frame is 2 samples -> 1 for each channel
        // so numSamplesToWrite is 2x numFramesToWrite

        unsigned numFramesToWrite = bufferSizeFrames - numPaddingFrames;

        unsigned numSamplesToWrite = numFramesToWrite * 2;

        fillSampleBuffer(numSamplesToWrite);

        wasapiClient.writeBuffer(sampleBuffer.buffer, numFramesToWrite);
    }

    wasapiClient.stopPlaying();
}

void AudioService::fillSampleBuffer(size_t numSamplesToWrite) {
    unsigned numChannels = 2;

    for (int i = 0; i < numSamplesToWrite; i += numChannels) {

        double sig = getSample();

        unsigned samp = scaleSignal(sig);

        sampleBuffer.buffer[i] = samp;       // L
        sampleBuffer.buffer[i + 1] = samp;   // R

        sampleCounter++;
    }
}

double getRand() {
    return rand() / (RAND_MAX + 1.0);
}

double AudioService::getSample() {
    if (trig) {
        r = getRand();
    }

    double t = getTime();

    double w = twoPi * freq;

    double theta = sin(w * t * 0.5) * modEnv.get(trig, 1, 50, 200, t) * 8 * r;

    double sinSig = sin((w * t) + theta);

    double envSig = ampEnv.get(trig, 1, 200, 500, t);

    double sig = sinSig * envSig * 0.5;

    return sig;
}

double AudioService::getTime() {
    return double(sampleCounter) * secondsPerSample;
}
