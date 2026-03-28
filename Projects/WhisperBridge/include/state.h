#pragma once
// BLE state is now accessed via the Ble singleton in ble.h:
//   Ble.isRunning()    — true while a boost sequence is executing
//   Ble.lastSuccess()  — result of the most recent attempt
