#pragma once

#include "../lib/readerwriterqueue.h"

#include "sample_buffer.hpp"
#include "ugens.hpp"
#include "wasapi_client.hpp"

class AudioService {
public:
    AudioService(
        WasapiClient& wasapiClient,
        moodycamel::ReaderWriterQueue<std::string>* queue
    );
    void run();
    void fillSampleBuffer(size_t numSamplesToWrite);
    double getSample();
    double getTime();

private:
    WasapiClient& wasapiClient;
    SampleBuffer sampleBuffer;
    unsigned bufferSizeBytes{0};
    unsigned bufferSizeFrames{0};

    unsigned long sampleCounter{0};
    double secondsPerSample{0.0};
    moodycamel::ReaderWriterQueue<std::string>* queue;

    // ugens
    Env env;
    AHREnv ampEnv;
    AHREnv modEnv;
    double freq{120.0};
    double r{0.0};
    bool trig{false};
};
