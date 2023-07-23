#pragma once

#include <Audioclient.h>
#include <iostream>
#include <mmdeviceapi.h>

class WasapiClient {
public:
    IMMDeviceEnumerator* enumerator;
    IMMDevice* device;
    IAudioClient* audioClient;
    IAudioRenderClient* renderClient;
    WAVEFORMATEXTENSIBLE waveFormat;
    HANDLE hEvent;
    HANDLE hTask;

    WasapiClient();
    ~WasapiClient();
    unsigned getBufferSizeFrames();
    unsigned getBufferSizeBytes();
    unsigned getCurrentPadding();
    void writeBuffer(unsigned long* source, unsigned numFramesToWrite);
    void startPlaying();
    void stopPlaying();

private:
    void init();
    void destroy();
    void getEnumerator();
    void getDevice();
    void getAudioClient();
    void getDevicePeriod(REFERENCE_TIME& devicePeriod);
    void checkFormatSupport();
    void initAudioClient(REFERENCE_TIME minimumDevicePeriod);
    void getRenderClient();
    void initEvent();
    void initTask();
};
