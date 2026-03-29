#include "ble.h"
#include "config.h"
#include <NimBLEDevice.h>

BleBoost Ble;

// ── BleBoost ──────────────────────────────────────────────────────────────────

void BleBoost::cleanup(NimBLEClient* client) {
    client->disconnect();
    NimBLEDevice::deleteClient(client);
}

void BleBoost::abandon(NimBLEClient* client, const char* verb, const char* subject) {
    Serial.printf("[BLE] %s %s\n", verb, subject);
    cleanup(client);
}

NimBLERemoteCharacteristic* BleBoost::getChar(
        NimBLEClient* client,
        const char* svcUuid, const char* charUuid,
        const char* svcLabel, const char* charLabel) {
    auto fail = [&](const char* label) -> NimBLERemoteCharacteristic* {
        abandon(client, label, "not found");
        return nullptr;
    };

    NimBLERemoteService* svc = client->getService(svcUuid);
    if (!svc) return fail(svcLabel);

    NimBLERemoteCharacteristic* ch = svc->getCharacteristic(charUuid);
    if (!ch) return fail(charLabel);
    return ch;
}

bool BleBoost::writeChar(NimBLEClient* client,
                         NimBLERemoteCharacteristic* ch,
                         const uint8_t* data, size_t len,
                         const char* label) {
    if (!ch->writeValue(data, len, true)) {
        abandon(client, "Write failed:", label);
        return false;
    }
    Serial.printf("[BLE] Written: %s\n", label);
    return true;
}

bool BleBoost::runSequence() {
    NimBLEClient* client = NimBLEDevice::createClient();

    Serial.println("[BLE] Connecting...");
    if (!client->connect(NimBLEAddress(FAN_MAC_ADDRESS))) {
        Serial.println("[BLE] Connection failed");
        NimBLEDevice::deleteClient(client);
        return false;
    }
    Serial.println("[BLE] Connected");

    NimBLERemoteCharacteristic* authChar = getChar(client,
        BLE_AUTH_SERVICE_UUID, BLE_AUTH_CHAR_UUID,
        "Auth service", "Auth characteristic");
    if (!authChar) return false;

    uint8_t pin[] = BLE_PIN_BYTES;
    if (!writeChar(client, authChar, pin, sizeof(pin), "PIN")) return false;
    vTaskDelay(pdMS_TO_TICKS(200));  // brief settle after auth

    NimBLERemoteCharacteristic* cmdChar = getChar(client,
        BLE_CMD_SERVICE_UUID, BLE_CMD_CHAR_UUID,
        "Command service", "Command characteristic");
    if (!cmdChar) return false;

    uint8_t boost[] = BLE_BOOST_BYTES;
    if (!writeChar(client, cmdChar, boost, sizeof(boost), "Boost command")) return false;

    cleanup(client);
    return true;
}

void BleBoost::runTask() {
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        _running     = true;
        _lastSuccess = runSequence();
        _running     = false;
        Serial.printf("[BLE] Sequence %s\n", _lastSuccess ? "succeeded" : "FAILED");
    }
}

void BleBoost::taskEntry(void* param) {
    static_cast<BleBoost*>(param)->runTask();
}

void BleBoost::setup() {
    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    xTaskCreate(taskEntry, "ble_boost", 8192, this, 5,
                reinterpret_cast<TaskHandle_t*>(&_task));
    Serial.println("[BLE] Ready");
}

void BleBoost::trigger() {
    if (!_task || _running) return;
    xTaskNotifyGive(static_cast<TaskHandle_t>(_task));
}
