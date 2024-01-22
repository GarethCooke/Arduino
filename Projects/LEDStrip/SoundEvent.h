#pragma once
#include <Arduino.h>

class SoundEvent
{
	friend class Initialiser;
	static constexpr unsigned int BANDS = 7;

public:
	static constexpr unsigned int	getBands()									{ return BANDS; }
	const String&					getFrequency(unsigned int nIndex)	const	{ return m_fequencies[assureIndex(nIndex)]; }
	int								getValue(unsigned int nIndex)		const	{ return m_results[assureIndex(nIndex)]; }

	template <typename FN> void iterate_bands(FN func) const
	{
		for (unsigned int band = 0; band < getBands(); band++)
			func(m_fequencies[band], m_results[band]);
	}

	class Initialiser
	{
	public:
		Initialiser();
		~Initialiser();
		Initialiser& operator=(int value);
		operator const SoundEvent& () const { return *m_pEvent; }
		void signalBeatDetected() { m_pEvent->signalBeatDetected(); }

	private:
		SoundEvent*		m_pEvent;
		unsigned int	m_current;
	};

	class Listener
	{
	public:
		virtual void notify(const SoundEvent& evt) = 0;
	};

	bool beatDetected() const { return m_beatDetected; }

private:
	SoundEvent();

	unsigned int	assureIndex(unsigned int nIndex) const	{ return nIndex < getBands() ? nIndex : getBands() - 1; }
	void			signalBeatDetected()					{ m_beatDetected = true; }

	static const String	m_fequencies[BANDS];
	int					m_results[BANDS];
	bool				m_beatDetected;
};

