#pragma once

class SampleBuffer {
public:
    unsigned long* buffer{NULL};
    unsigned numSamples{0};

    SampleBuffer() {}

    SampleBuffer(unsigned bufferSizeBytes) {
        init(bufferSizeBytes);
    }

    void init(unsigned bufferSizeBytes) {
        if (buffer != NULL) {
            delete[] buffer;
        }

        numSamples = bufferSizeBytes / sizeof(unsigned long);
        buffer = new unsigned long[numSamples];
    }

    void zero() {
        for (unsigned i = 0; i < numSamples; i++) {
            buffer[i] = 0;
        }
    }
};
