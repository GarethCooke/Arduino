#pragma once

#include <BeatWireComms.h>

class BeatWireReceiver : public BeatWireComms, public BeatReceiveImpl
{
public:
    static void create(TwoWire& wire, int sdaPin, int sclPin);
    static BeatWireReceiver& get();

private:
    static BeatWireReceiver* m_pWireReceiver;
    BeatWireReceiver(TwoWire& wire, int sdaPin, int sclPin);

    static void onReceive(int bytes);
    void receive(int bytes);
};