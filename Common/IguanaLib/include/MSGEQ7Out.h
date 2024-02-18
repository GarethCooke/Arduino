#pragma once

class MSGEQ7Out
{
    // class has no non-static pointers - can be mem copied
public:
    static constexpr unsigned int BANDS = 7;

    class Listener
    {
    public:
        virtual void notify(const MSGEQ7Out& evt) = 0;
    };

    class BandOutput
    {
        friend class MSGEQ7Out;

    public:
        uint8_t getValue() const { return m_value; }
        bool beatDetected() const { return m_beat; }

        void setValue(int value) { m_value = value; }
        void beatDetected(bool detected) { m_beat = detected; }

    private:
        uint8_t m_value = 0; // Value the program will output for a channel
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
            func(m_frequencies[band], m_bandoutput[band].m_value, m_bandoutput[band].m_beat);
    }

    const char* getFrequency(unsigned int nIndex) const { return m_frequencies[assureIndex(nIndex)]; }
    uint8_t getValue(unsigned int nIndex) const { return m_bandoutput[assureIndex(nIndex)].m_value; }
    BandOutput& getBand(unsigned int nIndex) { return m_bandoutput[nIndex]; }

    bool beatDetected() const { return m_beat; }
    void beatDetected(bool detected) { m_beat = detected; }

    static constexpr const size_t bytes() { return sizeof(MSGEQ7Out); }
    const uint8_t* toBytes() const { return reinterpret_cast<const uint8_t*>(this); }
    void fromyBytes(const uint8_t* src);

private:
    static const char* m_frequencies[BANDS];

    BandOutput m_bandoutput[BANDS];
    bool m_beat = 0; // added for efficiency - repeated data, could be derrived from the iterating the output items

    static unsigned int assureIndex(unsigned int nIndex) { return nIndex < getBands() ? nIndex : getBands() - 1; }
};
