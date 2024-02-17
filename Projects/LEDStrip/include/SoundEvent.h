#pragma once
#include <Arduino.h>

class SoundEvent
{
	friend class Initialiser;
	friend class Output;

	static constexpr unsigned int BANDS = 7;

public:
	static constexpr unsigned int getBands()
	{
		return BANDS;
	}

	class Band
	{
		friend SoundEvent;

	private:
		struct History
		{
			unsigned int m_level[8] = {0}; // Values from the BANDS frequency bands
			unsigned int m_totalLevel = 0; // Summed values from the BANDS frequency bands

			// To store the average 'energy' over the last ~1 second
			unsigned int m_samples[64] = {0}; // Low-pass filtered samples
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

	class BandOutput
	{
		friend class SoundEvent;

	public:
		BandOutput(const char *frequency) : m_frequency(frequency) {}
		bool beatDetected() const { return m_beat; }

	private:
		const char *m_frequency;
		unsigned int m_value = 0; // Value the program will output for a channel
		bool m_beat = 0;
	};

	class Output
	{
		friend class SoundEvent;

	public:
		template <typename FN>
		void iterate_bands(FN func) const
		{
			for (unsigned int band = 0; band < getBands(); band++)
				func(m_bandoutput[band].m_frequency, m_bandoutput[band].m_value);
		}

		const char *getFrequency(unsigned int nIndex) const { return m_bandoutput[assureIndex(nIndex)].m_frequency; }
		int getValue(unsigned int nIndex) const { return m_bandoutput[assureIndex(nIndex)].m_value; }

		bool beatDetected() const { return m_beat; }

	private:
		BandOutput m_bandoutput[BANDS] = {"63Hz", "160Hz", "400Hz", "1000Hz", "2500Hz", "6250Hz", "16000Hz"};
		bool m_beat = 0; // added for efficiency - repeated data, could be derrived from the iterating the output items
	};

	class Listener
	{
	public:
		virtual void notify(const SoundEvent::Output &evt) = 0;
	};

	void recordResult(unsigned int idx_band, int value);
	const Output &output() const { return m_output; }

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

	Band m_bands[BANDS];
	Output m_output;
	// Indexes m_indexes;

	static unsigned int assureIndex(unsigned int nIndex) { return nIndex < getBands() ? nIndex : getBands() - 1; }
};
