#include <Wire.h>
#include "BeatWireComms.h"

BeatWireComms::BeatWireComms(TwoWire& wire) : m_wire(wire)
{
}
