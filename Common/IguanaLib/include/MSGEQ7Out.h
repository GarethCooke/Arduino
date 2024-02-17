#pragma once

class MSGEQ7Out
{
public:
    static constexpr unsigned int BANDS = 7;

    class Listener
    {
    public:
        virtual void notify(const MSGEQ7Out &evt) = 0;
    };

    class BandOutput
    {
        friend class MSGEQ7Out;

    public:
        BandOutput(const char *frequency) : m_frequency(frequency) {}
        const char *getFrequency() const { return m_frequency; }
        int getValue() const { return m_value; }
        bool beatDetected() const { return m_beat; }

        void setFrequency(const char *frequency) { m_frequency = frequency; }
        void setValue(int value) { m_value = value; }
        void beatDetected(bool detected) { m_beat = detected; }

    private:
        const char *m_frequency;
        unsigned int m_value = 0; // Value the program will output for a channel
        bool m_beat = 0;
    };

    static constexpr unsigned int getBands()
    {
        return BANDS;
    }

    template <typename FN>
    void iterate_bands(FN func) const
    {
        for (unsigned int band = 0; band < getBands(); band++)
            func(m_bandoutput[band].m_frequency, m_bandoutput[band].m_value);
    }

    const char *getFrequency(unsigned int nIndex) const { return m_bandoutput[assureIndex(nIndex)].m_frequency; }
    int getValue(unsigned int nIndex) const { return m_bandoutput[assureIndex(nIndex)].m_value; }
    BandOutput &getBand(unsigned int nIndex) { return m_bandoutput[nIndex]; }

    bool beatDetected() const { return m_beat; }
    void beatDetected(bool detected) { m_beat = detected; }

private:
    BandOutput m_bandoutput[BANDS] = {"63Hz", "160Hz", "400Hz", "1000Hz", "2500Hz", "6250Hz", "16000Hz"};
    bool m_beat = 0; // added for efficiency - repeated data, could be derrived from the iterating the output items

    static unsigned int assureIndex(unsigned int nIndex) { return nIndex < getBands() ? nIndex : getBands() - 1; }
};
