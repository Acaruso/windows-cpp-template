#include "wasapi_client.hpp"

#include <Avrt.h>
#include <iostream>
#include <string>
#pragma comment(lib, "Avrt")

#include "audio_constants.hpp"
#include "audio_util.hpp"

WasapiClient::WasapiClient() {
    init();
}

WasapiClient::~WasapiClient() {
    destroy();
}

unsigned WasapiClient::getBufferSizeFrames() {
    unsigned bufferSize;
    HRESULT hr = this->audioClient->GetBufferSize(&bufferSize);

    if (FAILED(hr)) {
        throw std::runtime_error("ERROR " + std::to_string(hr) + ": GetBufferSize");
    }

    return bufferSize;
}

unsigned WasapiClient::getBufferSizeBytes() {
    unsigned bufferSizeFrames = this->getBufferSizeFrames();

    unsigned bufferSizeBytes = (
        (
            bufferSizeFrames
            * this->waveFormat.Format.nChannels
            * this->waveFormat.Format.wBitsPerSample
        ) / 8
    );

    return bufferSizeBytes;
}

void WasapiClient::writeBuffer(unsigned long* source, unsigned numFramesToWrite) {
    HRESULT hr;
    BYTE *dest = NULL;

    unsigned bufferSizeFrames = this->getBufferSizeFrames();

    unsigned bufferSizeBytes = this->getBufferSizeBytes();

    // after this call, dest will point to location in buffer to write to
    hr = this->renderClient->GetBuffer(numFramesToWrite, &dest);

    if (FAILED(hr)) {
        throw std::runtime_error("ERROR " + std::to_string(hr) + ": GetBuffer");
    }

    memcpy(
        dest,
        source,
        sizeof(unsigned long) * 2 * numFramesToWrite
    );

    hr = this->renderClient->ReleaseBuffer(numFramesToWrite, 0);

    if (FAILED(hr)) {
        throw std::runtime_error("ERROR " + std::to_string(hr) + ": ReleaseBuffer");
    }
}

unsigned WasapiClient::getCurrentPadding() {
    unsigned numPaddingFrames = 0;
    HRESULT hr = this->audioClient->GetCurrentPadding(&numPaddingFrames);

    if (FAILED(hr)) {
        throw std::runtime_error("ERROR " + std::to_string(hr) + ": GetCurrentPadding");
    }

    return numPaddingFrames;
}

void WasapiClient::startPlaying() {
    HRESULT hr = this->audioClient->Start();

    if (FAILED(hr)) {
        throw std::runtime_error("ERROR " + std::to_string(hr) + ": Start");
    }
}

void WasapiClient::stopPlaying() {
    HRESULT hr = this->audioClient->Stop();

    if (FAILED(hr)) {
        throw std::runtime_error("ERROR " + std::to_string(hr) + ": Stop");
    }
}

// init and destroy ////////////////////////////////////////////////////////////////

REFERENCE_TIME REFTIMES_PER_SEC = 10000000;

WAVEFORMATEXTENSIBLE getWaveFormat() {
    WAVEFORMATEXTENSIBLE w = {};
    w.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    w.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    w.Format.cbSize = 22;
    w.Format.nChannels = 2;
    w.Format.nSamplesPerSec = samplesPerSec;
    w.Format.wBitsPerSample = 32;
    w.Samples.wValidBitsPerSample = 24;
    w.Format.nBlockAlign = (w.Format.nChannels * w.Format.wBitsPerSample) / 8;
    w.Format.nAvgBytesPerSec = w.Format.nSamplesPerSec * w.Format.nBlockAlign;
    return w;
}

void WasapiClient::init() {
    getEnumerator();
    getDevice();
    getAudioClient();
    waveFormat = getWaveFormat();
    checkFormatSupport();
    REFERENCE_TIME minimumDevicePeriod = 0;
    getDevicePeriod(minimumDevicePeriod);
    initAudioClient(minimumDevicePeriod);
    getRenderClient();
    initEvent();
    initTask();
}

void WasapiClient::destroy() {
    std::cout << "destroying wasapi client" << std::endl;

    if (enumerator != NULL) {
        enumerator->Release();
    }

    if (device != NULL) {
        device->Release();
    }

    if (audioClient != NULL) {
        audioClient->Release();
    }

    if (renderClient != NULL) {
        renderClient->Release();
    }

    if (hEvent != NULL) {
        CloseHandle(hEvent);
    }

    if (hTask != NULL) {
        AvRevertMmThreadCharacteristics(hTask);
    }
}

void WasapiClient::getEnumerator() {
    CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

    IMMDeviceEnumerator* enumerator = NULL;

    HRESULT hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL,
        IID_IMMDeviceEnumerator,
        (void**)&enumerator
    );

    if (FAILED(hr)) {
        throw std::runtime_error("ERROR " + to_hex(hr) + ": getEnumerator");
    } else {
        std::cout << "SUCCESS: getEnumerator" << std::endl;
    }

    this->enumerator = enumerator;
}

void WasapiClient::getDevice() {
    IMMDevice* device = NULL;

    HRESULT hr = enumerator->GetDefaultAudioEndpoint(
        eRender,
        eConsole,
        &device
    );

    if (FAILED(hr)) {
        throw std::runtime_error("ERROR " + to_hex(hr) + ": getDevice");
    } else {
        std::cout << "SUCCESS: getDevice" << std::endl;
    }

    this->device = device;
}

void WasapiClient::getAudioClient() {
    IAudioClient* client = NULL;
    IID IID_IAudioClient = __uuidof(IAudioClient);

    HRESULT hr = device->Activate(
        IID_IAudioClient,
        CLSCTX_ALL,
        NULL,
        (void**)&client
    );

    if (FAILED(hr)) {
        throw std::runtime_error("ERROR " + to_hex(hr) + ": getAudioClient");
    } else {
        std::cout << "SUCCESS: getAudioClient" << std::endl;
    }

    audioClient = client;
}

void WasapiClient::getDevicePeriod(REFERENCE_TIME& devicePeriod) {
    REFERENCE_TIME defaultDevicePeriod = 0;
    REFERENCE_TIME minimumDevicePeriod = 0;

    HRESULT hr = audioClient->GetDevicePeriod(
        &defaultDevicePeriod,
        &minimumDevicePeriod
    );

    if (FAILED(hr)) {
        throw std::runtime_error("ERROR " + to_hex(hr) + ": getDevicePeriod");
    } else {
        std::cout << "SUCCESS: getDevicePeriod" << std::endl;
    }

    devicePeriod = minimumDevicePeriod;
}

void WasapiClient::checkFormatSupport() {
    WAVEFORMATEX match = {0};
    WAVEFORMATEX* p_match = &match;

    HRESULT hr = audioClient->IsFormatSupported(
        AUDCLNT_SHAREMODE_SHARED,
        &waveFormat.Format,
        &p_match
    );

    if (FAILED(hr)) {
        throw std::runtime_error("ERROR " + to_hex(hr) + ": checkFormatSupport");
    } else {
        std::cout << "SUCCESS: checkFormatSupport" << std::endl;
    }
}

void WasapiClient::initAudioClient(REFERENCE_TIME minimumDevicePeriod) {
    std::cout << "minimumDevicePeriod: " << minimumDevicePeriod << std::endl;

    HRESULT hr = audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        minimumDevicePeriod,
        0,
        &waveFormat.Format,
        NULL
    );

    if (FAILED(hr)) {
        throw std::runtime_error("ERROR " + to_hex(hr) + ": initAudioClient");
    } else {
        std::cout << "SUCCESS: initAudioClient" << std::endl;
    }
}

void WasapiClient::getRenderClient() {
    IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
    IAudioRenderClient *renderClient = NULL;

    HRESULT hr = audioClient->GetService(
        IID_IAudioRenderClient,
        (void**)&renderClient
    );

    if (FAILED(hr)) {
        throw std::runtime_error("ERROR " + to_hex(hr) + ": getRenderClient");
    } else {
        std::cout << "SUCCESS: getRenderClient" << std::endl;
    }

    this->renderClient = renderClient;
}

void WasapiClient::initEvent() {
    HANDLE hEvent = CreateEvent(nullptr, false, false, nullptr);

    if (hEvent == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("ERROR: CreateEvent");
    }

    HRESULT hr = audioClient->SetEventHandle(hEvent);

    if (FAILED(hr)) {
        throw std::runtime_error("ERROR " + to_hex(hr) + ": initEvent");
    } else {
        std::cout << "SUCCESS: initEvent" << std::endl;
    }

    this->hEvent = hEvent;
}

void WasapiClient::initTask() {
    // increase thread priority
    unsigned long taskIndex = 0;
    hTask = AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);

    if (hTask == NULL) {
        throw std::runtime_error("ERROR: initTask");
    } else {
        std::cout << "SUCCESS: initTask" << std::endl;
    }
}
