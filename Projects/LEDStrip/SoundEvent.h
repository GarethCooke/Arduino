#pragma once
#include <Arduino.h>

class SoundEvent
{
	friend class Initialiser;

public:
	unsigned int	getCount()						const { return 7; }
	const String& getFrequency(unsigned int nIndex)	const { return m_fequencies[assureIndex(nIndex)]; }
	int				getValue(unsigned int nIndex)	const { return m_results[assureIndex(nIndex)]; }

	class BandIterator
	{
	public:
		BandIterator(const SoundEvent& evt, unsigned int current = 0) : m_evt(evt), m_current(current) {}
		const BandIterator& operator++()					{ m_current++; return *this; }
		std::pair<const String*, int> operator*()			{ return std::make_pair(&m_evt.m_fequencies[m_current], m_evt.m_results[m_current]); }
		bool operator==(const BandIterator& right) const	{ return m_current == right.m_current && &m_evt == &right.m_evt; }
		bool operator!=(const BandIterator& right) const	{ return !(*this == right); }
	private:
		const SoundEvent&	m_evt;
		unsigned int		m_current;
	};

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

	BandIterator	begin()			const { return BandIterator(*this); }
	BandIterator	end()			const { return BandIterator(*this, getCount()); }
	bool			beatDetected()	const { return m_beatDetected; }

private:
	SoundEvent();

	unsigned int	assureIndex(unsigned int nIndex) const	{ return min(nIndex, getCount() - 1); }
	void			signalBeatDetected()					{ m_beatDetected = true; }

	static const String	m_fequencies[7];
	int					m_results[7];
	bool				m_beatDetected;
};
