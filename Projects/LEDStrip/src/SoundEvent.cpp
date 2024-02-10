// code inspired by and modified from https://github.com/D-Winker/Audio-Visual-MSGEQ7-.git

#include <algorithm>
#include "SoundEvent.h"

using std::begin;
using std::end;
using std::find_if;

bool SoundEvent::beatDetected() const
{
    return find_if(begin(m_bands), end(m_bands), [](const Band &band)
                   { return band.beatDetected(); }) != end(m_bands);
}

void SoundEvent::recordResult(unsigned int idx_band, int value)
{
    Band &band = m_bands[idx_band];
    Band::History &bandHistory = band.m_history;

    band.m_history.m_level[m_indexes.m_new] = value;
    bandHistory.m_totalLevel -= bandHistory.m_level[m_indexes.m_old]; // Clear out the old value
    bandHistory.m_totalLevel += bandHistory.m_level[m_indexes.m_new]; // Add in the new value

    if (m_indexes.m_new + 1 > 7)
    {
        // Update the m_samples and average of m_samples from the last second
        bandHistory.m_samples[m_indexes.m_start] = bandHistory.m_totalLevel >> 3; // Divide by 8
        bandHistory.m_totalSamples -= bandHistory.m_samples[m_indexes.m_end];     // Clear out the old value
        bandHistory.m_totalSamples += bandHistory.m_samples[m_indexes.m_start];   // Add in the new value
        bandHistory.m_avgSample = bandHistory.m_totalSamples >> 7;                // Divide by 128

        // Check for beats
        band.m_beat = bandHistory.m_samples[m_indexes.m_start] > (bandHistory.m_avgSample * bandHistory.m_C);
        if (band.m_beat)
        {
            Serial.printf("beat %d, %d, %d, %d, %f, %f\n", idx_band, value, bandHistory.m_samples[m_indexes.m_start], bandHistory.m_avgSample, bandHistory.m_C, bandHistory.m_avgSample * bandHistory.m_C);
        }

        if (band.m_output < bandHistory.m_samples[m_indexes.m_start])
        {
            if (band.m_beatTime < 5)
            {
                band.m_output = 0.5 * band.m_output + 0.5 * bandHistory.m_samples[m_indexes.m_start];
            }
            else
            {
                band.m_output = bandHistory.m_samples[m_indexes.m_start];
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
            band.m_output = band.m_fader * (0.8 * band.m_output + (1 - 0.8) * bandHistory.m_samples[m_indexes.m_start]);
            band.m_fader -= band.m_faderAdj;
            band.m_beatTime++;
            if (band.m_fader < 0)
            {
                band.m_fader = 0;
            }
        }

        // Calculate a new value for C using the variance of the measurements
        float var = 0;
        for (int sample = 0; sample < 128; sample++)
        {
            int temp = (bandHistory.m_samples[sample] - bandHistory.m_avgSample);
            var += (temp * temp) >> 7; // Divide by 128
        }
        bandHistory.m_C = (-0.00025714 * var) + 1.5142857; // Modified from the paper
    }
    else
        band.m_beat = 0;

    if (idx_band == SoundEvent::getBands() - 1)
    {
        if (++m_indexes.m_new > 7)
        {
            m_indexes.m_new = 0; // reset the index of the new value
            // Update the starting and ending indices for the bandHistory.m_samples from the last second
            m_indexes.m_start = (m_indexes.m_start + 1) & 127;
            m_indexes.m_end = (m_indexes.m_start + 1) & 127;
        }
        m_indexes.m_old = (m_indexes.m_new + 1) & 7;
    }
}
