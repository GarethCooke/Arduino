#include "BeatBroadcast.h"

class TwoWire;

class BeatReceiver
{
public:
    BeatReceiver(TwoWire &wire, int sdaPin, int sclPin);
    virtual ~BeatReceiver();

private:
    static constexpr const int m_address = BeatBroadcast::address();
    TwoWire &m_wire;

    static void receive(int bytes);
};
