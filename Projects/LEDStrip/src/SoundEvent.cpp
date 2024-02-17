// code inspired by https://github.com/D-Winker/Audio-Visual-MSGEQ7-.git

#include <algorithm>
#include "SoundEvent.h"

using std::begin;
using std::end;
using std::find_if;

void SoundEvent::recordResult(unsigned int idx_band, int value)
{
    Band &band = m_bands[idx_band];
    Band::History &bandHistory = band.m_history;
    Output::BandOutput &bandOutput = m_output.getBand(idx_band);

    // Update the m_samples and average of m_samples from the last second
    bandHistory.m_totalSamples -= bandHistory.m_samples[m_next]; // Clear out the old value
    bandHistory.m_samples[m_next] = value;
    bandHistory.m_totalSamples += bandHistory.m_samples[m_next]; // Add in the new value
    bandHistory.m_avgSample = bandHistory.m_totalSamples >> 6;   // Divide by 64

    value = value < m_leveltreshold ? 0 : value;

    // Check for beats
    if (bandHistory.m_nextAllowedBeat == static_cast<int>(m_next))
        bandHistory.m_nextAllowedBeat = -1;

    if ((bandHistory.m_nextAllowedBeat == -1) && (value > m_beatthreshold))
    {
        bandOutput.beatDetected(value > (bandHistory.m_avgSample * bandHistory.m_C));
        if (bandOutput.beatDetected())
            bandHistory.m_nextAllowedBeat = (m_next + 8) & 63; // don't allow another beat on this band for another 8 cycles

        // Calculate a new value for C using the variance of the measurements
        float var = 0;
        for (int sample = 0; sample < 64; sample++)
        {
            int temp = (bandHistory.m_samples[sample] - bandHistory.m_avgSample);
            var += (temp * temp) >> 6; // Divide by 64
        }
        bandHistory.m_C = (-0.00025714 * var) + 1.5142857; // Modified from the paper
    }
    else
        bandOutput.beatDetected(false);

    m_output.beatDetected(idx_band ? m_output.beatDetected() | bandOutput.beatDetected() : bandOutput.beatDetected()); // reset the output beat if this is the start of the next set of bands otherwise update it with the band beat

    // if (bandOutput.beatDetected())
    // {
    //     Serial.printf("beat, idx: %d,\tvalue: %d,\tave sample: %d,\tC: %f,\tave * C: %f, millis: %lu\n", idx_band, value, bandHistory.m_avgSample, bandHistory.m_C, bandHistory.m_avgSample * bandHistory.m_C, millis());
    // }

    if (bandOutput.getValue() < value)
    {
        if (band.m_beatTime < 5)
        {
            bandOutput.setValue(0.5 * bandOutput.getValue() + 0.5 * value);
        }
        else
        {
            bandOutput.setValue(value);
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
        bandOutput.setValue(band.m_fader * (0.8 * bandOutput.getValue() + (1 - 0.8) * value));
        bandOutput.setValue(band.m_fader * (0.8 * bandOutput.getValue() + (1 - 0.8) * value));
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

#ifdef ONE
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
        bandHistory.m_avgSample = bandHistory.m_totalSamples >> 6;                // Divide by 64

        // Check for beats
        bandOutput.beatDetected() = (bandHistory.m_samples[m_indexes.m_start] > 100) && bandHistory.m_samples[m_indexes.m_start] > (bandHistory.m_avgSample * bandHistory.m_C);
        // if (bandOutput.beatDetected())
        // {
        //     Serial.printf("beat %d, %d, %d, %d, %f, %f\n", idx_band, value, bandHistory.m_samples[m_indexes.m_start], bandHistory.m_avgSample, bandHistory.m_C, bandHistory.m_avgSample * bandHistory.m_C);
        // }

        if (bandOutput.getValue() < bandHistory.m_samples[m_indexes.m_start])
        {
            if (band.m_beatTime < 5)
            {
                bandOutput.setValue(0.5 * bandOutput.getValue() + 0.5 * bandHistory.m_samples[m_indexes.m_start]);
            }
            else
            {
                bandOutput.setValue(bandHistory.m_samples[m_indexes.m_start]);
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
            bandOutput.setValue(band.m_fader * (0.8 * bandOutput.getValue() + (1 - 0.8) * bandHistory.m_samples[m_indexes.m_start]));
            band.m_fader -= band.m_faderAdj;
            band.m_beatTime++;
            if (band.m_fader < 0)
            {
                band.m_fader = 0;
            }
        }

        // Calculate a new value for C using the variance of the measurements
        float var = 0;
        for (int sample = 0; sample < 64; sample++)
        {
            int temp = (bandHistory.m_samples[sample] - bandHistory.m_avgSample);
            var += (temp * temp) >> 6; // Divide by 64
        }
        bandHistory.m_C = (-0.00025714 * var) + 1.5142857; // Modified from the paper
    }
    else
        bandOutput.beatDetected(0);

    if (idx_band == SoundEvent::getBands() - 1)
    {
        if (++m_indexes.m_new > 7)
        {
            m_indexes.m_new = 0; // reset the index of the new value
            // Update the starting and ending indices for the bandHistory.m_samples from the last second
            m_indexes.m_start = (m_indexes.m_start + 1) & 63;
            m_indexes.m_end = (m_indexes.m_start + 1) & 63;
        }
        m_indexes.m_old = (m_indexes.m_new + 1) & 7;
    }
}
#endif
