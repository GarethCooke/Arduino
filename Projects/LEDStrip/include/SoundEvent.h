#pragma once
#include <Arduino.h>
#include <MSGEQ7Out.h>

class SoundEvent
{
	friend class Initialiser;

public:
	static constexpr const unsigned int getBands()
	{
		return MSGEQ7Out::getBands();
	}

	class Band
	{
		friend SoundEvent;

	private:
		struct History
		{
			unsigned int m_level[8] = { 0 }; // Values from the BANDS frequency bands
			unsigned int m_totalLevel = 0; // Summed values from the BANDS frequency bands

			// To store the average 'energy' over the last ~1 second
			unsigned int m_samples[64] = { 0 }; // Low-pass filtered samples
			unsigned int m_totalSamples = 0;  // Sums of 64 samples
			unsigned int m_avgSample = 0;	  // Averages of the last 64 samples
			float m_C = 1.3;				  // Multiplier for the 'beat threshold'
			int m_nextAllowedBeat = -1;		  // next allowed beat
		};

		float m_fader = 1;		// Fade out if there hasn't been a beat in a while
		float m_faderAdj = 0.1; // The rate at which the fader approaches zero
		int m_beatTime = 0;		// A count of the time between beats. Used to determine faderAdj
		History m_history;
	};

	void recordResult(unsigned int idx_band, int value);
	const MSGEQ7Out& output() const { return m_output; }

private:
	// struct Indexes
	// {
	// 	unsigned char m_old = 1;   // Oldest measurement in the array
	// 	unsigned char m_new = 0;   // New messurement
	// 	unsigned char m_end = 1;   // Oldest sample in the array
	// 	unsigned char m_start = 0; // New sample
	// };

	unsigned char m_next = 0;			 // next sample
	unsigned char m_leveltreshold = 40;	 // minumum sound level, to remove noise at low levels
	unsigned char m_beatthreshold = 150; // minimum beat threashold, to remove effect of noise at low levels
	// unsigned char m_dominantBand[2] = {0}; // the dominant band
	// unsigned int m_dominantAve[2] = {0};   // the dominant band's average sample

	Band m_bands[MSGEQ7Out::BANDS];
	MSGEQ7Out m_output;
	// Indexes m_indexes;
};
