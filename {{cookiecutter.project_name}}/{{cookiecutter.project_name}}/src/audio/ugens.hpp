#pragma once

#include "audio_util.hpp"

const double twoPi = 3.14159265359 * 2;

struct Env {
    unsigned timer = 0;
    bool on = false;

    double get(bool trig, double holdTime, double t) {
        double out = 0.0;
        unsigned holdTimeSamps = mstosamps(holdTime);

        if (trig) {
            on = true;
            timer = 0;
        }

        if (timer < holdTimeSamps) {
            out = 1.0;
        }

        timer += 1;

        return out;
    }
};

struct AHREnv {
    unsigned timer = 0;
    bool on = false;
    double sig;

    double get(bool trig, double a, double h, double r, double t) {
        unsigned attackSamps = mstosamps(a);
        unsigned holdSamps = mstosamps(h);
        unsigned releaseSamps = mstosamps(r);

        double attackDelta = 1.0 / (double)attackSamps;
        double releaseDelta = 1.0 / (double)releaseSamps;

        if (trig) {
            sig = 0.0;
            timer = 0;
        }

        if (timer < attackSamps) {
            sig += attackDelta;
        } else if (timer < attackSamps + holdSamps) {
            sig = 1.0;
        } else if(timer < attackSamps + holdSamps + releaseSamps) {
            sig -= releaseDelta;
        } else {
            sig = 0.0;
        }

        timer += 1;

        return sig;
    }
};
