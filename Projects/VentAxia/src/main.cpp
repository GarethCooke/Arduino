#include <Arduino.h>
#include <memory>
#include <BLEDevice.h>

using std::unique_ptr;

class IguanaService;

// static BLEUUID serviceUUID("e6834e4b-7b3a-48e6-91e4-f1d005f564d3");
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID("4cad343a-209a-40b7-b911-4d9b3df569b2");


BLERemoteCharacteristic* pRemoteCharacteristic;
unique_ptr<IguanaService> pIguanaService;


// Scan for BLE servers and find the first one that advertises the service we are looking for.
class IguanaService : public BLEAdvertisedDeviceCallbacks
{
public:
    // The remote service we wish to connect to.
    IguanaService(const char* serviceUUID) : m_serviceUUID(serviceUUID)
    {
        // Retrieve a Scanner and set the callback we want to use to be informed when we
        // have detected a new device.  Specify that we want active scanning and start the
        // scan to run for 30 seconds.
        BLEScan* pBLEScan = BLEDevice::getScan();
        pBLEScan->setAdvertisedDeviceCallbacks(this);
        pBLEScan->setActiveScan(true);
        pBLEScan->start(30);
    }

    // Called for each advertising BLE server.
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        Serial.print("BLE Advertised Device found: ");
        Serial.println(advertisedDevice.toString().c_str());

        // We have found a device, let us now see if it contains the service we are looking for.
        Serial.println("haveServiceUUID: ");
        Serial.println(advertisedDevice.haveServiceUUID());

        if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(m_serviceUUID))
        {
            Serial.print("Found our device!  address: ");
            advertisedDevice.getScan()->stop();
            m_pServerAddress = new BLEAddress(advertisedDevice.getAddress());
        } // Found our server
    }


    bool connectToServer()
    {
        if (!m_pServerAddress)
            return false;

        Serial.print("Forming a connection to ");
        Serial.println(m_pServerAddress->toString().c_str());

        BLEClient* pClient = BLEDevice::createClient();
        Serial.println(" - Created client");

        // Connect to the remove BLE Server.
        pClient->connect(*m_pServerAddress);
        Serial.println(" - Connected to server");

        // Obtain a reference to the service we are after in the remote BLE server.
        BLERemoteService* pRemoteService = pClient->getService(m_serviceUUID);
        std::map<std::string, BLERemoteService*>* pMap = pClient->getServices();
        for (std::pair<std::string, BLERemoteService*> element : *pMap) {
            Serial.print("Service: ");
            Serial.println(element.first.c_str());
            Serial.println(element.second->toString().c_str());
        }

        if (pRemoteService == nullptr)
        {
            Serial.print("Failed to find our service UUID: ");
            Serial.println(m_serviceUUID.toString().c_str());
            // m_pServerAddress = NULL;
            pClient->disconnect();
            return false;
        }
        Serial.println(" - Found our service");

        // Obtain a reference to the characteristic in the service of the remote BLE server.
        pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
        if (pRemoteCharacteristic == nullptr) {
            Serial.print("Failed to find our characteristic UUID: ");
            Serial.println(charUUID.toString().c_str());
            pClient->disconnect();
            return false;
        }
        Serial.println(" - Found our characteristic");

        // Read the value of the characteristic.
        std::string value = pRemoteCharacteristic->readValue();
        Serial.print("The characteristic value was: ");
        Serial.println(value.c_str());

        pRemoteCharacteristic->registerForNotify([](
            BLERemoteCharacteristic* pBLERemoteCharacteristic,
            uint8_t* pData,
            size_t length,
            bool isNotify) {
                Serial.print("Notify callback for characteristic ");
                Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
                Serial.print(" of data length ");
                Serial.println(length);
            });
        pClient->disconnect();
        return true;
    }

private:
    BLEUUID     m_serviceUUID;
    BLEAddress* m_pServerAddress = NULL;
};



void setup() {
    Serial.begin(115200);
    Serial.println("Starting Arduino BLE Client application...");
    BLEDevice::init("IguanaBoost");
    const char* serviceUUID = "0000180a-0000-1000-8000-00805f9b34fb"; // The remote service we wish to connect to.
    pIguanaService = unique_ptr<IguanaService>(new IguanaService(serviceUUID));
}


// This is the Arduino main loop function.
void loop() {

    // If the flag "doConnect" is true then we have scanned for and found the desired
    // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
    // connected we set the connected flag to be true.
    if (pIguanaService->connectToServer())
    {
        Serial.println("We are now connected to the BLE Server.");
        String newValue = "Time since boot: " + String(millis() / 1000);
        Serial.println("Setting new characteristic value to \"" + newValue + "\"");

        // Set the characteristic's value to be the array of bytes that is actually a string.
        pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
    }
    else
        Serial.println("We have failed to connect to the server; there is nothin more we will do.");

    delay(1000);
}