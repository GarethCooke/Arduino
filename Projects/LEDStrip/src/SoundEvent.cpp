// code inspired by https://github.com/D-Winker/Audio-Visual-MSGEQ7-.git

#include <algorithm>
#include "SoundEvent.h"

using std::begin;
using std::end;
using std::find_if;

bool SoundEvent::beatDetected() const
{
    return find_if(begin(m_bands), end(m_bands), [](const Band &band)
                   { return band.beatDetected(); }) != end(m_bands);
    // return find_if(begin(m_dominantBand), end(m_dominantBand), [&](unsigned char idx_band)
    //                { return m_bands[idx_band].beatDetected(); }) != end(m_dominantBand);
}

void SoundEvent::recordResult(unsigned int idx_band, int value)
{
    Band &band = m_bands[idx_band];
    Band::History &bandHistory = band.m_history;

    // Update the m_samples and average of m_samples from the last second
    bandHistory.m_totalSamples -= bandHistory.m_samples[m_next]; // Clear out the old value
    bandHistory.m_samples[m_next] = value;
    bandHistory.m_totalSamples += bandHistory.m_samples[m_next]; // Add in the new value
    bandHistory.m_avgSample = bandHistory.m_totalSamples >> 6;   // Divide by 64

    // for (int dominant = 0; dominant < 2; dominant++)
    // {
    //     if (m_dominantBand[dominant] == idx_band)
    //     {
    //         m_dominantAve[dominant] = bandHistory.m_avgSample;
    //         break;
    //     }
    // }
    // if (m_dominantAve[0] < bandHistory.m_avgSample)
    // {
    //     m_dominantBand[0] = idx_band;
    //     m_dominantAve[0] = bandHistory.m_avgSample;
    // }
    // else if ((m_dominantAve[1] < bandHistory.m_avgSample) && m_dominantBand[0] != idx_band)
    // {
    //     m_dominantBand[1] = idx_band;
    //     m_dominantAve[1] = bandHistory.m_avgSample;
    // }

    // static unsigned long seq = 0;
    // Serial.printf("%lu,%d,%d,%d,%d,%f,%f\n", seq++, idx_band, value, bandHistory.m_totalSamples, bandHistory.m_avgSample, bandHistory.m_C, bandHistory.m_avgSample * bandHistory.m_C);

    // Check for beats
    if (bandHistory.m_nextAllowedBeat == static_cast<int>(m_next))
        bandHistory.m_nextAllowedBeat = -1;

    if ((bandHistory.m_nextAllowedBeat == -1) && (value > m_threshold))
    {
        band.m_beat = value > (bandHistory.m_avgSample * bandHistory.m_C);
        if (band.m_beat)
            bandHistory.m_nextAllowedBeat = (m_next + 16) & 63; // don't allow another beat on this band for another 16 cycles

        // Calculate a new value for C using the variance of the measurements
        float var = 0;
        for (int sample = 0; sample < 64; sample++)
        {
            int temp = (bandHistory.m_samples[sample] - bandHistory.m_avgSample);
            var += (temp * temp) >> 6; // Divide by 64
        }
        bandHistory.m_C = (-0.00025714 * var) + 1.5142857; // Modified from the paper        band.m_beat = value > (bandHistory.m_avgSample * bandHistory.m_C);
    }
    else
        band.m_beat = false;

    // if (band.m_beat)
    // {
    //     Serial.printf("beat, idx: %d,\tvalue: %d,\tave sample: %d,\tC: %f,\tave * C: %f, millis: %lu\n", idx_band, value, bandHistory.m_avgSample, bandHistory.m_C, bandHistory.m_avgSample * bandHistory.m_C, millis());
    // }

    if (band.m_output < value)
    {
        if (band.m_beatTime < 5)
        {
            band.m_output = 0.5 * band.m_output + 0.5 * value;
        }
        else
        {
            band.m_output = value;
        }
        band.m_fader = 1;
        if (band.m_beatTime != 0)
        {
            band.m_faderAdj = band.m_faderAdj * 0.75 + (1 - 0.75) * (1 / band.m_beatTime);
            if (band.m_beatTime < 10)
            {
                band.m_faderAdj = 0.25;
                if (band.m_beatTime < 5)
                {
                    band.m_faderAdj = 0;
                }
            }
            if (band.m_faderAdj < 0.01)
            {
                band.m_faderAdj = 0.01;
            }
        }
        band.m_beatTime = 0;
    }
    else
    {
        band.m_output = band.m_fader * (0.8 * band.m_output + (1 - 0.8) * value);
        band.m_output = band.m_fader * (0.8 * band.m_output + (1 - 0.8) * value);
        band.m_fader -= band.m_faderAdj;
        band.m_beatTime++;
        if (band.m_fader < 0)
        {
            band.m_fader = 0;
        }
    }

    if (idx_band == SoundEvent::getBands() - 1)
    {
        // Update the starting and ending indices for the bandHistory.m_samples from the last second
        m_next = (m_next + 1) & 63;

        // if (m_next == 0)
        // {
        //     static unsigned long last = millis();
        //     Serial.printf("time taken %lu\n", millis() - last);
        //     last = millis();
        // }
    }
}
