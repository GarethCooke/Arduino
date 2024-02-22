#pragma once

#include <BeatWireComms.h>

class BeatWireReceiver : public BeatWireComms, public BeatReceiveImpl
{
public:
    static void create(TwoWire& wire, int sdaPin, int sclPin);
    static BeatWireReceiver& get();

protected:
    virtual void read(int bytes);
    virtual void flush();

private:
    static BeatWireReceiver* m_pWireReceiver;
    BeatWireReceiver(TwoWire& wire, int sdaPin, int sclPin);

    static void onReceive(int bytes);
};