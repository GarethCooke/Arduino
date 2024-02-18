#include <set>
#include "BeatBroadcast.h"

using std::set;

class TwoWire;

class BeatReceiver
{
public:
    static void create(TwoWire& wire, int sdaPin, int sclPin);
    static BeatReceiver& get();


    void addListener(MSGEQ7Out::Listener* pListener);
    void removeListener(MSGEQ7Out::Listener* pListener);

private:
    static constexpr const int m_address = BeatBroadcast::address();
    TwoWire& m_wire;
    std::set<MSGEQ7Out::Listener*> m_listeners;

    BeatReceiver(TwoWire& wire, int sdaPin, int sclPin);

    static void onReceive(int bytes);
    void receive(int bytes);
};
