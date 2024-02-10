#pragma once
#include <Arduino.h>

class SoundEvent
{
	friend class Initialiser;
	static constexpr unsigned int BANDS = 7;

public:
	static constexpr unsigned int getBands() { return BANDS; }
	const String &getFrequency(unsigned int nIndex) const { return m_bands[assureIndex(nIndex)].m_frequency; }
	int getValue(unsigned int nIndex) const { return m_bands[assureIndex(nIndex)].m_output; }

	bool beatDetected() const;
	void recordResult(unsigned int idx_band, int value);

	template <typename FN>
	void iterate_bands(FN func) const
	{
		for (unsigned int band = 0; band < getBands(); band++)
			func(m_bands[band].m_frequency, m_bands[band].m_output);
	}

	class Band
	{
		friend SoundEvent;

	public:
		Band(const char *frequency)
			: m_frequency(frequency)
		{
		}

		bool beatDetected() const { return m_beat; }

	private:
		struct History
		{
			unsigned int m_level[8] = {0}; // Values from the BANDS frequency bands
			unsigned int m_totalLevel = 0; // Summed values from the BANDS frequency bands

			// To store the average 'energy' over the last ~1 second
			unsigned int m_samples[128] = {0}; // Low-pass filtered samples
			unsigned int m_totalSamples = 0;   // Sums of 128 samples
			unsigned int m_avgSample = 0;	   // Averages of the last 128 samples
			float m_C = 1.3;				   // Multiplier for the 'beat threshold'
		};

		const String m_frequency;
		bool m_beat = 0;
		unsigned int m_output = 0; // Value the program will output for a channel
		float m_fader = 1;		   // Fade out if there hasn't been a beat in a while
		float m_faderAdj = 0.1;	   // The rate at which the fader approaches zero
		int m_beatTime = 0;		   // A count of the time between beats. Used to determine faderAdj
		History m_history;
	};

	class Listener
	{
	public:
		virtual void notify(const SoundEvent &evt) = 0;
	};

private:
	struct Indexes
	{
		unsigned char m_old = 1;   // Oldest measurement in the array
		unsigned char m_new = 0;   // New messurement
		unsigned char m_end = 1;   // Oldest sample in the array
		unsigned char m_start = 0; // New sample
	};

	Band m_bands[BANDS] = {"63Hz", "160Hz", "400Hz", "1000Hz", "2500Hz", "6250Hz", "16000Hz"};
	Indexes m_indexes;

	unsigned int assureIndex(unsigned int nIndex) const { return nIndex < getBands() ? nIndex : getBands() - 1; }
};
