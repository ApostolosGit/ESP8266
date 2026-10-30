// GENERATED v7.0: CommonPlatform V1.05 + preserved application declarations/implementation from EnergyMeter v6.01
// ESP8266_common_platform - central editable shared firmware.
// Application generation inserts only the selected application's declarations,
// implementation and profile at the marked integration points below.
// Baseline: EnergyMeter 5.58 behavior, cleaned source commit 0c874d3 (5.59).
// BEGIN PLATFORM PROFILE
#define PLATFORM_CUSTOM_APPLICATION 1
#define PLATFORM_NAME "ESP8266_common_platform"
#define PLATFORM_VERSION "1.05"
#define VERSION "7.0"
#define BUILD_DATE "30/10/2026"
#define DEVICE_LABEL "EnergyMeter"
#define APPLICATION_ID "energymeter"
#define METER_TYPE 2 // 1: DDS238; 2: JSY-MK333
#if METER_TYPE == 1
#define DEVICE_ID "DDS"
#define DEVICE_KIND "DDS238"
#define DRIVER_ID "DDS238"
#define OTA_PROFILE_PREFIX "energymeter-dds238"
#elif METER_TYPE == 2
#define DEVICE_ID "jsy_house"
#define DEVICE_KIND "JSY-MK-333"
#define DRIVER_ID "JSY-MK333"
#define OTA_PROFILE_PREFIX "energymeter-jsy-mk333"
#else
#error "Unsupported EnergyMeter driver"
#endif
#define MQTT_TOPIC_PREFIX "home/energy/"
#define OLED_TYPE 2
#define APP_SAMPLE_INTERVAL_MS 3000UL
// END PLATFORM PROFILE

// Actuator applications opt into shared lifecycle hooks.
#ifndef PLATFORM_LOCAL_CONTROL_APPLICATION
#define PLATFORM_LOCAL_CONTROL_APPLICATION 0
#endif
#ifndef PLATFORM_FLASH_BUTTON_ENABLED
#define PLATFORM_FLASH_BUTTON_ENABLED 1
#endif
#ifndef PLATFORM_SENSOR_ERROR
#define PLATFORM_SENSOR_ERROR "modbus_read_failed"
#endif
// Applications may require more room for their own content buffers.
#ifndef PLATFORM_WEB_MIN_HEAP_BYTES
#define PLATFORM_WEB_MIN_HEAP_BYTES 1024UL
#endif
#ifndef PLATFORM_WEB_MIN_BLOCK_BYTES
#define PLATFORM_WEB_MIN_BLOCK_BYTES 256U
#endif

// Build identity describes compiled capabilities; it does not probe the display.
#if OLED_TYPE == 2
#define OLED_ID "SSD1309"
#define OTA_PROFILE_ID OTA_PROFILE_PREFIX "-ssd1309"
#elif OLED_TYPE == 0
#define OLED_ID "NONE"
#define OTA_PROFILE_ID OTA_PROFILE_PREFIX "-none"
#else
#error "Unsupported OTA display profile"
#endif
#define OTA_PROTOCOL_VERSION 2

#define USE_STATIC_IP 1
#define SITE_IP_A 192
#define SITE_IP_B 168
#define SITE_IP_C 1
#ifndef SITE_IP_D
#define SITE_IP_D 80
#endif
#define SITE_GW_A 192
#define SITE_GW_B 168
#define SITE_GW_C 1
#define SITE_GW_D 1
#define SITE_MASK_A 255
#define SITE_MASK_B 255
#define SITE_MASK_C 255
#define SITE_MASK_D 0
#define SITE_DNS_A 192
#define SITE_DNS_B 168
#define SITE_DNS_C 1
#define SITE_DNS_D 1

#define PLATFORM_STRINGIFY_INNER(value) #value
#define PLATFORM_STRINGIFY(value) PLATFORM_STRINGIFY_INNER(value)
#define PLATFORM_SETUP_IP_TEXT "192.168.1." PLATFORM_STRINGIFY(SITE_IP_D)
#define PLATFORM_SETUP_BASE_URL "http://" PLATFORM_SETUP_IP_TEXT "/"

#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <Updater.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <stdarg.h>
#include <pgmspace.h>
#include <stddef.h>
extern "C" {
#include <user_interface.h>
}
struct RtcOledDiagnostic;
struct DeviceCredentialRecord;
struct RtcCrashTrace;
struct RecoveryRecord;
struct OledDiagnostic;
enum class DisplayFont : uint8_t;
enum MqttAdminCommand : uint8_t;

// BEGIN APPLICATION DECLARATIONS
#include <EEPROM.h>
#define METER_DDS238 1
#define METER_JSY_MK333 2
#ifndef METER_TYPE
#define METER_TYPE METER_JSY_MK333
#endif
#define METER_NAME DEVICE_KIND
#if METER_TYPE == METER_DDS238
#define METER_READ_ERROR "dds238_read_failed"
#else
#define METER_READ_ERROR "jsy_read_failed"
#endif
#define EEPROM_ADDR_TARIFF_MODE 40
#define EEPROM_ADDR_TARIFF_MAGIC 80
const uint32_t TARIFF_MAGIC = 0x54524632UL;
#define DE_RE D5
const uint8_t ID = 1;
struct V3AccountingState;
struct V3CheckpointRecord;
struct V5Day;
struct V5State;
struct V3UtilityReading;
struct V3AccumPoint;
// v3 utility-reading command staging.
uint32_t mqttAdminUtilityId = 0;
bool mqttAdminUtilityDual = false;
bool mqttAdminUtilityExactTime = false;
uint16_t mqttAdminUtilityMinute = 0;
char mqttAdminUtilityDate[11] = {0};
double mqttAdminUtilityZ = 0.0f;
double mqttAdminUtilityZ1 = 0.0f;
double mqttAdminUtilityZ2 = 0.0f;
uint8_t mqttAdminUtilityKind = 0; // 0=T.K., 1=E.K.
bool mqttAdminTariffDual = false;

const char* LEGACY_STATS_FILE = "/daily_stats.txt";
const char* LEGACY_REGISTER_FILE = "/register_history.txt";

// Ελλάδα / Europe-Athens:
// EET = UTC+2, EEST = UTC+3.
// Η αλλαγή θερινής/χειμερινής ώρας γίνεται αυτόματα από localtime().



// =====================================================
// V3 ON-FLASH ABSOLUTE ENERGY SCHEMA
//
// "V3" εδώ είναι format compatibility label. Τα αρχεία/structs έχουν μείνει
// σκόπιμα με αυτά τα ονόματα ώστε firmware upgrades να διαβάζουν τα ήδη
// αποθηκευμένα δεδομένα χωρίς migration ή reset ιστορικού.
// =====================================================
const char* V3_CLEAN_MARKER_FILE = "/v3_clean_start.marker";
const char* V3_STATE_FILE = "/energy_v3_state.bin";
const char* V3_CHECKPOINT_FILE = "/energy_v3_cp.bin";
const char* V3_UTILITY_POINT_FILE = "/energy_v3_utility_points.bin";
const char* V3_UTILITY_FILE = "/utility_v3.bin";

const uint32_t V3_STATE_MAGIC = 0x45335631UL;      // "E3V1"
const uint32_t V3_CHECKPOINT_MAGIC = 0x43335631UL; // "C3V1"
const uint32_t V3_UTILITY_MAGIC = 0x55335631UL;    // "U3V1"
const uint16_t V3_DATA_VERSION = 1;
const uint16_t V3_WINDOW_MINUTE_START = 8U * 60U;
const uint16_t V3_WINDOW_MINUTE_END = 18U * 60U;
const uint16_t V3_WINDOW_MINUTE_MID = 13U * 60U;
const uint16_t V3_TIME_WINDOW = 0xFFFFU;
const uint32_t V3_CHECKPOINT_SECONDS = 30UL * 60UL;
const uint32_t V3_INTERPOLATION_MAX_GAP = 2UL * 60UL * 60UL;
const uint32_t V3_NEAR_POINT_MAX_GAP = 5UL * 60UL;
const uint32_t V3_CHECKPOINT_RETENTION_SECONDS = 45UL * 24UL * 60UL * 60UL;
const uint32_t V3_CHECKPOINT_COMPACT_INTERVAL = 48UL;
const uint8_t V3_MODE_MONO = 0;
const uint8_t V3_MODE_DUAL = 1;
// Binary values stay unchanged for OTA/data compatibility.
// v4.00 semantics:
//   FINAL        = user-entered "Καταχώρηση Μέτρησης ΔΕΗ" (comparison-only)
//   INTERMEDIATE = current "Καταχώρηση Ένδειξης Μετρητή ΔΕΗ" (ESP anchor / %)
const uint8_t V3_READING_FINAL = 0;
const uint8_t V3_READING_INTERMEDIATE = 1;
// Version-1 reserved bytes extend semantics without changing binary record size.
const uint8_t V3_EK_CORRECTION = 1;
const uint8_t V3_EK_DEVIATION = 2;
const uint8_t V3_EK_EVALUATED = 4;
const uint8_t V3_EK_BASE_KNOWN = 8;
const uint8_t V3_EK_NET_SNAPSHOT = 1; // reserved2[2]: signed net period Wh
const uint32_t V3_UTILITY_POINT_MAGIC = 0x56335550UL;
// For an exact near-now E.K., spanImportWh/spanZ1Wh hold immutable
// predicted/actual consumption for the SAME period, before correction.
// Other records retain their existing uncertainty-span meanings.

struct V3AccountingState {
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    uint64_t lastRawImportWh;
    uint64_t lastRawExportWh;
    uint64_t internalImportWh;
    uint64_t internalExportWh;
    uint64_t z1ImportWh;
    uint64_t z2ImportWh;
    uint64_t z1ExportWh;
    uint64_t z2ExportWh;
    uint32_t lastEpoch;
    uint32_t continuityGeneration;
    uint32_t lastCheckpointSlot;
    uint8_t rawBaselineValid;
    uint8_t reserved2[3];
    uint32_t checksum;
};

struct V3CheckpointRecord {
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    uint32_t epoch;
    uint32_t continuityGeneration;
    uint64_t internalImportWh;
    uint64_t internalExportWh;
    uint64_t z1ImportWh;
    uint64_t z2ImportWh;
    uint64_t z1ExportWh;
    uint64_t z2ExportWh;
    uint32_t checksum;
};

struct V3UtilityReading {
    uint32_t magic;
    uint16_t version;
    uint8_t active;
    uint8_t mode;
    uint32_t id;
    uint32_t measuredEpoch;
    uint32_t enteredEpoch;
    uint16_t minuteOfDay; // 0..1439 exact, 0xFFFF = 08:00-18:00 window
    uint16_t reserved;
    uint64_t officialZWh;
    uint64_t officialZ1Wh;
    uint64_t officialZ2Wh;
    uint64_t anchorImportWh;
    uint64_t anchorZ1ImportWh;
    uint64_t anchorZ2ImportWh;
    uint64_t spanImportWh;
    uint64_t spanZ1Wh;
    uint64_t spanZ2Wh;
    uint8_t mappingValid;
    uint8_t reserved2[3];
    uint32_t checksum;
};

struct V3AccumPoint {
    uint32_t epoch;
    uint32_t continuityGeneration;
    uint64_t importWh;
    uint64_t exportWh;
    uint64_t z1ImportWh;
    uint64_t z2ImportWh;
    uint64_t z1ExportWh;
    uint64_t z2ExportWh;
    uint32_t sampleEpoch; // actual sample time; interpolation uses target time
    uint8_t mappingSource; // 1=exact, 2=interpolated, 3=nearby, 4=window
};

struct V3UtilityPointRecord {
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    uint32_t utilityId;
    uint32_t anchorEpoch;
    uint64_t anchorImportWh;
    uint64_t anchorZ1ImportWh;
    uint64_t anchorZ2ImportWh;
    V3AccumPoint point;
    uint32_t checksum;
};

V3AccumPoint v3EstimatePoint = {};
bool v3EstimatePointValid = false;

V3AccountingState v3State = {};
V3UtilityReading v3LatestUtility = {};
bool v3LatestUtilityValid = false;
V3UtilityReading v3EstimateUtility = {};
bool v3EstimateUtilityValid = false;
uint32_t v3UtilityExcludedId = 0; // exclude the edited record while building its replacement
uint32_t v3LatestIntermediateId = 0;

// Explicit prototype prevents the Arduino 1.x sketch preprocessor from
// generating an invalid prototype before the custom struct declaration.
bool buildV3UtilityRecord(V3UtilityReading& rec,
                          uint32_t id,
                          uint8_t readingKind,
                          bool dual,
                          const char* dateText,
                          bool exactTime,
                          uint16_t minuteOfDay,
                          double z,
                          double z1,
                          double z2);

uint64_t v3RawImportWh = 0;
uint64_t v3RawExportWh = 0;

float voltage = 0.0f;
float current = 0.0f;
float power = 0.0f;
float pf = 0.0f;
float freq = 0.0f;
// Directional cumulative counters exposed on the OLED.
float totalEnergyFwd = 0.0f;
float totalEnergyRev = 0.0f;

#if METER_TYPE == METER_JSY_MK333
// =====================================================
// JSY-MK-333 TTL
//
// JSY TXD -> NodeMCU D5 / GPIO14
// JSY RXD -> NodeMCU D6 / GPIO12
// JSY GND -> NodeMCU GND
// NodeMCU VV (~5V) -> JSY 5-12V
// =====================================================

float voltage1 = 0.0f;
float voltage2 = 0.0f;
float voltage3 = 0.0f;
float current1 = 0.0f;
float current2 = 0.0f;
float current3 = 0.0f;

// JSY 0x0132 - Current active-power direction.
// bit0=L1, bit1=L2, bit2=L3, bit3=Total active power
// 0 = Forward, 1 = Reverse
uint16_t jsyPowerDirection = 0;
bool jsyReverse1 = false;
bool jsyReverse2 = false;
bool jsyReverse3 = false;
bool jsyReverseTotal = false;

float power1 = 0.0f;
float power2 = 0.0f;
float power3 = 0.0f;
float totalPowerJSY = 0.0f;
float pf1 = 0.0f;
float pf2 = 0.0f;
float pf3 = 0.0f;
#endif


bool dualZoneEnabled = false;

// Μονοζωνικό
double tariffZReference = 0.0f;
double tariffZNow = 0.0f;

// Διζωνικό
double tariffZ1Reference = 0.0f;
double tariffZ1Now = 0.0f;
double tariffZ2Reference = 0.0f;
double tariffZ2Now = 0.0f;

// Physical cumulative import/export counters come directly from both meters.
// DDS238: Forward 0x000A, Reverse 0x0008 (Total 0x0000 is not signed net).
// JSY: Forward 0x013A, Reverse 0x0142; 0x0132 signs instantaneous power only.
// Utility estimates use changes in Forward minus changes in Reverse.




#ifndef ESP8266_COMMON_PLATFORM_ENERGY_SNAPSHOT_H
#define ESP8266_COMMON_PLATFORM_ENERGY_SNAPSHOT_H

#include <stdint.h>

// Raw driver output. Drivers commit this value only after a complete successful
// sample; application/accounting code decides how to use cumulative counters.
struct EnergySnapshot {
    float voltage = 0.0f;
    float current = 0.0f;
    float power = 0.0f;
    float pf = 0.0f;
    float freq = 0.0f;
    float totalEnergyFwd = 0.0f;
    float totalEnergyRev = 0.0f;
    uint64_t rawImportWh = 0;
    uint64_t rawExportWh = 0;

    float voltage1 = 0.0f;
    float voltage2 = 0.0f;
    float voltage3 = 0.0f;
    float current1 = 0.0f;
    float current2 = 0.0f;
    float current3 = 0.0f;
    float power1 = 0.0f;
    float power2 = 0.0f;
    float power3 = 0.0f;
    float totalPowerJSY = 0.0f;
    float pf1 = 0.0f;
    float pf2 = 0.0f;
    float pf3 = 0.0f;
    uint16_t jsyPowerDirection = 0;
    bool jsyReverse1 = false;
    bool jsyReverse2 = false;
    bool jsyReverse3 = false;
    bool jsyReverseTotal = false;
};

void beginMeasurementDriver();
bool readMeasurementDriver(EnergySnapshot& out);

#endif
// END APPLICATION DECLARATIONS

// Display backend for the generated, single-translation-unit firmware sketch.
// OLED_TYPE is a build setting: 2 = SSD1309, 0 = no physical display.
// No U8g2 code or framebuffer is included in a headless build.
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#ifndef OLED_TYPE
  #error "Set OLED_TYPE to 2 (SSD1309) or 0 (no display) in the device profile"
#endif

enum class DisplayFont : uint8_t {
    Tiny4x6,
    Small5x7,
    Medium6x12,
    Normal10
};

#if OLED_TYPE == 2
  #include <U8g2lib.h>

class PlatformDisplay {
public:
    PlatformDisplay() : oled_(U8G2_R0, U8X8_PIN_NONE) {}

    // This describes the selected build capability, not an I2C hardware probe.
    bool available() const { return true; }
    void begin() { oled_.begin(); }
    void clear() { oled_.clearBuffer(); }
    void present() { oled_.sendBuffer(); }
    void contrast(uint8_t value) { oled_.setContrast(value); }

    void font(DisplayFont value) {
        switch (value) {
            case DisplayFont::Tiny4x6: oled_.setFont(u8g2_font_4x6_tf); break;
            case DisplayFont::Small5x7: oled_.setFont(u8g2_font_5x7_tf); break;
            case DisplayFont::Medium6x12: oled_.setFont(u8g2_font_6x12_tf); break;
            case DisplayFont::Normal10: oled_.setFont(u8g2_font_helvR10_tr); break;
        }
    }

    void text(int16_t x, int16_t y, const char* value) {
        if (value) oled_.drawStr(x, y, value);
    }

private:
    U8G2_SSD1309_128X64_NONAME0_F_HW_I2C oled_;
};

#elif OLED_TYPE == 0

class PlatformDisplay {
public:
    bool available() const { return false; }
    void begin() {}
    void clear() {}
    void present() {}
    void contrast(uint8_t) {}
    void font(DisplayFont) {}
    void text(int16_t, int16_t, const char*) {}
};

#else
  #error "Invalid OLED_TYPE: use 2 (SSD1309) or 0 (no display)"
#endif

static PlatformDisplay platformDisplay;

void displayClearAndPresent() {
    platformDisplay.clear();
    platformDisplay.present();
}

// Compatibility with the EnergyMeter line layout: draw into the existing
// framebuffer, use the original 420 font selector, and present immediately.
// A headless build avoids formatting the text as well as all display I/O.
void displayPrintf(int fontCode, int line, int pos, const char* format, ...) {
    if (!platformDisplay.available() || fontCode != 420 || !format) return;
    char buffer[128];
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(buffer, sizeof(buffer), format, arguments);
    va_end(arguments);
    platformDisplay.font(DisplayFont::Normal10);
    platformDisplay.text((pos - 1) * 8, line * 15, buffer);
    platformDisplay.present();
}



void applicationInitialize();
void applicationInitializeTopics();
bool applicationReadMeasurements();
void applicationRenderMeasurements(bool ok);
bool applicationStageCommand(const String& message);
bool applicationProcessCommand(MqttAdminCommand command);
void applicationResetCommand();
void applicationBackground(unsigned long now);
bool applicationAppendDebug(WiFiClient& client);
void applicationFormatSummary(char* out, size_t capacity);
bool applicationDualZoneEnabled();
#if PLATFORM_LOCAL_CONTROL_APPLICATION
void applicationEarlyInitialize();
void applicationServiceLocal(unsigned long now);
void applicationPrepareOta();
void applicationConfigureWebRoutes();
bool applicationImportLegacyCredentials();
bool applicationClearLegacyCredentials();
#endif

// Wi-Fi / MQTT runtime credentials.
// Broker host/port are public build settings. Wi-Fi/MQTT login secrets stay in LittleFS.
// Existing V1 credential records remain compatible after OTA.
char ssid[33] = {0};
char password[65] = {0};
#define MQTT_BROKER_HOST "b7e93fa24c0c4c86a995afecd61e93f3.s1.eu.hivemq.cloud"
const uint16_t MQTT_BROKER_PORT = 8883;
char mqttHost[128] = MQTT_BROKER_HOST;
uint16_t mqttPort = MQTT_BROKER_PORT;
char mqttUser[65] = {0};
char mqttPassword[65] = {0};

const char* DEVICE_CREDENTIAL_FILE = "/device_credentials.bin";
const char* DEVICE_CREDENTIAL_TEMP_FILE = "/device_credentials.tmp";
const uint32_t DEVICE_CREDENTIAL_MAGIC = 0x43524432UL; // "CRD2"
const uint16_t DEVICE_CREDENTIAL_VERSION = 1;

struct DeviceCredentialRecord {
    uint32_t magic;
    uint16_t version;
    uint16_t mqttPort;
    char ssid[33];
    char wifiPassword[65];
    char mqttHost[128];
    char mqttUser[65];
    char mqttPassword[65];
    uint32_t checksum;
};

uint32_t deviceCredentialChecksum(const DeviceCredentialRecord& rec) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&rec);
    const size_t n = offsetof(DeviceCredentialRecord, checksum);
    uint32_t h = 2166136261UL;
    for (size_t i = 0; i < n; ++i) {
        h ^= p[i];
        h *= 16777619UL;
    }
    return h;
}

bool credentialFieldIsValid(const char* value, size_t capacity) {
    if (!value || capacity == 0) return false;
    size_t n = strnlen(value, capacity);
    return n > 0 && n < capacity;
}

bool copyCredentialField(char* dst, size_t dstSize, const char* src) {
    if (!dst || dstSize == 0 || !src) return false;
    size_t n = strlen(src);
    if (n == 0 || n >= dstSize) return false;
    memcpy(dst, src, n + 1);
    return true;
}

bool copyOptionalCredentialField(char* dst, size_t capacity, const char* src) {
    if (!dst || !src || capacity == 0) return false;
    const size_t n = strnlen(src, capacity);
    if (n >= capacity) return false;
    memcpy(dst, src, n + 1);
    return true;
}

// Wi-Fi availability does not depend on MQTT credentials or broker acceptance.
bool validateDeviceCredentialRecord(const DeviceCredentialRecord& rec) {
    return
        rec.magic == DEVICE_CREDENTIAL_MAGIC &&
        rec.version == DEVICE_CREDENTIAL_VERSION &&
        rec.checksum == deviceCredentialChecksum(rec) &&
        credentialFieldIsValid(rec.ssid, sizeof(rec.ssid)) &&
        credentialFieldIsValid(rec.wifiPassword, sizeof(rec.wifiPassword)) &&
        strnlen(rec.mqttHost, sizeof(rec.mqttHost)) < sizeof(rec.mqttHost) &&
        strnlen(rec.mqttUser, sizeof(rec.mqttUser)) < sizeof(rec.mqttUser) &&
        strnlen(rec.mqttPassword, sizeof(rec.mqttPassword)) < sizeof(rec.mqttPassword);
}

bool applyDeviceCredentials(const DeviceCredentialRecord& rec) {
    if (!validateDeviceCredentialRecord(rec)) return false;
    if (!copyCredentialField(ssid, sizeof(ssid), rec.ssid) ||
        !copyCredentialField(password, sizeof(password), rec.wifiPassword) ||
        !copyCredentialField(mqttHost, sizeof(mqttHost), MQTT_BROKER_HOST) ||
        !copyOptionalCredentialField(mqttUser, sizeof(mqttUser), rec.mqttUser) ||
        !copyOptionalCredentialField(mqttPassword, sizeof(mqttPassword), rec.mqttPassword)) {
        return false;
    }
    mqttPort = MQTT_BROKER_PORT;
    return true;
}

bool readDeviceCredentialRecordAtPath(DeviceCredentialRecord& rec, const char* path) {
    if (!LittleFS.exists(path)) return false;
    File file = LittleFS.open(path, "r");
    if (!file) return false;
    memset(&rec, 0, sizeof(rec));
    bool ok =
        file.size() == (size_t)sizeof(rec) &&
        file.readBytes(reinterpret_cast<char*>(&rec), sizeof(rec)) == sizeof(rec);
    file.close();
    return ok && validateDeviceCredentialRecord(rec);
}

bool readDeviceCredentialRecord(DeviceCredentialRecord& rec) {
    return readDeviceCredentialRecordAtPath(rec, DEVICE_CREDENTIAL_FILE);
}

bool writeDeviceCredentialRecord(const DeviceCredentialRecord& rec) {
    if (!validateDeviceCredentialRecord(rec)) return false;
    // Keep the existing record intact until the staged write has been closed
    // and read back. LittleFS rename replaces the destination atomically.
    File file = LittleFS.open(DEVICE_CREDENTIAL_TEMP_FILE, "w");
    if (!file) return false;
    bool ok = file.write(reinterpret_cast<const uint8_t*>(&rec), sizeof(rec)) == sizeof(rec);
    file.flush();
    file.close();
    DeviceCredentialRecord verified = {};
    ok = ok && readDeviceCredentialRecordAtPath(verified, DEVICE_CREDENTIAL_TEMP_FILE) &&
         memcmp(&verified, &rec, sizeof(rec)) == 0;
    if (!ok || !LittleFS.rename(DEVICE_CREDENTIAL_TEMP_FILE, DEVICE_CREDENTIAL_FILE)) {
        LittleFS.remove(DEVICE_CREDENTIAL_TEMP_FILE);
        return false;
    }
    return readDeviceCredentialRecord(verified) && memcmp(&verified, &rec, sizeof(rec)) == 0;
}

bool loadDeviceCredentials() {
    DeviceCredentialRecord stored = {};
    return readDeviceCredentialRecord(stored) && applyDeviceCredentials(stored);
}

// ===== MQTT REQUEST/RESPONSE TOPICS =====

// Το ESP θα ακούει στο REQUEST topic και θα απαντά στο STATE topic
// μόνο όταν ο χρήστης στείλει το μήνυμα: update
const size_t MQTT_DEVICE_ID_BYTES = sizeof(DEVICE_ID) + 13; // dash + 12 hex digits + NUL
const size_t MQTT_TOPIC_BASE_BYTES = sizeof(MQTT_TOPIC_PREFIX) + MQTT_DEVICE_ID_BYTES - 1;
char mqttDeviceId[MQTT_DEVICE_ID_BYTES] = {};
char MQTT_STATE_TOPIC[MQTT_TOPIC_BASE_BYTES + sizeof("/state") - 1] = {};
char MQTT_STATUS_TOPIC[MQTT_TOPIC_BASE_BYTES + sizeof("/status") - 1] = {};
char MQTT_REQUEST_TOPIC[MQTT_TOPIC_BASE_BYTES + sizeof("/request") - 1] = {};

// ===== MQTT ADMIN TOPICS: ΔΕΗ / HISTORY / OTA =====
char MQTT_ADMIN_REQUEST_TOPIC[MQTT_TOPIC_BASE_BYTES + sizeof("/admin/request") - 1] = {};
char MQTT_ADMIN_RESPONSE_TOPIC[MQTT_TOPIC_BASE_BYTES + sizeof("/admin/response") - 1] = {};
char MQTT_ADMIN_CSV_TOPIC[MQTT_TOPIC_BASE_BYTES + sizeof("/admin/csv") - 1] = {};

BearSSL::WiFiClientSecure mqttNet;
PubSubClient mqttClient(mqttNet);

bool mqttUpdateRequested = false;
unsigned long lastMqttReconnectMs = 0;

// v5.57 long-run network stability.
// The APP asks for an update every minute. Keep the hot request path
// allocation-free and back off failed publishes instead of retrying every
// 1.5 seconds forever under memory/network pressure.
const unsigned long MQTT_UPDATE_RETRY_STEPS_MS[] = {
    5000UL, 10000UL, 30000UL
};
const uint8_t MQTT_UPDATE_RETRY_STEP_COUNT =
    sizeof(MQTT_UPDATE_RETRY_STEPS_MS) / sizeof(MQTT_UPDATE_RETRY_STEPS_MS[0]);
uint8_t mqttUpdateRetryStep = 0;
unsigned long mqttUpdateRetryDelayMs = 1500UL;

// If a previously healthy unit stays on Wi-Fi but repeatedly gets MQTT
// transport state -2, recycle only the network stack. This releases stale
// lwIP/TLS resources and restarts the local HTTP listener without rebooting
// metering/accounting state.
const uint8_t MQTT_RUNTIME_RECOVERY_FAILURES_LOW_HEAP = 4;
const uint8_t MQTT_RUNTIME_RECOVERY_FAILURES_GENERIC = 12;
uint8_t mqttRuntimeConnectFailureStreak = 0;
bool networkStackRecoveryPending = false;
uint32_t networkStackRecoveryCount = 0;
// Boot-time broker failures back off gradually. Once this ESP has had a
// working MQTT session, a later router/Internet interruption uses a much
// faster recovery cadence so MQTT comes back without a device restart.
const unsigned long MQTT_RETRY_STEPS_MS[] = {
    10000UL, 30000UL, 60000UL, 120000UL, 300000UL
};
const unsigned long MQTT_RUNTIME_RETRY_STEPS_MS[] = {
    5000UL, 10000UL, 30000UL
};
const uint8_t MQTT_RETRY_STEP_COUNT =
    sizeof(MQTT_RETRY_STEPS_MS) / sizeof(MQTT_RETRY_STEPS_MS[0]);
const uint8_t MQTT_RUNTIME_RETRY_STEP_COUNT =
    sizeof(MQTT_RUNTIME_RETRY_STEPS_MS) / sizeof(MQTT_RUNTIME_RETRY_STEPS_MS[0]);
uint8_t mqttRetryStep = 0;
unsigned long mqttRetryDelayMs = MQTT_RETRY_STEPS_MS[0];
uint32_t mqttConnectFailures = 0;
bool mqttEverConnected = false;

// V2.01: passive diagnostics of the REAL MQTT/BearSSL connection.
uint32_t mqttConnectAttempts = 0;
unsigned long mqttLastAttemptMs = 0;
int mqttLastAttemptState = -1;
bool mqttLastConnectSucceeded = false;
bool mqttLastStatusPublishSucceeded = false;
bool mqttLastRequestSubscribeSucceeded = false;
bool mqttLastAdminSubscribeSucceeded = false;
int mqttLastSslErrorCode = 0;
char mqttLastSslErrorText[160] = {0};
// Capture the resource gate before TLS allocates buffers; current web-request
// heap alone cannot explain a deferred connection attempt.
uint32_t mqttConnectHeapAtAttempt = 0;
uint16_t mqttConnectBlockAtAttempt = 0;
uint32_t mqttConnectPreflightDeferrals = 0;
// v2.02: Negotiate smaller TLS records when the HiveMQ endpoint supports
// TLS 1.2 Maximum Fragment Length Negotiation. Never silently assume MFLN.
const uint16_t MQTT_TLS_SMALL_RX_BYTES = 4096;
const uint16_t MQTT_TLS_COMPAT_RX_BYTES = 8192;
const uint16_t MQTT_TLS_TX_BYTES = 512;
const uint16_t MQTT_PACKET_BUFFER_BYTES = 1024; // enough for ~844-byte JSY state + topic/header
const uint16_t MQTT_STATE_PAYLOAD_BYTES = 864;  // bounded stack buffer; includes utility reference date
// The publish heap check uses locals inside publishMqttSample(); no diagnostic
// globals are kept for values that are never displayed or consumed later.
const unsigned long MQTT_UPDATE_MIN_INTERVAL_MS = 1500UL;
unsigned long lastMqttUpdatePublishMs = 0;

// v2.21: ESP8266 Core 2.7.2 write_P()/sendContent_P() allocates an
// internal buffer sized from tcp_sndbuf. Under fragmented low heap that
// allocation can fail and ProgmemStream then reaches memcpy_P with a null
// destination. Root HTML is therefore copied through this fixed RAM slice
// and sent with normal WiFiClient::write().
const size_t WEB_ROOT_RAM_SLICE_BYTES = 64;
// v4.04 drains every 64-byte COPY write before queuing the next slice.
// These floors allow the page under TLS load, while refusing normal writes
// above the observed crash point (464 B free / 184 B max block). A smaller
// emergency response reports memory pressure instead of a blank HTTP 204.
const uint32_t WEB_ROOT_MIN_HEAP_BYTES = 1024UL;
const uint16_t WEB_ROOT_MIN_BLOCK_BYTES = 256U;
// Browsers can cancel an in-flight page by pressing Refresh again. Explicit
// socket cleanup + a very short reload guard prevents abandoned HTTP sockets
// from exhausting the ESP8266 lwIP PCB/pbuf pool while MQTT keeps running.
const unsigned long WEB_ROOT_MIN_REQUEST_GAP_MS = 350UL;
unsigned long webRootLastRequestMs = 0;
uint32_t webRootRequestCount = 0;
uint32_t webRootRateLimitedCount = 0;
uint32_t webRootWriteAbortCount = 0;
uint32_t webRootForcedCloseCount = 0;
uint32_t webRootBusyCount = 0;
const unsigned long WEB_RESPONSE_BUDGET_MS = 10000UL;
// Network Debug may require longer to stream under TLS load.
unsigned long webResponseBudgetMs = WEB_RESPONSE_BUDGET_MS;
unsigned long webResponseStartedMs = 0;
bool webResponseActive = false;
const unsigned long MQTT_MFLN_RECHECK_MS = 300000UL;
bool mqttMflnChecked = false;
bool mqttMflnSupported = false;
bool mqttMflnNegotiatedAtLastConnect = false;
bool mqttSmallTlsBufferActive = false;
bool mqttCompatTlsBufferActive = false;
unsigned long mqttMflnLastProbeMs = 0;
unsigned long mqttMflnProbeDurationMs = 0;
uint32_t mqttMflnProbeCount = 0;
uint16_t mqttTlsRecvConfiguredBytes = 16384;
const unsigned long MQTT_DNS_TIMEOUT_MS = 1500UL;
const unsigned long MQTT_TLS_TIMEOUT_MS = 4000UL;

// v2.06: crash breadcrumb in RTC user memory. It survives exception/watchdog
// resets without flash wear and records the last high-risk operation entered.
// Offset is in 4-byte blocks; 64 is outside the first 32 blocks used by OTA.
const uint32_t RTC_CRASH_TRACE_MAGIC = 0x43543242UL; // "CT2B" v2.11 layout
const uint32_t RTC_CRASH_TRACE_OFFSET = 64;
enum CrashStage : uint32_t {
    CRASH_STAGE_NONE = 0,
    CRASH_STAGE_SETUP = 10,
    CRASH_STAGE_BOOT_OLED_INIT = 11,
    CRASH_STAGE_BOOT_FILESYSTEM = 12,
    CRASH_STAGE_BOOT_CREDENTIALS = 13,
    CRASH_STAGE_BOOT_RECOVERY_CHECK = 14,
    CRASH_STAGE_BOOT_RECOVERY_CLEAR = 15,
    CRASH_STAGE_BOOT_EEPROM = 20,
    CRASH_STAGE_BOOT_CLEAN_START = 21,
    CRASH_STAGE_BOOT_TARIFF = 22,
    CRASH_STAGE_BOOT_STATE = 23,
    CRASH_STAGE_BOOT_UTILITY = 24,
    CRASH_STAGE_BOOT_ESTIMATE = 25,
    CRASH_STAGE_BOOT_SERIAL = 30,
    CRASH_STAGE_BOOT_WEB_CONFIG = 31,
    CRASH_STAGE_BOOT_MQTT_CONFIG = 32,
    CRASH_STAGE_WIFI_CONNECT = 40,
    CRASH_STAGE_BOOT_AP_START = 41,
    CRASH_STAGE_BOOT_AP_IDLE = 42,
    CRASH_STAGE_BOOT_AP_FAILED = 43,
    CRASH_STAGE_NETWORK_SERVICES = 50,
    CRASH_STAGE_BOOT_LOOP_ENTER = 51,
    CRASH_STAGE_BOOT_FIRST_WEB = 52,
    CRASH_STAGE_BOOT_MQTT_DNS = 59,
    CRASH_STAGE_MFLN_PROBE = 60,
    CRASH_STAGE_MQTT_CONNECT = 61,
    CRASH_STAGE_BOOT_FIRST_METER = 65,
    CRASH_STAGE_BOOT_ACCOUNTING = 66,
    CRASH_STAGE_BOOT_FIRST_DISPLAY = 67,
    CRASH_STAGE_BOOT_RUNNING = 68,
    CRASH_STAGE_BOOT_RECOVERY_IDLE = 69,
    CRASH_STAGE_MQTT_JSON_BUILD = 70,
    CRASH_STAGE_MQTT_PUBLISH = 72,
    CRASH_STAGE_WEB_ROOT = 80,
    CRASH_STAGE_WEB_ROOT_HEADER = 85,
    CRASH_STAGE_WEB_ROOT_RAMCOPY = 86,
    CRASH_STAGE_WEB_ROOT_WRITE = 87,
    CRASH_STAGE_WEB_SETUP = 88,
    CRASH_STAGE_WEB_NET_DEBUG = 89,
    CRASH_STAGE_REMOTE_OTA_PREP = 90,
    CRASH_STAGE_REMOTE_OTA_TLS = 91,
    CRASH_STAGE_REMOTE_OTA_HTTP = 92,
    CRASH_STAGE_REMOTE_OTA_FLASH = 93,
    CRASH_STAGE_BOOT_OLED_WRITE = 100
};
bool bootCheckpointsActive = true;
bool bootOledReady = false;
bool bootFirstMeterDone = false;
bool bootFirstMqttAttempted = false;
uint32_t bootLastCheckpoint = CRASH_STAGE_NONE;
void bootCheckpoint(uint32_t stage);
void finishBootCheckpoints(uint32_t stage);
struct RtcCrashTrace {
    uint32_t magic;
    uint32_t stage;
    uint32_t detail;
    uint32_t heapFree;
    uint32_t maxFreeBlock;
    uint32_t heapFragmentation;
    uint32_t checksum;
};
RtcCrashTrace rtcTraceBeforeReset = {};
bool rtcTraceBeforeResetValid = false;

uint32_t rtcCrashTraceChecksum(uint32_t stage, uint32_t detail,
                               uint32_t heapFree, uint32_t maxFreeBlock,
                               uint32_t heapFragmentation) {
    return RTC_CRASH_TRACE_MAGIC ^ stage ^ detail ^ heapFree ^
           maxFreeBlock ^ heapFragmentation ^ 0xA526C31DUL;
}

void loadRtcCrashTrace() {
    RtcCrashTrace t = {};
    static_assert((sizeof(RtcCrashTrace) % 4) == 0,
                  "RTC crash trace size must be a multiple of 4 bytes");
    if (ESP.rtcUserMemoryRead(
            RTC_CRASH_TRACE_OFFSET,
            reinterpret_cast<uint32_t*>(&t),
            sizeof(t)) &&
        t.magic == RTC_CRASH_TRACE_MAGIC &&
        t.checksum == rtcCrashTraceChecksum(t.stage, t.detail,
                                             t.heapFree, t.maxFreeBlock,
                                             t.heapFragmentation)) {
        rtcTraceBeforeReset = t;
        rtcTraceBeforeResetValid = true;
    }
}

void markCrashStage(uint32_t stage, uint32_t detail) {
    RtcCrashTrace t = {};
    t.magic = RTC_CRASH_TRACE_MAGIC;
    t.stage = stage;
    t.detail = detail;
    uint32_t freeHeap = 0;
    uint16_t maxBlock = 0;
    uint8_t fragmentation = 0;
    ESP.getHeapStats(&freeHeap, &maxBlock, &fragmentation);
    t.heapFree = freeHeap;
    t.maxFreeBlock = maxBlock;
    t.heapFragmentation = fragmentation;
    t.checksum = rtcCrashTraceChecksum(stage, detail,
                                       t.heapFree, t.maxFreeBlock,
                                       t.heapFragmentation);
    ESP.rtcUserMemoryWrite(
        RTC_CRASH_TRACE_OFFSET,
        reinterpret_cast<uint32_t*>(&t),
        sizeof(t));
}

void clearRtcCrashTrace() {
    RtcCrashTrace empty = {};
    ESP.rtcUserMemoryWrite(
        RTC_CRASH_TRACE_OFFSET,
        reinterpret_cast<uint32_t*>(&empty),
        sizeof(empty));
    rtcTraceBeforeReset = RtcCrashTrace();
    rtcTraceBeforeResetValid = false;
}

const char* crashStageLabel(uint32_t stage) {
    switch (stage) {
        case CRASH_STAGE_SETUP: return "SETUP";
        case CRASH_STAGE_BOOT_OLED_INIT: return "OLED_INIT";
        case CRASH_STAGE_BOOT_FILESYSTEM: return "FILESYSTEM";
        case CRASH_STAGE_BOOT_CREDENTIALS: return "LOAD_CREDENTIALS";
        case CRASH_STAGE_BOOT_RECOVERY_CHECK: return "CHECK_RECOVERY";
        case CRASH_STAGE_BOOT_RECOVERY_CLEAR: return "CLEAR_RECOVERY";
        case CRASH_STAGE_BOOT_EEPROM: return "EEPROM_INIT";
        case CRASH_STAGE_BOOT_CLEAN_START: return "V3_INIT";
        case CRASH_STAGE_BOOT_TARIFF: return "LOAD_TARIFF";
        case CRASH_STAGE_BOOT_STATE: return "LOAD_STATE";
        case CRASH_STAGE_BOOT_UTILITY: return "LOAD_UTILITY";
        case CRASH_STAGE_BOOT_ESTIMATE: return "ESTIMATE_VIEW";
        case CRASH_STAGE_BOOT_SERIAL: return "METER_SERIAL";
        case CRASH_STAGE_BOOT_WEB_CONFIG: return "WEB_CONFIG";
        case CRASH_STAGE_BOOT_MQTT_CONFIG: return "MQTT_CONFIG";
        case CRASH_STAGE_WIFI_CONNECT: return "WIFI_CONNECT";
        case CRASH_STAGE_BOOT_AP_START: return "SETUP_AP_START";
        case CRASH_STAGE_BOOT_AP_IDLE: return "SETUP_AP_WAIT";
        case CRASH_STAGE_BOOT_AP_FAILED: return "SETUP_AP_FAILED";
        case CRASH_STAGE_NETWORK_SERVICES: return "NETWORK_SERVICES";
        case CRASH_STAGE_BOOT_LOOP_ENTER: return "ENTER_LOOP";
        case CRASH_STAGE_BOOT_FIRST_WEB: return "FIRST_WEB";
        case CRASH_STAGE_BOOT_MQTT_DNS: return "MQTT_DNS";
        case CRASH_STAGE_MFLN_PROBE: return "MFLN_PROBE";
        case CRASH_STAGE_MQTT_CONNECT: return "MQTT_CONNECT";
        case CRASH_STAGE_BOOT_FIRST_METER: return "FIRST_METER_READ";
        case CRASH_STAGE_BOOT_ACCOUNTING: return "FIRST_ACCOUNTING";
        case CRASH_STAGE_BOOT_FIRST_DISPLAY: return "FIRST_LIVE_OLED";
        case CRASH_STAGE_BOOT_RUNNING: return "LOCAL_RUNNING";
        case CRASH_STAGE_BOOT_RECOVERY_IDLE: return "RECOVERY_WAIT";
        case CRASH_STAGE_MQTT_JSON_BUILD: return "MQTT_JSON_BUILD";
        case CRASH_STAGE_MQTT_PUBLISH: return "MQTT_PUBLISH";
        case CRASH_STAGE_WEB_ROOT: return "WEB_ROOT";
        case CRASH_STAGE_WEB_ROOT_HEADER: return "WEB_ROOT_HEADER";
        case CRASH_STAGE_WEB_ROOT_RAMCOPY: return "WEB_ROOT_RAMCOPY";
        case CRASH_STAGE_WEB_ROOT_WRITE: return "WEB_ROOT_WRITE";
        case CRASH_STAGE_WEB_SETUP: return "WEB_SETUP";
        case CRASH_STAGE_WEB_NET_DEBUG: return "WEB_NET_DEBUG";
        case CRASH_STAGE_REMOTE_OTA_PREP: return "REMOTE_OTA_PREP";
        case CRASH_STAGE_REMOTE_OTA_TLS: return "REMOTE_OTA_TLS";
        case CRASH_STAGE_REMOTE_OTA_HTTP: return "REMOTE_OTA_HTTP";
        case CRASH_STAGE_REMOTE_OTA_FLASH: return "REMOTE_OTA_FLASH";
        case CRASH_STAGE_BOOT_OLED_WRITE: return "CHECKPOINT_OLED_WRITE";
        default: return "UNKNOWN/NONE";
    }
}



// ===== MQTT ADMIN COMMAND STATE =====
enum MqttAdminCommand : uint8_t {
    MQTT_ADMIN_NONE = 0,
    MQTT_ADMIN_REMOTE_OTA,
    MQTT_ADMIN_OTA_REJECTED,
    MQTT_ADMIN_RESTART,
    // APPLICATION COMMAND ENUM
    MQTT_ADMIN_INVALID
};

MqttAdminCommand mqttAdminCommand = MQTT_ADMIN_NONE;
bool mqttAdminCommandPending = false;

// v4.03 remote normal restart, requested from MQTT.app Settings.
bool mqttAdminRestartPending = false;
unsigned long mqttAdminRestartRequestedMs = 0;
const unsigned long MQTT_ADMIN_RESTART_DELAY_MS = 1200UL;

// v2.26: remote pull-OTA command transported by the already-established
// MQTT admin channel. The firmware file itself is downloaded by the ESP8266
// over outbound HTTPS, so no router port-forwarding is required.
//
// Command:
// ota_https_v2|<size_bytes>|<md5_32hex>|<https_url>|<compiled_profile>
//
// Size and MD5 are mandatory. The URL should preferably be a short-lived
// private/presigned HTTPS URL is still recommended for controlled rollout.
// v3.00+ firmware binaries do not contain site Wi-Fi/MQTT credentials.
const size_t REMOTE_OTA_URL_BYTES = 420;
const size_t REMOTE_OTA_MD5_BYTES = 33;
char mqttAdminOtaUrl[REMOTE_OTA_URL_BYTES] = {0};
char mqttAdminOtaMd5[REMOTE_OTA_MD5_BYTES] = {0};
uint32_t mqttAdminOtaSize = 0;
const char* mqttAdminOtaError = "invalid_parameters";
// v2.29: GitHub raw does not negotiate TLS MFLN. Download the image with
// HTTP byte ranges instead, keeping every HTTPS response small enough for
// an 8 KB BearSSL RX buffer. This avoids the old 16 KB contiguous-heap path.
const uint16_t REMOTE_OTA_TLS_RX_RANGE = 8192;
const uint16_t REMOTE_OTA_TLS_TX = 512;
const uint16_t REMOTE_OTA_RANGE_BYTES = 4096;
const uint16_t REMOTE_OTA_STREAM_BUFFER_BYTES = 512;
const unsigned long REMOTE_OTA_IO_TIMEOUT_MS = 15000UL;
const unsigned long REMOTE_OTA_TOTAL_TIMEOUT_MS = 240000UL;
const unsigned long REMOTE_OTA_ACK_SETTLE_MS = 180UL;

ESP8266WebServer server(80);
DNSServer credentialDns;
bool credentialPortalActive = false;
bool credentialPortalOnly = false;
// AP-only describes the radio, not whether a configured meter may keep running.
bool normalMeterInitialized = false;
unsigned long lastRead = 0;
const unsigned long interval = APP_SAMPLE_INTERVAL_MS;
bool lastMeterReadOk = false;
// Device transport and physical meter health are independent.
bool meterReadAttempted = false;
bool meterHasGoodSample = false;
bool meterHealthDirty = true;
bool meterHealthConnectionPublished = false;
uint32_t meterReadFailures = 0;
unsigned long meterLastGoodMs = 0;
unsigned long meterLastHealthPublishMs = 0;
char MQTT_HEALTH_TOPIC[MQTT_TOPIC_BASE_BYTES + sizeof("/health") - 1] = {};

unsigned long credentialPortalLastAttemptMs = 0;
const unsigned long CREDENTIAL_AP_RETRY_MS = 30000UL;
uint32_t credentialApAttempts = 0;
uint8_t credentialApLastFailure = 0; // 1=radio mode, 2=IP config, 3=AP start
uint32_t credentialApHeapAtAttempt = 0;
bool credentialRoutesConfigured = false;
bool credentialRestartPending = false;
unsigned long credentialRestartRequestedMs = 0;
unsigned long wifiOfflineSinceMs = 0;
const unsigned long CREDENTIAL_RESTART_DELAY_MS = 1200UL;
// Valid credentials always stay in local metering mode, even with no router.
// Missing credentials and deliberate Recovery are the only automatic AP paths.
const unsigned long WIFI_RETRY_STEPS_MS[] = {15000UL, 30000UL, 60000UL, 120000UL};
const uint8_t WIFI_RETRY_STEP_COUNT = sizeof(WIFI_RETRY_STEPS_MS) / sizeof(WIFI_RETRY_STEPS_MS[0]);
uint8_t wifiRetryStep = 0;
unsigned long lastWifiRetry = 0;
uint32_t wifiConnectAttempts = 0;
bool wifiWasConnected = false;
WiFiEventHandler wifiDisconnectHandler;
uint16_t wifiLastDisconnectReason = 0;
const uint8_t FLASH_SETUP_PIN = 0; // NodeMCU FLASH / D3; do not hold during reset.
const uint32_t FLASH_DEBOUNCE_MS = 40UL;
const uint32_t FLASH_ACTION_COOLDOWN_MS = 750UL;
volatile bool flashPressActive = false;
volatile bool flashSetupRequested = false;
volatile uint32_t flashPressStartedMs = 0;
bool flashActionTaken = false;
uint32_t flashLastActionMs = 0;
const unsigned long WIFI_HINT_INTERVAL_MS = 15000UL;
const unsigned long WIFI_HINT_DURATION_MS = 4000UL;
bool wifiHintVisible = false;
bool wifiHintDrawPending = false;
unsigned long wifiHintStartedMs = 0;
unsigned long wifiHintLastFrameMs = 0;
String credentialApName;
String credentialApPassword;

// Setup AP is open; every HTTP page/action requires this PIN in AP mode.
const char AP_WEB_PIN[] = "12134";
const uint32_t AP_WEB_SESSION_MS = 15UL * 60UL * 1000UL;
char apWebSessionToken[33] = {0};
uint32_t apWebSessionStartedMs = 0;

void resetApWebSession() {
    apWebSessionToken[0] = '\0';
    apWebSessionStartedMs = 0;
}

bool apWebSessionValid() {
    if (!apWebSessionToken[0]) return false;
    if ((uint32_t)(millis() - apWebSessionStartedMs) >= AP_WEB_SESSION_MS) {
        resetApWebSession();
        return false;
    }
    String cookies = server.header("Cookie");
    char expected[44];
    snprintf(expected, sizeof(expected), "EMSESSION=%s", apWebSessionToken);
    int start = 0;
    while (start < (int)cookies.length()) {
        int end = cookies.indexOf(';', start);
        if (end < 0) end = cookies.length();
        String part = cookies.substring(start, end);
        part.trim();
        if (part == expected) return true;
        start = end + 1;
    }
    return false;
}

void sendApPinPage(int statusCode) {
    server.sendHeader("Cache-Control", "no-store");
    server.send_P(statusCode, PSTR("text/html; charset=utf-8"), PSTR(
        "<!doctype html><html><head><meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>" DEVICE_LABEL " PIN</title><style>body{font:22px/1.35 Arial,sans-serif;max-width:760px;margin:auto;padding:10px;color:#142431}h1,h2,small{font-size:inherit}pre{font:inherit;white-space:pre-wrap;overflow-wrap:anywhere}input,button{font:inherit}</style></head><body>"
        "<h2>" DEVICE_LABEL "</h2><p>Enter the PIN to open this device.</p>"
        "<form method='post' action='/ap-login'>"
        "<label>PIN <input name='pin' type='password' inputmode='numeric' "
        "pattern='[0-9]{5}' minlength='5' maxlength='5' required autofocus "
        "autocomplete='off'></label> <button type='submit'>Open</button>"
        "</form></body></html>"));
}

bool requireApWebPin() {
    if (!credentialPortalActive) return true; // Existing LAN behavior.
    if (apWebSessionValid()) return true;
    sendApPinPage(server.method() == HTTP_GET ? 200 : 403);
    return false;
}

void closeRootClient(WiFiClient& client, bool graceful);
class WebResponseScope {
public:
    WebResponseScope() : client(server.client()) {
        webResponseStartedMs = millis();
        webResponseBudgetMs = WEB_RESPONSE_BUDGET_MS;
        webResponseActive = true;
        client.setTimeout(700);
        client.setNoDelay(true);
    }
    ~WebResponseScope() {
        if (client.connected()) closeRootClient(client, true);
        webResponseActive = false;
    }
private:
    WiFiClient client;
};

void handleApWebLogin() {
    WebResponseScope responseScope;
    if (!credentialPortalActive) {
        server.send(404, "text/plain", "Not found");
        return;
    }
    if (server.arg("pin") != AP_WEB_PIN) {
        sendApPinPage(403);
        return;
    }
    // The PIN is never sent to the browser or stored in the cookie.
    snprintf(apWebSessionToken, sizeof(apWebSessionToken), "%08lX%08lX%08lX%08lX",
             (unsigned long)os_random(), (unsigned long)os_random(),
             (unsigned long)os_random(), (unsigned long)os_random());
    apWebSessionStartedMs = millis();
    char cookie[108];
    snprintf(cookie, sizeof(cookie),
             "EMSESSION=%s; Path=/; HttpOnly; SameSite=Strict; Max-Age=900",
             apWebSessionToken);
    server.sendHeader("Cache-Control", "no-store");
    server.sendHeader("Set-Cookie", cookie);
    server.sendHeader("Location", "/", true);
    server.send(303, "text/plain", "");
}


// Non-blocking, fixed-size diagnostic queue. Each distinct failure category
// gets at least four seconds on the OLED; retries never erase an earlier failure.
// RTC replay also retains an error through an automatic OTA/restart, without
// writing flash or changing credentials, accounting or Recovery storage.
enum OledDiagnosticCode : uint32_t {
    OLED_DIAG_WIFI = 1, OLED_DIAG_SETUP_AP = 2, OLED_DIAG_MQTT = 3,
    OLED_DIAG_METER = 4, OLED_DIAG_FILESYSTEM = 5, OLED_DIAG_CREDENTIALS = 6,
    OLED_DIAG_OTA = 7, OLED_DIAG_RECOVERY_CLEAR = 8
};
const unsigned long OLED_DIAGNOSTIC_HOLD_MS = 4000UL;
const uint8_t OLED_DIAGNOSTIC_SLOTS = 8;
struct OledDiagnostic {
    uint32_t code;
    char title[24];
    char detail[64];
};
OledDiagnostic oledDiagnostics[OLED_DIAGNOSTIC_SLOTS] = {};
uint8_t oledDiagnosticCount = 0;
bool oledDiagnosticStarted = false;
unsigned long oledDiagnosticStartedMs = 0;
unsigned long oledDiagnosticLastFrameMs = 0;
const uint32_t RTC_OLED_DIAGNOSTIC_MAGIC = 0x4F443136UL;
const uint32_t RTC_OLED_DIAGNOSTIC_OFFSET = 80; // crash trace uses blocks 64..70
struct RtcOledDiagnostic {
    uint32_t magic;
    OledDiagnostic message;
    uint32_t checksum;
};
void queueOledDiagnostic(uint32_t code, const char* title, const char* detail);
bool drawOledDiagnostic(unsigned long now);
void loadOledDiagnostic();
void serviceOledDiagnostic(unsigned long now);
const char* wifiFailureLabel();


const char* GREECE_TZ = "EET-2EEST,M3.5.0/3,M10.5.0/4";

bool littleFsReady = false;
bool networkServicesStarted = false;

// Ver 2.0: crash recovery latch. No credentials or measurements are stored.
const char* RECOVERY_FILE = "/recovery_v2.dat";
const uint32_t RECOVERY_MAGIC = 0x52323031UL; // R201
const uint32_t RECOVERY_REASON_FORCED_OTA = 0xF001UL;
struct RecoveryRecord {
    uint32_t magic;
    uint32_t reason;
    uint32_t exccause;
    uint32_t epc1;
    uint32_t epc2;
    uint32_t epc3;
    uint32_t excvaddr;
    uint32_t depc;
    uint32_t checksum;
};
RecoveryRecord recoveryIncident = {};
RecoveryRecord currentBootReset = {};
bool recoveryMode = false;
bool recoveryNormalRestartPending = false;
unsigned long recoveryNormalRestartRequestedMs = 0;
const unsigned long RECOVERY_NORMAL_RESTART_DELAY_MS = 750UL;
// Manual ESP restart does not clear OTA-only Recovery or permit fan operation.
bool recoveryEspRestartPending = false;
unsigned long recoveryEspRestartRequestedMs = 0;
const unsigned long RECOVERY_ESP_RESTART_DELAY_MS = 750UL;
bool recoveryIncidentPersistent = false;
bool recoveryOtaInProgress = false;
bool recoveryResetClearFailed = false;
bool recoveryOtaClearFailed = false;
bool forceRecoveryRestartPending = false;
unsigned long forceRecoveryRestartRequestedMs = 0;
const unsigned long FORCE_RECOVERY_RESTART_DELAY_MS = 1000UL;
unsigned long recoveryWifiRetryMs = 0;
unsigned long recoveryDisplayMs = 0;
const unsigned long RECOVERY_WIFI_RETRY_MS = 15000UL;

// Το DEVICE_ID μπορεί να έχει "_" για τα MQTT topics.
// Για ArduinoOTA/mDNS το μετατρέπουμε σε hostname με "-".
// Παράδειγμα: jsy_house -> jsy-house
String otaHostname;


// =====================================================
// ΤΙΜΟΛΟΓΙΟ Ζ / Ζ1-Ζ2 ΚΑΙ ΣΗΜΑΣΙΑ ΚΑΤΑΧΩΡΗΣΕΩΝ ΔΕΗ
//
// Υπάρχουν δύο ανεξάρτητες έννοιες:
//
// 1) "Καταχώρηση Ένδειξης Μετρητή ΔΕΗ" (INTERMEDIATE / ek):
//    - γίνεται για το ΤΩΡΑ,
//    - παίρνει αυτόματα ώρα/ημερομηνία ESP,
//    - δένει την ένδειξη με τα ίδιας στιγμής internal Import/Export,
//    - είναι το anchor από το οποίο υπολογίζεται το "Εκτίμηση τώρα".
//
// 2) "Καταχώρηση Μέτρησης ΔΕΗ" (FINAL / tk):
//    - μπορεί να εισαχθεί αργότερα με την πραγματική ημερομηνία μέτρησης,
//    - είναι μόνο reference/comparison,
//    - ΔΕΝ ξαναδένει παλιούς ESP counters και ΔΕΝ μετακινεί το estimate anchor.
//
// Η "ΔΕΗ τελευταία" επιλέγεται με βάση την πιο πρόσφατη ΠΡΑΓΜΑΤΙΚΗ
// ημερομηνία μέτρησης, όχι τη σειρά εισαγωγής.
// Διαφορά = Εκτίμηση τώρα - ΔΕΗ τελευταία.
//
// Στο μονοζωνικό χρησιμοποιείται Ζ.
// Στο διζωνικό:
//   Ζ1 = κανονική ζώνη
//   Ζ2 = μειωμένη ζώνη
// Το estimate εξελίσσεται με signed net ενέργεια: Import - Export.
//
// ΩΡΑΡΙΟ Ζ2:
// Χειμερινή περίοδος (Νοέμβριος - Μάρτιος):
//   02:00-05:00 και 12:00-15:00
//
// Θερινή περίοδος (Απρίλιος - Οκτώβριος):
//   02:00-04:00 και 11:00-15:00
//
// Όλες οι άλλες ώρες είναι Ζ1.
//
// Η ώρα έρχεται από NTP. Μετά τον πρώτο έγκυρο συγχρονισμό,
// το system clock του ESP8266 συνεχίζει να λειτουργεί χωρίς Internet
// όσο το ESP παραμένει τροφοδοτημένο.
//
// Χωρίς battery-backed RTC, μετά από πλήρη διακοπή τροφοδοσίας
// δεν μπορεί να είναι γνωστή με απόλυτη ακρίβεια η ώρα μέσα στο κενό.
// Όταν επανέλθει NTP, το accounting κατανέμει το μη καταγεγραμμένο delta
// χρονικά στις ζώνες ως την καλύτερη δυνατή εκτίμηση.
// =====================================================

bool isSystemTimeValid() {
    return time(nullptr) > 1600000000;
}









void formatCurrentDateTime(char* out, size_t outSize) {
    if (!out || outSize == 0) return;
    out[0] = '\0';
    if (!isSystemTimeValid()) {
        snprintf(out, outSize, "--");
        return;
    }
    time_t nowValue = time(nullptr);
    struct tm localTm;
    if (!localtime_r(&nowValue, &localTm)) {
        snprintf(out, outSize, "--");
        return;
    }
    // Numeric formatting avoids linking the newlib locale tables into small
    // headless applications just to display an ISO-style timestamp.
    const int n = snprintf_P(out, outSize, PSTR("%04d-%02d-%02d %02d:%02d:%02d"),
        localTm.tm_year + 1900, localTm.tm_mon + 1, localTm.tm_mday,
        localTm.tm_hour, localTm.tm_min, localTm.tm_sec);
    if (n < 0 || (size_t)n >= outSize) out[0] = '\0';
}

// ==========================================================
// MODULE: STORAGE / ACCOUNTING
// ==========================================================

// =====================================================
// APPLICATION STORAGE INTEGRATION
// =====================================================


uint32_t oledDiagnosticChecksum(const RtcOledDiagnostic& record) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&record);
    uint32_t result = 2166136261UL;
    for (size_t i = 0; i < offsetof(RtcOledDiagnostic, checksum); ++i) {
        result = (result ^ bytes[i]) * 16777619UL;
    }
    return result;
}

void persistOledDiagnostic() {
    RtcOledDiagnostic record = {};
    static_assert(sizeof(record) % 4 == 0, "RTC OLED record must be word aligned");
    static_assert(RTC_OLED_DIAGNOSTIC_OFFSET * 4 + sizeof(record) <= 512,
                  "RTC OLED record must fit user memory");
    static_assert(RTC_OLED_DIAGNOSTIC_OFFSET >= RTC_CRASH_TRACE_OFFSET +
                  sizeof(RtcCrashTrace) / 4, "RTC OLED and crash trace overlap");
    if (oledDiagnosticCount) {
        record.magic = RTC_OLED_DIAGNOSTIC_MAGIC;
        record.message = oledDiagnostics[oledDiagnosticCount - 1]; // Latest cause on reboot.
        record.checksum = oledDiagnosticChecksum(record);
    }
    ESP.rtcUserMemoryWrite(RTC_OLED_DIAGNOSTIC_OFFSET,
                          reinterpret_cast<uint32_t*>(&record), sizeof(record));
}

void loadOledDiagnostic() {
    RtcOledDiagnostic record = {};
    if (!ESP.rtcUserMemoryRead(RTC_OLED_DIAGNOSTIC_OFFSET,
                             reinterpret_cast<uint32_t*>(&record), sizeof(record)) ||
        record.magic != RTC_OLED_DIAGNOSTIC_MAGIC ||
        record.checksum != oledDiagnosticChecksum(record) ||
        record.message.code < OLED_DIAG_WIFI || record.message.code > OLED_DIAG_RECOVERY_CLEAR ||
        !memchr(record.message.title, '\0', sizeof(record.message.title)) ||
        !memchr(record.message.detail, '\0', sizeof(record.message.detail))) return;
    oledDiagnostics[0] = record.message;
    oledDiagnosticCount = 1;
    oledDiagnosticStarted = false; // Replay gets four readable seconds.
}

void queueOledDiagnostic(uint32_t code, const char* title, const char* detail) {
    if (code < OLED_DIAG_WIFI || code > OLED_DIAG_RECOVERY_CLEAR) return;
    for (uint8_t i = 0; i < oledDiagnosticCount; ++i) {
        if (oledDiagnostics[i].code == code) return; // Preserve the first cause.
    }
    if (oledDiagnosticCount >= OLED_DIAGNOSTIC_SLOTS) return;
    OledDiagnostic& message = oledDiagnostics[oledDiagnosticCount++];
    memset(&message, 0, sizeof(message));
    message.code = code;
    snprintf(message.title, sizeof(message.title), "%s", title ? title : "Diagnostic failure");
    snprintf(message.detail, sizeof(message.detail), "%s", detail ? detail : "Cause unavailable");
    if (oledDiagnosticCount == 1) {
        oledDiagnosticStarted = false;
    }
    persistOledDiagnostic();
}

bool drawOledDiagnostic(unsigned long now) {
    if (!bootOledReady || !oledDiagnosticCount) return false;
    if (oledDiagnosticStarted && now - oledDiagnosticStartedMs >= OLED_DIAGNOSTIC_HOLD_MS) {
        for (uint8_t i = 1; i < oledDiagnosticCount; ++i) oledDiagnostics[i - 1] = oledDiagnostics[i];
        --oledDiagnosticCount;
        oledDiagnosticStarted = false;
        persistOledDiagnostic();
        if (!oledDiagnosticCount) return false;
    }
    if (!oledDiagnosticStarted) {
        oledDiagnosticStarted = true;
        oledDiagnosticStartedMs = now;
    }
    const OledDiagnostic& message = oledDiagnostics[0];
    char line[33] = {};
    if (!platformDisplay.available()) {
        oledDiagnosticLastFrameMs = now;
        return false; // Lifecycle above still expires/persists the queue without OLED.
    }
    platformDisplay.clear();
    platformDisplay.font(DisplayFont::Small5x7);
    platformDisplay.text(0, 10, message.title);
    platformDisplay.font(DisplayFont::Tiny4x6);
    memcpy(line, message.detail, 32);
    line[32] = '\0';
    platformDisplay.text(0, 20, line);
    memcpy(line, message.detail + 32, 32);
    line[32] = '\0';
    platformDisplay.text(0, 30, line);
    if (credentialPortalActive) {
        // Keep setup access visible while retaining the diagnostic above it.
        platformDisplay.text(0, 40, credentialApName.c_str());
        snprintf(line, sizeof(line), "WiFi open PIN:%s", AP_WEB_PIN);
        platformDisplay.text(0, 50, line);
        platformDisplay.text(0, 60, PLATFORM_SETUP_IP_TEXT "/setup");
    } else {
        if (normalMeterInitialized && !recoveryMode) {
            applicationFormatSummary(line, sizeof(line));
        } else {
            snprintf(line, sizeof(line), "%s", recoveryMode ? "OTA RECOVERY / meter paused" : "Setup / meter not initialized");
        }
        platformDisplay.text(0, 40, line);
        const unsigned long left = OLED_DIAGNOSTIC_HOLD_MS - (now - oledDiagnosticStartedMs);
        snprintf(line, sizeof(line), "Hold %lus; pending %u", (left + 999UL) / 1000UL,
                 (unsigned)(oledDiagnosticCount - 1));
        platformDisplay.text(0, 50, line);
        platformDisplay.text(0, 60, recoveryMode ? "FLASH:AP setup RST:normal" : "FLASH:AP setup");
    }
    platformDisplay.present();
    oledDiagnosticLastFrameMs = millis();
    return true;
}

// Startup-only diagnostics: no Strings, delay(), heap allocation or flash writes.
// The CP is the operation ABOUT TO START. RTC separately identifies a hang while
// transmitting the checkpoint itself (stage=100, detail=the pending CP).
void bootCheckpoint(uint32_t stage) {
    if (!bootCheckpointsActive) return;
    bootLastCheckpoint = stage;
    markCrashStage(stage, ESP.getFreeHeap());
    if (!bootOledReady) return; // OLED init can only be recorded in RTC.
    if (drawOledDiagnostic(millis())) return;

    char heading[28];
    char memoryLine[32];
    snprintf(heading, sizeof(heading), "BOOT v%s  CP%lu", VERSION, (unsigned long)stage);
    snprintf(memoryLine, sizeof(memoryLine), "H:%lu T:%lus",
             (unsigned long)ESP.getFreeHeap(), (unsigned long)(millis() / 1000UL));
    markCrashStage(CRASH_STAGE_BOOT_OLED_WRITE, stage);
    platformDisplay.clear();
    platformDisplay.font(DisplayFont::Medium6x12);
    platformDisplay.text(0, 12, heading);
    platformDisplay.text(0, 28, "Starting:");
    platformDisplay.font(DisplayFont::Small5x7);
    platformDisplay.text(0, 43, crashStageLabel(stage));
    platformDisplay.text(0, 60, memoryLine);
    platformDisplay.present();
    markCrashStage(stage, ESP.getFreeHeap());
}

void finishBootCheckpoints(uint32_t stage) {
    if (!bootCheckpointsActive) return;
    bootLastCheckpoint = stage;
    markCrashStage(stage, ESP.getFreeHeap());
    bootCheckpointsActive = false; // Do not overwrite live/AP/Recovery screens.
}

void cs() {
    platformDisplay.clear();
    platformDisplay.present();
}

void doublePrintf(int fontCode, int line, int pos, const char* fmt, ...) {
    if (fontCode != 420) return;

    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    platformDisplay.font(DisplayFont::Normal10);
    int x = (pos - 1) * 8;
    int y = line * 15;
    platformDisplay.text(x, y, buf);
    platformDisplay.present();
}

// ==========================================================
// END MODULE: STORAGE / ACCOUNTING
// ==========================================================

// ==========================================================
// MODULE: MQTT / TLS / BROKER
// ==========================================================

// ===== MQTT CALLBACK: INCOMING APP/ADMIN REQUESTS =====
// Όταν ο χρήστης στείλει "update" στο MQTT_REQUEST_TOPIC,
// σηκώνεται μόνο ένα flag. Η ανάγνωση του επιλεγμένου μετρητή γίνεται αργότερα στο loop().
bool isValidMd5Hex(const String& value) {
    if (value.length() != 32) return false;
    for (size_t i = 0; i < 32; ++i) {
        char c = value[i];
        if (!((c >= '0' && c <= '9') ||
              (c >= 'a' && c <= 'f') ||
              (c >= 'A' && c <= 'F'))) {
            return false;
        }
    }
    return true;
}

bool parseHttpsHostPort(const char* url, char* host, size_t hostSize, uint16_t& port) {
    if (url == nullptr || host == nullptr || hostSize < 2) return false;
    const char prefix[] = "https://";
    if (strncmp(url, prefix, sizeof(prefix) - 1) != 0) return false;

    const char* begin = url + sizeof(prefix) - 1;
    const char* end = strchr(begin, '/');
    if (end == nullptr) end = begin + strlen(begin);
    if (end <= begin) return false;

    const char* colon = nullptr;
    for (const char* p = begin; p < end; ++p) {
        if (*p == ':') colon = p;
    }

    const char* hostEnd = colon ? colon : end;
    size_t len = (size_t)(hostEnd - begin);
    if (len == 0 || len >= hostSize) return false;

    memcpy(host, begin, len);
    host[len] = '\0';
    port = 443;

    if (colon) {
        unsigned long parsed = strtoul(colon + 1, nullptr, 10);
        if (parsed == 0 || parsed > 65535UL) return false;
        port = (uint16_t)parsed;
    }
    return true;
}







// Stage validation only; network/flash work and replies remain outside callback.
bool stageRemoteHttpsOtaCommand(const String& command) {
    bool legacy = command.startsWith("ota_https|");
    if (!legacy && !command.startsWith("ota_https_v2|")) return false;

    mqttAdminOtaError = legacy ? "ota_protocol_required" : "invalid_parameters";
    mqttAdminOtaSize = 0;
    mqttAdminOtaUrl[0] = '\0';
    mqttAdminOtaMd5[0] = '\0';
    mqttAdminCommand = MQTT_ADMIN_OTA_REJECTED;
    mqttAdminCommandPending = true;
    if (legacy) return true;

    int p1 = command.indexOf('|');
    int p2 = command.indexOf('|', p1 + 1);
    int p3 = command.indexOf('|', p2 + 1);
    int p4 = command.indexOf('|', p3 + 1);
    if (p1 < 0 || p2 <= p1 || p3 <= p2 || p4 <= p3 ||
        command.indexOf('|', p4 + 1) >= 0) return true;

    String rawSize = command.substring(p1 + 1, p2);
    String rawMd5 = command.substring(p2 + 1, p3);
    String rawUrl = command.substring(p3 + 1, p4);
    String rawProfile = command.substring(p4 + 1);
    rawSize.trim(); rawMd5.trim(); rawUrl.trim(); rawProfile.trim();

    if (rawProfile != OTA_PROFILE_ID) {
        mqttAdminOtaError = "profile_mismatch";
        return true;
    }
    if (rawSize.length() == 0 || rawSize.length() > 10) return true;
    for (size_t i = 0; i < rawSize.length(); ++i)
        if (rawSize[i] < '0' || rawSize[i] > '9') return true;
    char* end = nullptr;
    unsigned long parsedSize = strtoul(rawSize.c_str(), &end, 10);
    bool valuesOk = end && *end == '\0' &&
        parsedSize >= 65536UL && parsedSize <= ESP.getFreeSketchSpace() &&
        isValidMd5Hex(rawMd5) && rawUrl.startsWith("https://") &&
        rawUrl.length() > 12 && rawUrl.length() < REMOTE_OTA_URL_BYTES;
    if (!valuesOk) return true;

    mqttAdminOtaSize = (uint32_t)parsedSize;
    rawMd5.toLowerCase();
    rawMd5.toCharArray(mqttAdminOtaMd5, sizeof(mqttAdminOtaMd5));
    rawUrl.toCharArray(mqttAdminOtaUrl, sizeof(mqttAdminOtaUrl));
    mqttAdminCommand = MQTT_ADMIN_REMOTE_OTA;
    return true;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, MQTT_ADMIN_REQUEST_TOPIC) == 0) {
        String adminMessage;
        for (unsigned int i = 0; i < length; ++i) {
            adminMessage += (char)payload[i];
        }
        adminMessage.trim();

        if (mqttAdminCommandPending) return;

        if (applicationStageCommand(adminMessage)) return;

        if (adminMessage.equalsIgnoreCase("restart_esp")) {
            mqttAdminCommand = MQTT_ADMIN_RESTART;
            mqttAdminCommandPending = true;
            return;
        }

        if (stageRemoteHttpsOtaCommand(adminMessage)) return;

        mqttAdminCommand = MQTT_ADMIN_INVALID;
        mqttAdminCommandPending = true;
        return;
    }
    if (strcmp(topic, MQTT_REQUEST_TOPIC) != 0) return;

    // Hot path: APP auto-refreshes every minute. Avoid Arduino String
    // allocation/free churn here because, on the small ESP8266 heap, repeated
    // temporary allocations can reduce the largest free block over long runs.
    unsigned int first = 0;
    unsigned int last = length;
    while (first < last &&
           (payload[first] == ' ' || payload[first] == '\t' ||
            payload[first] == '\r' || payload[first] == '\n')) ++first;
    while (last > first &&
           (payload[last - 1] == ' ' || payload[last - 1] == '\t' ||
            payload[last - 1] == '\r' || payload[last - 1] == '\n')) --last;

    static const char updateWord[] = "update";
    bool isUpdate = (last - first) == sizeof(updateWord) - 1;
    for (unsigned int i = 0; isUpdate && i < sizeof(updateWord) - 1; ++i) {
        char c = (char)payload[first + i];
        if (c >= 'A' && c <= 'Z') c = (char)(c + ('a' - 'A'));
        if (c != updateWord[i]) isUpdate = false;
    }

    if (isUpdate) {
        // Keep even a closely spaced request pending. A second app/window may
        // have subscribed after the previous broadcast and still needs a reply.
        mqttUpdateRequested = true;
    }
}

void resetMqttUpdateRetry() {
    mqttUpdateRetryStep = 0;
    mqttUpdateRetryDelayMs = MQTT_UPDATE_MIN_INTERVAL_MS;
}

void scheduleMqttUpdateRetry() {
    uint8_t step = mqttUpdateRetryStep;
    if (step >= MQTT_UPDATE_RETRY_STEP_COUNT) step = MQTT_UPDATE_RETRY_STEP_COUNT - 1;
    mqttUpdateRetryDelayMs = MQTT_UPDATE_RETRY_STEPS_MS[step];
    if (mqttUpdateRetryStep + 1 < MQTT_UPDATE_RETRY_STEP_COUNT) mqttUpdateRetryStep++;
}

bool mqttUpdateResponseDue(unsigned long now) {
    return mqttUpdateRequested &&
           (lastMqttUpdatePublishMs == 0 ||
            now - lastMqttUpdatePublishMs >= mqttUpdateRetryDelayMs);
}

void resetMqttRuntimeFailureStreak() {
    mqttRuntimeConnectFailureStreak = 0;
}

void noteMqttRuntimeTransportFailure() {
    // Only self-heal a unit that has already proved its broker credentials and
    // still reports router Wi-Fi as connected. DNS failures are handled by the
    // normal retry path and do not trigger a LAN recycle.
    if (!mqttEverConnected || WiFi.status() != WL_CONNECTED ||
        mqttLastAttemptState != -2) {
        resetMqttRuntimeFailureStreak();
        return;
    }
    if (mqttRuntimeConnectFailureStreak < 0xFF) ++mqttRuntimeConnectFailureStreak;

    const uint32_t freeHeap = ESP.getFreeHeap();
    const uint16_t maxBlock = ESP.getMaxFreeBlockSize();
    const bool localResourcePressure =
        mqttLastSslErrorCode == -1000 ||
        freeHeap < (uint32_t)mqttTlsRecvConfiguredBytes + 7000UL ||
        maxBlock < (uint16_t)(mqttTlsRecvConfiguredBytes + 1536U);
    const uint8_t threshold = localResourcePressure
        ? MQTT_RUNTIME_RECOVERY_FAILURES_LOW_HEAP
        : MQTT_RUNTIME_RECOVERY_FAILURES_GENERIC;

    if (mqttRuntimeConnectFailureStreak >= threshold) {
        networkStackRecoveryPending = true;
    }
}

// ===== MQTT CONFIGURATION VALIDATION =====
bool mqttSettingsReady() {
    return mqttHost[0] != '\0' &&
           mqttUser[0] != '\0' &&
           mqttPassword[0] != '\0' &&
           mqttPort != 0;
}

// ===== MQTT / TLS INITIALIZATION =====
void configureMqtt() {
    // TLS στην πόρτα 8883.
    // Για την αρχική λειτουργία χρησιμοποιείται setInsecure(),
    // δηλαδή κρυπτογράφηση χωρίς έλεγχο CA certificate.
    mqttNet.setInsecure();
    mqttNet.setTimeout(MQTT_TLS_TIMEOUT_MS);

    // BearSSL default RX = 16384 (+325 overhead). On ESP8266 that can leave
    // too little contiguous heap for TLS/Web coexistence. configureMqtt() runs
    // before Wi-Fi joins; the actual RX size is selected after an MFLN capability
    // probe in maintainMqtt(), BEFORE mqttClient.connect().
    mqttNet.setBufferSizes(16384, MQTT_TLS_TX_BYTES);

    mqttClient.setServer(mqttHost, mqttPort);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setKeepAlive(30);
    mqttClient.setSocketTimeout(3);

    // DDS238 και JSY χρησιμοποιούν το ίδιο packet ceiling. Δεν χρειάζεται
    // ξεχωριστό compile-time branch εδώ· το JSY JSON παραμένει εντός 1024 B.
    mqttClient.setBufferSize(MQTT_PACKET_BUFFER_BYTES);
}

// Descriptions live in flash and must be read via writeCredentialProgmem().
// Keeping these outside RAM matters while MQTT/BearSSL is connected.
const char* mqttStateDescription(int code) {
    // Explicit branches avoid RAM-resident compiler switch lookup tables.
    if (code == -4) return PSTR("MQTT reply timed out");
    if (code == -3) return PSTR("MQTT connection was lost");
    if (code == -2) return PSTR("Connection failed; network or TLS may be involved");
    if (code == -1) return PSTR("MQTT client is disconnected");
    if (code == 0) return PSTR("MQTT connection accepted");
    if (code == 1) return PSTR("Broker rejected the MQTT protocol version");
    if (code == 2) return PSTR("Broker rejected the client ID");
    if (code == 3) return PSTR("Broker reported that the service is unavailable");
    if (code == 4) return PSTR("Broker rejected the username or password");
    if (code == 5) return PSTR("Broker denied access; check credentials and permissions");
    return PSTR("Unknown MQTT status");
}

const char* wifiStatusDescription(int code) {
    // Explicit branches avoid RAM-resident compiler switch lookup tables.
    if (code == 0) return PSTR("Idle or connection attempt in progress");
    if (code == 1) return PSTR("Configured Wi-Fi network was not found");
    if (code == 2) return PSTR("Wi-Fi scan completed");
    if (code == 3) return PSTR("Connected to the router");
    if (code == 4) return PSTR("Connection attempt failed; see recorded disconnect reason");
    if (code == 5) return PSTR("Connection to the router was lost");
    if (code == 6) return PSTR("Disconnected from the router");
    if (code == 255) return PSTR("Wi-Fi interface unavailable");
    return PSTR("Unknown Wi-Fi status");
}

const char* wifiDisconnectDescription(uint16_t code) {
    // Explicit branches avoid RAM-resident compiler switch lookup tables.
    if (code == 0) return PSTR("No disconnect reason recorded");
    if (code == 1) return PSTR("Unspecified Wi-Fi disconnect");
    if (code == 2) return PSTR("Wi-Fi authentication expired");
    if (code == 3) return PSTR("Wi-Fi authentication was ended");
    if (code == 4) return PSTR("Router association expired");
    if (code == 5) return PSTR("Router rejected association: too many clients");
    if (code == 6 || code == 9) return PSTR("Router requires Wi-Fi authentication");
    if (code == 7) return PSTR("Client was not associated with the router");
    if (code == 8) return PSTR("Router association was ended");
    if (code == 10 || code == 11) return PSTR("Router rejected radio capabilities");
    if (code == 13 || code == 17) return PSTR("Invalid or inconsistent Wi-Fi security information");
    if (code == 14) return PSTR("Wi-Fi message integrity check failed");
    if (code == 15 || code == 204) return PSTR("Wi-Fi security handshake timed out; password or radio link may be involved");
    if (code == 16) return PSTR("Wi-Fi group key update timed out");
    if (code == 18 || code == 19 || code == 20 || code == 21 || code == 22 || code == 24) return PSTR("Router rejected the Wi-Fi security configuration");
    if (code == 23) return PSTR("Enterprise Wi-Fi authentication failed");
    if (code == 200) return PSTR("Router beacons were lost; signal or router interruption may be involved");
    if (code == 201) return PSTR("Configured Wi-Fi network was not found");
    if (code == 202) return PSTR("Wi-Fi authentication failed; check credentials and router settings");
    if (code == 203) return PSTR("Router association failed");
    return PSTR("Unknown SDK disconnect reason");
}

const char* apFailureDescription(uint8_t code) {
    // Explicit branches avoid RAM-resident compiler switch lookup tables.
    if (code == 0) return PSTR("No AP startup failure recorded");
    if (code == 1) return PSTR("Could not enable the AP radio mode");
    if (code == 2) return PSTR("Could not configure the AP IP address");
    if (code == 3) return PSTR("Could not start the setup access point");
    return PSTR("Unknown AP startup failure");
}

const char* networkOperationDescription() {
    if (credentialPortalOnly) return PSTR("AP-only setup: router Wi-Fi and MQTT are intentionally paused");
    if (recoveryMode) return PSTR("OTA-only recovery: MQTT measurements are paused");
    if (WiFi.status() != WL_CONNECTED) return PSTR("Waiting for the router Wi-Fi connection before MQTT can connect");
    if (!mqttSettingsReady()) return PSTR("MQTT settings are incomplete");
    if (mqttClient.connected()) return PSTR("Router Wi-Fi and MQTT are connected");
    return PSTR("Router Wi-Fi is connected; MQTT is disconnected and will retry");
}

const char* dnsLookupDescription(bool attempted, bool usable) {
    if (!attempted) {
        if (WiFi.status() != WL_CONNECTED) return PSTR("Not run: router Wi-Fi is disconnected");
        return PSTR("Not run: MQTT host is not configured");
    }
    return usable ? PSTR("Lookup succeeded with a usable address")
                  : PSTR("Lookup failed or returned an unusable address");
}

// ===== MQTT CONNECT / RECONNECT / SUBSCRIBE =====
void scheduleMqttRetry() {
    if (mqttEverConnected) {
        uint8_t step = mqttRetryStep;
        if (step >= MQTT_RUNTIME_RETRY_STEP_COUNT) {
            step = MQTT_RUNTIME_RETRY_STEP_COUNT - 1;
        }
        mqttRetryDelayMs = MQTT_RUNTIME_RETRY_STEPS_MS[step];
        if (mqttRetryStep + 1 < MQTT_RUNTIME_RETRY_STEP_COUNT) mqttRetryStep++;
    } else {
        uint8_t step = mqttRetryStep;
        if (step >= MQTT_RETRY_STEP_COUNT) step = MQTT_RETRY_STEP_COUNT - 1;
        mqttRetryDelayMs = MQTT_RETRY_STEPS_MS[step];
        if (mqttRetryStep + 1 < MQTT_RETRY_STEP_COUNT) mqttRetryStep++;
    }
    // Cooldown starts AFTER the blocking DNS/TCP/TLS operation, not before it.
    lastMqttReconnectMs = millis();
}

void maintainMqtt(unsigned long now) {
    if (WiFi.status() != WL_CONNECTED || !mqttSettingsReady()) return;

    if (mqttClient.connected()) {
        mqttRetryStep = 0;
        mqttRetryDelayMs = mqttEverConnected
            ? MQTT_RUNTIME_RETRY_STEPS_MS[0]
            : MQTT_RETRY_STEPS_MS[0];

        // PubSubClient::loop() is the MQTT keepalive/liveness check. v4.01
        // ignored its return value; after an Internet-only router outage the
        // stale TLS transport could therefore remain unusable until reboot.
        // Explicitly tear it down and enter fast runtime recovery.
        if (!mqttClient.loop()) {
            mqttNet.stop(0);
            mqttRetryStep = 0;
            mqttRetryDelayMs = MQTT_RUNTIME_RETRY_STEPS_MS[0];
            lastMqttReconnectMs = millis();
        }
        return;
    }

    if (lastMqttReconnectMs != 0 &&
        now - lastMqttReconnectMs < mqttRetryDelayMs) {
        return;
    }

    bootFirstMqttAttempted = true;

    // A previously healthy MQTT connection that died while Wi-Fi stayed
    // associated may leave BearSSL/TCP state behind. Start every runtime
    // recovery attempt from a clean transport before DNS/TLS reconnect.
    if (mqttEverConnected) mqttNet.stop(0);

    mqttConnectAttempts++;
    mqttLastAttemptMs = millis();
    mqttLastConnectSucceeded = false;
    mqttLastStatusPublishSucceeded = false;
    mqttLastRequestSubscribeSucceeded = false;
    mqttLastAdminSubscribeSucceeded = false;
    mqttLastSslErrorCode = 0;
    mqttLastSslErrorText[0] = '\0';

    // Explicitly bounded DNS preflight also warms the cache for the hostname
    // TLS connection below (keep the hostname/SNI, not an IP-only TLS connect).
    bootCheckpoint(CRASH_STAGE_BOOT_MQTT_DNS);
    IPAddress brokerIp;
    if (WiFi.hostByName(mqttHost, brokerIp, MQTT_DNS_TIMEOUT_MS) != 1) {
        mqttLastAttemptState = -2;
        snprintf(mqttLastSslErrorText, sizeof(mqttLastSslErrorText), "MQTT DNS unavailable");
        mqttConnectFailures++;
        resetMqttRuntimeFailureStreak();
        queueOledDiagnostic(OLED_DIAG_MQTT, "MQTT DNS FAILED", "Broker hostname lookup failed; retrying");
        scheduleMqttRetry();
        return;
    }

    // MFLN preflight uses a temporary plain TCP socket with a partial
    // ClientHello, NOT a second BearSSL instance and NO MQTT credentials.
    // A broker that supports a 4096-byte TLS fragment lets us save 4KB
    // of heap. If unsupported/temporarily unavailable, retain the safe
    // compatibility buffer and recheck periodically.
    // Never force a small buffer without MFLN.
    if (!mqttMflnChecked ||
        (!mqttMflnSupported &&
         millis() - mqttMflnLastProbeMs >= MQTT_MFLN_RECHECK_MS)) {
        bootCheckpoint(CRASH_STAGE_MFLN_PROBE);
        markCrashStage(CRASH_STAGE_MFLN_PROBE, ESP.getFreeHeap());
        mqttMflnChecked = true;
        mqttMflnProbeCount++;
        mqttMflnLastProbeMs = millis();
        mqttMflnSupported =
            BearSSL::WiFiClientSecure::probeMaxFragmentLength(
                brokerIp, mqttPort, MQTT_TLS_SMALL_RX_BYTES);
        mqttMflnProbeDurationMs = millis() - mqttMflnLastProbeMs;
        mqttMflnLastProbeMs = millis();
        mqttSmallTlsBufferActive = mqttMflnSupported;
        mqttCompatTlsBufferActive = !mqttMflnSupported;
        mqttTlsRecvConfiguredBytes = mqttSmallTlsBufferActive
            ? MQTT_TLS_SMALL_RX_BYTES
            : MQTT_TLS_COMPAT_RX_BYTES;
        mqttNet.setBufferSizes(mqttTlsRecvConfiguredBytes, MQTT_TLS_TX_BYTES);
    }

    char clientId[sizeof(mqttDeviceId) + sizeof("esp-") - 1];
    snprintf_P(clientId, sizeof(clientId), PSTR("esp-%s"), mqttDeviceId);

    bootCheckpoint(CRASH_STAGE_MQTT_CONNECT);
    markCrashStage(CRASH_STAGE_MQTT_CONNECT, ESP.getFreeHeap());

    uint32_t connectFreeHeap = ESP.getFreeHeap();
    uint16_t connectMaxBlock = ESP.getMaxFreeBlockSize();
    mqttConnectHeapAtAttempt = connectFreeHeap;
    mqttConnectBlockAtAttempt = connectMaxBlock;
    const uint32_t connectHeapFloor =
        (uint32_t)mqttTlsRecvConfiguredBytes + 6500UL;
    if (connectFreeHeap < connectHeapFloor ||
        connectMaxBlock < (uint16_t)(mqttTlsRecvConfiguredBytes + 1024U)) {
        ++mqttConnectPreflightDeferrals;
        mqttLastSslErrorCode = -1000;
        snprintf(mqttLastSslErrorText, sizeof(mqttLastSslErrorText),
                 "TLS connect skipped: low heap RX=%u free=%lu max=%u",
                 (unsigned)mqttTlsRecvConfiguredBytes,
                 (unsigned long)connectFreeHeap,
                 (unsigned)connectMaxBlock);
        mqttLastAttemptState = -2;
        mqttConnectFailures++;
        noteMqttRuntimeTransportFailure();
        queueOledDiagnostic(OLED_DIAG_MQTT, "MQTT TLS LOW HEAP", "TLS attempt deferred; network self-heal if persistent");
        scheduleMqttRetry();
        return;
    }

    // Core 2.7.2 has a separate BearSSL/TCP timeout; PubSubClient's 3s timeout
    // alone only bounds MQTT packet reads, not the transport handshake.
    mqttNet.setTimeout(MQTT_TLS_TIMEOUT_MS);
    bool ok = mqttClient.connect(
        clientId,
        mqttUser,
        mqttPassword,
        MQTT_STATUS_TOPIC,
        0,
        true,
        "offline"
    );
    mqttLastConnectSucceeded = ok;
    mqttLastAttemptState = mqttClient.state();
    mqttMflnNegotiatedAtLastConnect =
        ok && (mqttNet.getMFLNStatus() != 0);

    // Guard against endpoint changes between preflight and connection:
    // no normal MQTT traffic is accepted with an unnegotiated 4KB RX.
    if (ok && mqttSmallTlsBufferActive &&
        !mqttMflnNegotiatedAtLastConnect) {
        mqttLastSslErrorCode = 0;
        snprintf(mqttLastSslErrorText, sizeof(mqttLastSslErrorText),
                 "MFLN probe passed but live TLS did not negotiate 4096; "
                 "connection closed for safety");
        mqttClient.disconnect();
        mqttLastConnectSucceeded = false;
        mqttLastAttemptState = mqttClient.state();
        ok = false;
        // Probe again on the next scheduled reconnect.
        mqttMflnChecked = false;
    }

    // Snapshot actual BearSSL failure ASAP. Error 0 on a failed connect
    // means the SSL error is unavailable, not that TLS succeeded.
    if (!ok && mqttLastSslErrorText[0] == '\0') {
        mqttLastSslErrorCode =
            mqttNet.getLastSSLError(mqttLastSslErrorText,
                                    sizeof(mqttLastSslErrorText));
    }

    if (ok) {
        mqttEverConnected = true;
        resetMqttRuntimeFailureStreak();
        mqttRetryStep = 0;
        mqttRetryDelayMs = MQTT_RUNTIME_RETRY_STEPS_MS[0];
        mqttLastStatusPublishSucceeded =
            mqttClient.publish(MQTT_STATUS_TOPIC, "online", true);
        mqttLastRequestSubscribeSucceeded =
            mqttClient.subscribe(MQTT_REQUEST_TOPIC);
        mqttLastAdminSubscribeSucceeded =
            mqttClient.subscribe(MQTT_ADMIN_REQUEST_TOPIC);
    } else {
        mqttConnectFailures++;
        noteMqttRuntimeTransportFailure();
        char detail[64];
        snprintf(detail, sizeof(detail), "MQTT state=%d; SSL=%ld. Retrying",
                 mqttLastAttemptState, (long)mqttLastSslErrorCode);
        queueOledDiagnostic(OLED_DIAG_MQTT, "MQTT CONNECT FAILED", detail);
        mqttNet.stop(0);
        scheduleMqttRetry();
    }
}

// ===== MQTT HEALTH + LIVE MEASUREMENT PUBLISH =====
// v2.12: normal PubSubClient publish(), but JSON is built in a bounded
// stack buffer instead of allocating a second ~1KB heap buffer. This avoids
// the v2.11 MQTT_PUBLISH condition where only 1736 bytes of heap remained.
// Small retained discovery/health frame, also available without any meter.
// Identity derives from the station MAC, independent of site and credentials.
void initializeMqttIdentity() {
    uint8_t mac[6] = {};
    WiFi.macAddress(mac);
    snprintf_P(mqttDeviceId, sizeof(mqttDeviceId), PSTR("%s-%02X%02X%02X%02X%02X%02X"), DEVICE_ID,
        (unsigned)mac[0], (unsigned)mac[1], (unsigned)mac[2], (unsigned)mac[3], (unsigned)mac[4], (unsigned)mac[5]);
    snprintf_P(MQTT_STATE_TOPIC, sizeof(MQTT_STATE_TOPIC), PSTR(MQTT_TOPIC_PREFIX "%s/state"), mqttDeviceId);
    snprintf_P(MQTT_STATUS_TOPIC, sizeof(MQTT_STATUS_TOPIC), PSTR(MQTT_TOPIC_PREFIX "%s/status"), mqttDeviceId);
    snprintf_P(MQTT_REQUEST_TOPIC, sizeof(MQTT_REQUEST_TOPIC), PSTR(MQTT_TOPIC_PREFIX "%s/request"), mqttDeviceId);
    snprintf_P(MQTT_ADMIN_REQUEST_TOPIC, sizeof(MQTT_ADMIN_REQUEST_TOPIC), PSTR(MQTT_TOPIC_PREFIX "%s/admin/request"), mqttDeviceId);
    snprintf_P(MQTT_ADMIN_RESPONSE_TOPIC, sizeof(MQTT_ADMIN_RESPONSE_TOPIC), PSTR(MQTT_TOPIC_PREFIX "%s/admin/response"), mqttDeviceId);
    snprintf_P(MQTT_ADMIN_CSV_TOPIC, sizeof(MQTT_ADMIN_CSV_TOPIC), PSTR(MQTT_TOPIC_PREFIX "%s/admin/csv"), mqttDeviceId);
    snprintf_P(MQTT_HEALTH_TOPIC, sizeof(MQTT_HEALTH_TOPIC), PSTR(MQTT_TOPIC_PREFIX "%s/health"), mqttDeviceId);
    applicationInitializeTopics();
}

bool publishMeterHealth() {
    if (!mqttClient.connected()) return false;
    char payload[576];
    char age[16] = "null";
    if (meterHasGoodSample) snprintf(age, sizeof(age), "%lu", (unsigned long)(millis() - meterLastGoodMs));
    int n = snprintf_P(payload, sizeof(payload), PSTR("{\"meter\":\"%s\",\"device\":\"%s\",\"firmware\":\"%s\","
        "\"application\":\"%s\",\"platform_version\":\"%s\",\"driver\":\"%s\","
        "\"oled\":\"%s\",\"profile\":\"%s\",\"ota_protocol\":%u,"
        "\"chip_id\":%lu,\"dual_zone\":%s,\"meter_ok\":%s,\"read_attempted\":%s,"
        "\"has_sample\":%s,\"last_sample_age_ms\":%s,\"read_failures\":%lu,"
        "\"error\":\"%s\",\"rssi\":%d}"),
        DEVICE_KIND, mqttDeviceId, VERSION, APPLICATION_ID, PLATFORM_VERSION, DRIVER_ID,
        OLED_ID, OTA_PROFILE_ID, (unsigned)OTA_PROTOCOL_VERSION, (unsigned long)ESP.getChipId(),
        applicationDualZoneEnabled() ? "true" : "false", lastMeterReadOk ? "true" : "false",
        meterReadAttempted ? "true" : "false", meterHasGoodSample ? "true" : "false", age,
        (unsigned long)meterReadFailures,
        !meterReadAttempted ? "awaiting_first_read" : (lastMeterReadOk ? "" : PLATFORM_SENSOR_ERROR), WiFi.RSSI());
    if (n <= 0 || n >= (int)sizeof(payload)) return false;
    bool ok = mqttClient.publish(MQTT_HEALTH_TOPIC, payload, true);
    if (ok) {
        meterHealthDirty = false; meterHealthConnectionPublished = true;
        meterLastHealthPublishMs = millis();
    }
    return ok;
}

void serviceMeterHealth(unsigned long now) {
    if (!mqttClient.connected()) { meterHealthConnectionPublished = false; return; }
    if (meterHealthDirty || !meterHealthConnectionPublished || now - meterLastHealthPublishMs >= 30000UL)
        publishMeterHealth();
}



// Keep the large sample buffer out of the sensor-error publish stack.
bool publishMqttMeasurements() {
    if (!mqttClient.connected()) return false;
#if !PLATFORM_LOCAL_CONTROL_APPLICATION
    if (!lastMeterReadOk) return publishMeterHealth();
#endif
    return publishMqttSample();
}

// ==========================================================
// END MODULE: MQTT / TLS / BROKER
// ==========================================================

// ==========================================================
// MODULE: METER HEALTH / OLED ADAPTER
// ==========================================================

// =====================================================
// OLED / COMMON METER ADAPTERS
// =====================================================
// Every physical read updates health, including utility indication requests.
bool readMeterWithHealth() {
    bool ok = applicationReadMeasurements();
    if (!meterReadAttempted || ok != lastMeterReadOk) meterHealthDirty = true;
    meterReadAttempted = true; lastMeterReadOk = ok;
    if (ok) { meterHasGoodSample = true; meterLastGoodMs = millis(); }
    else if (meterReadFailures != 0xFFFFFFFFUL) ++meterReadFailures;
    return ok;
}

// ==========================================================
// END MODULE: METER HEALTH / OLED ADAPTER
// ==========================================================

// ==========================================================
// MODULE: LOCAL WEB
// ==========================================================

// ===== LOCAL WEB DASHBOARD / LOW-HEAP GUARDS =====
// Η πλήρης σελίδα στέλνεται μία φορά και οι live μετρήσεις ανανεώνονται από
// το /live. Τα writes γίνονται σε μικρά RAM slices, με heap/max-block guards,
// deadline και explicit socket close. Έτσι το Web δεν ανταγωνίζεται το
// BearSSL για μεγάλο συνεχόμενο heap block ούτε αφήνει abandoned TCP sockets.

bool webRootHeapHealthy(uint32_t* freeOut = nullptr,
                        uint16_t* maxBlockOut = nullptr,
                        uint8_t* fragmentationOut = nullptr) {
    uint32_t freeHeap = 0;
    uint16_t maxBlock = 0;
    uint8_t fragmentation = 0;
    ESP.getHeapStats(&freeHeap, &maxBlock, &fragmentation);
    if (freeOut) *freeOut = freeHeap;
    if (maxBlockOut) *maxBlockOut = maxBlock;
    if (fragmentationOut) *fragmentationOut = fragmentation;
    return freeHeap >= WEB_ROOT_MIN_HEAP_BYTES &&
           maxBlock >= WEB_ROOT_MIN_BLOCK_BYTES;
}

void closeRootClient(WiFiClient& client, bool graceful = false) {
    // In core 2.7.2 stop(0) means the default 300 ms flush, not immediate close.
    client.stop(graceful ? 250 : 1);
    webRootForcedCloseCount++;
    yield();
}

bool writeWebBytes(WiFiClient& client, const uint8_t* data, size_t length, bool emergency) {
    if (!data || !length) return true;
    const size_t sliceBytes = emergency ? 32 : WEB_ROOT_RAM_SLICE_BYTES;
    for (size_t offset=0; offset<length;) {
        uint32_t freeHeap=0;
        uint16_t maxBlock=0;
        uint8_t fragmentation=0;
        ESP.getHeapStats(&freeHeap,&maxBlock,&fragmentation);
        const bool budgetExpired = webResponseActive &&
            (uint32_t)(millis()-webResponseStartedMs) >= webResponseBudgetMs;
        if (budgetExpired || freeHeap < (emergency ? 512UL : WEB_ROOT_MIN_HEAP_BYTES) ||
            maxBlock < (emergency ? 128U : WEB_ROOT_MIN_BLOCK_BYTES)) {
            webRootWriteAbortCount++;
            closeRootClient(client, false);
            return false;
        }
        size_t chunk=length-offset;
        if (chunk>sliceBytes) chunk=sliceBytes;
        markCrashStage(CRASH_STAGE_WEB_ROOT_WRITE,freeHeap);
        // Ordinary COPY writes keep the RAM slice safe on timeout. Drain each
        // slice before queuing more, so the page cannot consume the TCP heap.
        if (client.write(data+offset,chunk)!=chunk || !client.flush(250)) {
            webRootWriteAbortCount++;
            closeRootClient(client, false);
            return false;
        }
        offset+=chunk;
        yield();
    }
    return true;
}

bool writeRootBytes(WiFiClient& client, const uint8_t* data, size_t length) {
    return writeWebBytes(client,data,length,false);
}

void sendRootBusyAndClose(WiFiClient& client, int code, uint32_t freeHeap, uint16_t maxBlock) {
    // Unlike 204, this gives a first-time browser an explicit reason to retry.
    char body[96], response[320];
    int bodyLen=snprintf(body,sizeof(body),"Web busy (%d). Heap=%lu, block=%u. Retry after one second.\n",
                         code,(unsigned long)freeHeap,(unsigned)maxBlock);
    if(bodyLen<=0 || bodyLen>=(int)sizeof(body)) {closeRootClient(client,false);return;}
    int n=snprintf(response,sizeof(response),
        "HTTP/1.1 %d %s\r\nContent-Type: text/plain; charset=utf-8\r\n"
        "Content-Length: %d\r\nRetry-After: 1\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n%s",
        code,code==429 ? "Too Many Requests" : "Service Unavailable",bodyLen,body);
    if(n>0 && n<(int)sizeof(response)) writeWebBytes(client,reinterpret_cast<const uint8_t*>(response),(size_t)n,true);
    if(client.connected()) closeRootClient(client,true);
}

bool writeRootProgmemViaRam(WiFiClient& client, PGM_P content, size_t length) {
    uint8_t ramSlice[WEB_ROOT_RAM_SLICE_BYTES];
    size_t offset = 0;

    while (offset < length) {
        size_t chunk = length - offset;
        if (chunk > sizeof(ramSlice)) chunk = sizeof(ramSlice);

        uint32_t freeHeap = 0;
        if (!webRootHeapHealthy(&freeHeap, nullptr, nullptr)) {
            markCrashStage(CRASH_STAGE_WEB_ROOT_RAMCOPY, freeHeap);
            webRootWriteAbortCount++;
            closeRootClient(client, false);
            return false;
        }

        markCrashStage(CRASH_STAGE_WEB_ROOT_RAMCOPY, freeHeap);
        memcpy_P(ramSlice, content + offset, chunk);

        if (!writeRootBytes(client, ramSlice, chunk)) return false;

        offset += chunk;
        yield();
    }
    return true;
}



// ==========================================================
// END MODULE: LOCAL WEB
// ==========================================================

// ==========================================================
// MODULE: ADMIN / REMOTE OTA
// ==========================================================

// ===== MQTT ADMIN RESPONSE HELPER =====
void publishMqttAdminResponse(const char* payload) {
    if (!mqttClient.connected()) return;

    mqttClient.publish(
        MQTT_ADMIN_RESPONSE_TOPIC,
        payload,
        false
    );
}

// =====================================================
// ΔΕΗ UTILITY READINGS / ESTIMATE ANCHORS
// =====================================================











// ===== REMOTE HTTPS RANGE PULL OTA =====
// Redraw the complete OTA status frame in one OLED buffer transaction.
// doublePrintf() intentionally preserves the existing framebuffer, so using
// it for a changing progress string leaves old pixels behind.
void drawRemoteOtaStatus(const char* status) {
    char sizeLine[32];
    snprintf(
        sizeLine,
        sizeof(sizeLine),
        "%lu bytes",
        (unsigned long)mqttAdminOtaSize
    );

    platformDisplay.clear();
    platformDisplay.font(DisplayFont::Normal10);
    platformDisplay.text(0, 15, "REMOTE OTA");
    platformDisplay.text(0, 30, sizeLine);
    platformDisplay.text(0, 45, status ? status : "");
    platformDisplay.present();
}

void remoteOtaFailAndRestart(const char* reason) {
    queueOledDiagnostic(OLED_DIAG_OTA, "REMOTE OTA FAILED", reason ? reason : "Cause unavailable");
    drawOledDiagnostic(millis());
    delay(1800);
    clearRtcCrashTrace();
    ESP.restart();
}

bool streamHttpRangeIntoUpdate(
    HTTPClient& http,
    uint32_t expectedBytes,
    uint32_t& totalWritten,
    unsigned long otaStartedMs
) {
    WiFiClient* stream = http.getStreamPtr();
    if (stream == nullptr) return false;

    uint8_t buffer[REMOTE_OTA_STREAM_BUFFER_BYTES];
    uint32_t rangeWritten = 0;
    unsigned long lastDataMs = millis();

    while (rangeWritten < expectedBytes) {
        if (millis() - otaStartedMs >= REMOTE_OTA_TOTAL_TIMEOUT_MS) return false;
        int availableNow = stream->available();

        if (availableNow > 0) {
            uint32_t remaining = expectedBytes - rangeWritten;
            size_t want = sizeof(buffer);
            if ((uint32_t)want > remaining) want = (size_t)remaining;
            if (availableNow < (int)want) want = (size_t)availableNow;

            int got = stream->read(buffer, want);
            if (got > 0) {
                size_t flashed = Update.write(buffer, (size_t)got);
                if (flashed != (size_t)got) return false;

                rangeWritten += (uint32_t)got;
                totalWritten += (uint32_t)got;
                lastDataMs = millis();
                yield();
                continue;
            }
        }

        if (!stream->connected() && stream->available() <= 0) {
            return false;
        }

        if (millis() - lastDataMs > REMOTE_OTA_IO_TIMEOUT_MS) {
            return false;
        }

        delay(1);
        yield();
    }

    return true;
}

void processRemoteHttpsOta() {
    if (mqttAdminOtaSize == 0 ||
        mqttAdminOtaMd5[0] == '\0' ||
        mqttAdminOtaUrl[0] == '\0') {
        publishMqttAdminResponse(
            "{\"ok\":false,\"cmd\":\"ota_https\",\"error\":\"missing_parameters\"}"
        );
        return;
    }

    char otaHost[128];
    uint16_t otaPort = 443;
    if (!parseHttpsHostPort(mqttAdminOtaUrl, otaHost, sizeof(otaHost), otaPort)) {
        publishMqttAdminResponse(
            "{\"ok\":false,\"cmd\":\"ota_https\",\"error\":\"invalid_https_url\"}"
        );
        return;
    }

#if PLATFORM_LOCAL_CONTROL_APPLICATION
    applicationPrepareOta();
#endif
    char accepted[180];
    snprintf(
        accepted,
        sizeof(accepted),
        "{\"ok\":true,\"cmd\":\"ota_https\",\"state\":\"starting\","
        "\"from\":\"%s\",\"size\":%lu}",
        VERSION,
        (unsigned long)mqttAdminOtaSize
    );
    publishMqttAdminResponse(accepted);
    mqttClient.loop();
    delay(REMOTE_OTA_ACK_SETTLE_MS);

    markCrashStage(CRASH_STAGE_REMOTE_OTA_PREP, ESP.getFreeHeap());

    server.stop();

    if (mqttClient.connected()) {
        mqttClient.publish(MQTT_STATUS_TOPIC, "offline", true);
        mqttClient.disconnect();
        delay(30);
    }
    mqttNet.stop(0);
    delay(100);
    yield();

    mqttClient.setBufferSize(128);
    delay(20);
    yield();

    uint32_t heapBefore = 0;
    uint16_t maxBlockBefore = 0;
    uint8_t fragBefore = 0;
    ESP.getHeapStats(&heapBefore, &maxBlockBefore, &fragBefore);

    if (heapBefore < 15000UL || maxBlockBefore < 9000U) {
        remoteOtaFailAndRestart("heap before range OTA");
        return;
    }

    markCrashStage(CRASH_STAGE_REMOTE_OTA_TLS, heapBefore);

    BearSSL::WiFiClientSecure otaNet;
    otaNet.setInsecure();
    otaNet.setBufferSizes(REMOTE_OTA_TLS_RX_RANGE, REMOTE_OTA_TLS_TX);
    otaNet.setTimeout(REMOTE_OTA_IO_TIMEOUT_MS);

    HTTPClient http;
    http.setTimeout(REMOTE_OTA_IO_TIMEOUT_MS);
    http.setReuse(true);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setRedirectLimit(3);
    http.setUserAgent(String("" DEVICE_LABEL "/") + VERSION);

    markCrashStage(CRASH_STAGE_REMOTE_OTA_HTTP, ESP.getFreeHeap());

    if (!http.begin(otaNet, String(mqttAdminOtaUrl))) {
        remoteOtaFailAndRestart("HTTP begin");
        return;
    }

    String rangeValue;
    rangeValue.reserve(48);

    uint32_t offset = 0;
    uint32_t totalWritten = 0;
    bool updateStarted = false;

    drawRemoteOtaStatus("Preparing HTTPS...");
    const unsigned long otaStartedMs = millis();
    uint8_t lastDisplayedPercent = 255;

    while (offset < mqttAdminOtaSize) {
        if (millis() - otaStartedMs >= REMOTE_OTA_TOTAL_TIMEOUT_MS) {
            http.end();
            otaNet.stop();
            if (updateStarted) Update.end();
            remoteOtaFailAndRestart("OTA total timeout");
            return;
        }
        uint32_t endOffset = offset + REMOTE_OTA_RANGE_BYTES - 1UL;
        if (endOffset >= mqttAdminOtaSize) {
            endOffset = mqttAdminOtaSize - 1UL;
        }
        uint32_t expected = endOffset - offset + 1UL;

        char rangeText[48];
        snprintf(
            rangeText,
            sizeof(rangeText),
            "bytes=%lu-%lu",
            (unsigned long)offset,
            (unsigned long)endOffset
        );
        rangeValue = rangeText;
        http.addHeader("Range", rangeValue, false, true);

        int code = http.GET();
        if (code != HTTP_CODE_PARTIAL_CONTENT) {
            http.end();
            otaNet.stop();
            if (updateStarted) Update.end();
            remoteOtaFailAndRestart("HTTP range GET");
            return;
        }

        int contentLength = http.getSize();
        if (contentLength <= 0 || (uint32_t)contentLength != expected) {
            http.end();
            otaNet.stop();
            if (updateStarted) Update.end();
            remoteOtaFailAndRestart("range size mismatch");
            return;
        }

        if (!updateStarted) {
            Update.clearError();
            if (!Update.begin(mqttAdminOtaSize, U_FLASH)) {
                http.end();
                otaNet.stop();
                remoteOtaFailAndRestart("Update.begin");
                return;
            }
            if (!Update.setMD5(mqttAdminOtaMd5)) {
                http.end();
                otaNet.stop();
                Update.end();
                remoteOtaFailAndRestart("invalid MD5");
                return;
            }
            updateStarted = true;
            markCrashStage(CRASH_STAGE_REMOTE_OTA_FLASH, ESP.getFreeHeap());
        }

        if (!streamHttpRangeIntoUpdate(http, expected, totalWritten, otaStartedMs)) {
            http.end();
            otaNet.stop();
            Update.end();
            remoteOtaFailAndRestart("range stream");
            return;
        }

        offset = endOffset + 1UL;

        uint8_t percent = (uint8_t)((offset * 100UL) / mqttAdminOtaSize);
        if (percent != lastDisplayedPercent) {
            char progressLine[32];
            snprintf(
                progressLine,
                sizeof(progressLine),
                "Downloading %u%%",
                (unsigned int)percent
            );
            drawRemoteOtaStatus(progressLine);
            lastDisplayedPercent = percent;
        }
        markCrashStage(CRASH_STAGE_REMOTE_OTA_FLASH, offset);

        yield();
    }

    http.end();
    otaNet.stop();

    if (!updateStarted || totalWritten != mqttAdminOtaSize) {
        if (updateStarted) Update.end();
        remoteOtaFailAndRestart("short range image");
        return;
    }

    if (!Update.end()) {
        remoteOtaFailAndRestart("MD5 / flash verify");
        return;
    }

    cs();
    doublePrintf(420, 1, 1, "REMOTE OTA OK");
    doublePrintf(420, 2, 1, "Verified MD5");
    doublePrintf(420, 3, 1, "Restarting...");
    clearRtcCrashTrace();
    delay(300);
    ESP.restart();
}


// ===== MQTT ADMIN COMMAND EXECUTION (OUTSIDE CALLBACK) =====
void processMqttAdminCommand() {
    if (!mqttAdminCommandPending) return;

    MqttAdminCommand command = mqttAdminCommand;

    mqttAdminCommandPending = false;
    mqttAdminCommand = MQTT_ADMIN_NONE;

    switch (command) {
        case MQTT_ADMIN_REMOTE_OTA:
            processRemoteHttpsOta();
            break;

        case MQTT_ADMIN_OTA_REJECTED: {
            char reply[128];
            snprintf(reply, sizeof(reply), "{\"ok\":false,\"cmd\":\"ota_https\",\"error\":\"%s\"}", mqttAdminOtaError);
            publishMqttAdminResponse(reply);
            break;
        }

        case MQTT_ADMIN_RESTART:
            publishMqttAdminResponse(
                "{\"ok\":true,\"cmd\":\"restart_esp\",\"state\":\"restarting\"}"
            );
            mqttAdminRestartPending = true;
            mqttAdminRestartRequestedMs = millis();
            break;

        default:
            if (applicationProcessCommand(command)) break;
            publishMqttAdminResponse(
                "{\"ok\":false,\"error\":\"invalid_command\"}"
            );
            break;
    }

    applicationResetCommand();
}

// ==========================================================
// END MODULE: ADMIN / REMOTE OTA
// ==========================================================

// ==========================================================
// MODULE: RECOVERY / WIFI / NETWORK
// ==========================================================

// =====================================================
// RESET FORENSICS / OTA-ONLY RECOVERY
//
// REASON_DEFAULT_RST (power-up) and normal software/OTA restart are NOT
// treated as faults. Hardware WDT, exception and software WDT are faults.
// External RST pin is a special, deliberate exit from recovery. It is
// reported as REASON_EXT_SYS_RST by the SDK, including some USB/DTR resets.
// A crash stays latched across power cycles.
// Recovery can be cleared by successful OTA, external RST, or the
// Recovery-only "Restart normally" action, provided LittleFS deletion succeeds.
// =====================================================
const char* resetReasonLabel(uint32_t reason) {
    switch (reason) {
        case REASON_DEFAULT_RST: return "POWER ON";
        case REASON_WDT_RST: return "HARDWARE WDT";
        case REASON_EXCEPTION_RST: return "EXCEPTION";
        case REASON_SOFT_WDT_RST: return "SOFTWARE WDT";
        case REASON_SOFT_RESTART: return "SOFTWARE / OTA";
        case REASON_DEEP_SLEEP_AWAKE: return "DEEP SLEEP";
        case REASON_EXT_SYS_RST: return "EXTERNAL RESET";
        case RECOVERY_REASON_FORCED_OTA: return "FORCED OTA RECOVERY";
        default: return "UNKNOWN";
    }
}

bool isFaultReset(uint32_t reason) {
    return reason == REASON_WDT_RST ||
           reason == REASON_EXCEPTION_RST ||
           reason == REASON_SOFT_WDT_RST;
}

uint32_t recoveryChecksum() {
    const RecoveryRecord& rec = recoveryIncident;
    return rec.magic ^ rec.reason ^ rec.exccause ^ rec.epc1 ^
           rec.epc2 ^ rec.epc3 ^ rec.excvaddr ^ rec.depc ^ 0xA25E2F00UL;
}

void captureCurrentReset() {
    currentBootReset = RecoveryRecord();
    RecoveryRecord& rec = currentBootReset;
    rec.magic = RECOVERY_MAGIC;
    const struct rst_info* rst = ESP.getResetInfoPtr();
    if (rst != nullptr) {
        rec.reason = rst->reason;
        rec.exccause = rst->exccause;
        rec.epc1 = rst->epc1;
        rec.epc2 = rst->epc2;
        rec.epc3 = rst->epc3;
        rec.excvaddr = rst->excvaddr;
        rec.depc = rst->depc;
    }
}


bool loadRecoveryIncident() {
    if (!littleFsReady || !LittleFS.exists(RECOVERY_FILE)) return false;
    File file = LittleFS.open(RECOVERY_FILE, "r");
    if (!file) return false;
    RecoveryRecord rec = {};
    bool valid = file.size() == (size_t)sizeof(rec) &&
                 file.readBytes((char*)&rec, sizeof(rec)) == sizeof(rec);
    file.close();
    if (!valid) return false;
    recoveryIncident = rec;
    // Accept an external-reset incident written by the initial v2.0
    // diagnostic build as a legacy latched record. NEW external RST events
    // clear the latch instead of ever creating another one.
    bool recognizedSavedReason = isFaultReset(recoveryIncident.reason) ||
                                  recoveryIncident.reason == REASON_EXT_SYS_RST ||
                                  recoveryIncident.reason == RECOVERY_REASON_FORCED_OTA;
    if (recoveryIncident.magic != RECOVERY_MAGIC ||
        recoveryIncident.checksum != recoveryChecksum() ||
        !recognizedSavedReason) {
        recoveryIncident = RecoveryRecord();
        return false;
    }
    recoveryIncidentPersistent = true;
    return true;
}

bool saveRecoveryIncident() {
    if (!littleFsReady) return false;
    recoveryIncident.magic = RECOVERY_MAGIC;
    recoveryIncident.checksum = recoveryChecksum();
    File file = LittleFS.open(RECOVERY_FILE, "w");
    if (!file) return false;
    bool ok = file.write((const uint8_t*)&recoveryIncident,
                         sizeof(recoveryIncident)) == sizeof(recoveryIncident);
    file.close();
    recoveryIncidentPersistent = ok;
    return ok;
}

void showRecoveryScreen() {
    if (drawOledDiagnostic(millis())) return;
    cs();
    doublePrintf(420, 1, 1, "RECOVERY / Ver %s", VERSION);
    doublePrintf(420, 2, 1, "%s (%lu)",
                 resetReasonLabel(recoveryIncident.reason),
                 (unsigned long)recoveryIncident.reason);
    if (WiFi.status() == WL_CONNECTED) {
        doublePrintf(420, 3, 1, "IP: %s", WiFi.localIP().toString().c_str());
    } else if (credentialPortalActive) {
        doublePrintf(420, 3, 1, "AP: %s", WiFi.softAPIP().toString().c_str());
    } else {
        doublePrintf(420, 3, 1, "WiFi reconnecting");
    }
    doublePrintf(420, 4, 1, "RST = NORMAL / OTA");
}

String buildRecoveryReport() {
    String d;
    d.reserve(1700);
    d += "" DEVICE_LABEL " OTA-ONLY RECOVERY / Ver " + String(VERSION) + "\n";
    d += "Measurements, MQTT, admin commands and statistics: DISABLED\n";
    d += "Firmware=" + String(VERSION) + " build=" + String(BUILD_DATE) + "\n";
    d += "Meter=" + String(DEVICE_KIND) + " OLED=" + String(OLED_TYPE) + "\n";
    d += "Incident reason=" + String(resetReasonLabel(recoveryIncident.reason)) +
         " (" + String(recoveryIncident.reason) + ")\n";
    d += "Incident stored in LittleFS=" +
         String(recoveryIncidentPersistent ? "YES" : "NO (current boot only)") + "\n";
    d += "RTC crash breadcrumb=";
    if (rtcTraceBeforeResetValid) {
        d += String(crashStageLabel(rtcTraceBeforeReset.stage)) +
             " (" + String(rtcTraceBeforeReset.stage) + ")" +
             " detail=" + String(rtcTraceBeforeReset.detail) + "\n";
        d += "RTC trace heap free bytes=" + String(rtcTraceBeforeReset.heapFree) + "\n";
        d += "RTC trace max free block bytes=" + String(rtcTraceBeforeReset.maxFreeBlock) + "\n";
        d += "RTC trace heap fragmentation percent=" +
             String(rtcTraceBeforeReset.heapFragmentation) + "\n";
    } else {
        d += "UNAVAILABLE\n";
    }
    d += "External RST clear failed=" +
         String(recoveryResetClearFailed ? "YES" : "NO") + "\n";
    d += "Recovery exit: successful OTA, external RST pin, or /restart-normal\n";
    d += "Exception cause=" + String(recoveryIncident.exccause) + "\n";
    char address[20];
    snprintf(address, sizeof(address), "0x%08lX", (unsigned long)recoveryIncident.epc1);
    d += "EPC1=" + String(address) + "\n";
    snprintf(address, sizeof(address), "0x%08lX", (unsigned long)recoveryIncident.epc2);
    d += "EPC2=" + String(address) + "\n";
    snprintf(address, sizeof(address), "0x%08lX", (unsigned long)recoveryIncident.epc3);
    d += "EPC3=" + String(address) + "\n";
    snprintf(address, sizeof(address), "0x%08lX", (unsigned long)recoveryIncident.excvaddr);
    d += "EXCVADDR=" + String(address) + "\n";
    snprintf(address, sizeof(address), "0x%08lX", (unsigned long)recoveryIncident.depc);
    d += "DEPC=" + String(address) + "\n";
    d += "Current boot reason=" + String(resetReasonLabel(currentBootReset.reason)) +
         " (" + String(currentBootReset.reason) + ")\n";
    d += "ESP reset information=" + ESP.getResetInfo() + "\n";
    d += "Chip ID=" + String(ESP.getChipId(), HEX) + "\n";
    d += "CPU MHz=" + String(ESP.getCpuFreqMHz()) + "\n";
    d += "Core version=" + ESP.getCoreVersion() + "\n";
    d += "SDK version=" + String(ESP.getSdkVersion()) + "\n";
    d += "Uptime seconds=" + String(millis()/1000UL) + "\n";
    d += "Free heap bytes=" + String(ESP.getFreeHeap()) + "\n";
#if defined(UMM_INFO)
    d += "Maximum free block bytes=" + String(ESP.getMaxFreeBlockSize()) + "\n";
    d += "Heap fragmentation percent=" + String(ESP.getHeapFragmentation()) + "\n";
#endif
    d += "Free contiguous stack bytes=" + String(ESP.getFreeContStack()) + "\n";
    d += "Free sketch space bytes=" + String(ESP.getFreeSketchSpace()) + "\n";
    d += "Flash configured bytes=" + String(ESP.getFlashChipSize()) + "\n";
    d += "Flash actual bytes=" + String(ESP.getFlashChipRealSize()) + "\n";
    d += "Wi-Fi status=" + String((int)WiFi.status()) + "\n";
    d += "Static local IP=" + WiFi.localIP().toString() + "\n";
    d += "Gateway=" + WiFi.gatewayIP().toString() + "\n";
    d += "Subnet=" + WiFi.subnetMask().toString() + "\n";
    d += "DNS=" + WiFi.dnsIP().toString() + "\n";
    if (WiFi.status() == WL_CONNECTED) {
        d += "Wi-Fi RSSI dBm=" + String(WiFi.RSSI()) + "\n";
        d += "Web root requests=" + String(webRootRequestCount) + "\n";
        d += "Web rapid reloads retried=" + String(webRootRateLimitedCount) + "\n";
        d += "Web memory busy=" + String(webRootBusyCount) + "\n";
        d += "Web write aborts=" + String(webRootWriteAbortCount) + "\n";
        d += "Web sockets explicitly closed=" + String(webRootForcedCloseCount) + "\n";
        d += "Wi-Fi channel=" + String(WiFi.channel()) + "\n";
    }
    d += "Recovery OTA active=" + String(recoveryOtaInProgress ? "YES" : "WAITING") + "\n";
    d += "OTA recovery clear failed=" + String(recoveryOtaClearFailed ? "YES" : "NO") + "\n";
    d += "AP attempts=" + String(credentialApAttempts) + " last failure=" +
         String(credentialApLastFailure) + " (1=radio,2=IP,3=start), heap=" +
         String(credentialApHeapAtAttempt) + "\n";
    d += "OLED failure hold ms=" + String(OLED_DIAGNOSTIC_HOLD_MS) +
         " pending=" + String(oledDiagnosticCount) + "\n";
    if (oledDiagnosticCount) {
        d += "OLED diagnostic=" + String(oledDiagnostics[0].title) + "\n";
        d += "OLED cause=" + String(oledDiagnostics[0].detail) + "\n";
    }
    d += "Only Recovery HTTP endpoints are available: /, /debug, /debug.txt, /restart-normal, POST /esp-restart.\n";
    d += "Successful OTA or Restart normally clears the recovery incident before reboot.\n";
    d += "If clearing the incident fails, Recovery remains latched. External RST also clears Recovery.\n";
    d += "No passwords are shown in this diagnostic report.\n";
    return d;
}

void handleRecoveryText() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/plain; charset=utf-8", buildRecoveryReport());
}

void handleRecoveryPage() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    String report = buildRecoveryReport();
    String html;
    html.reserve(report.length() + 650);
    html += F("<!doctype html><html><head><meta charset='utf-8'>"
              "<meta name='viewport' content='width=device-width,initial-scale=1'>"
              "<meta http-equiv='refresh' content='10'>"
              "<title>" DEVICE_LABEL " RECOVERY</title>"
              "<style>body{font:22px/1.35 Arial,sans-serif;margin:15px auto;padding:12px;"
              "max-width:900px}pre{white-space:pre-wrap;overflow-wrap:anywhere;"
              "border:1px solid #bbb;padding:12px;background:#f5f5f5}"
              "h1{color:#b00020}</style><style>body{font:22px/1.35 Arial,sans-serif;max-width:760px;margin:auto;padding:10px;color:#142431}h1,h2,small{font-size:inherit}pre{font:inherit;white-space:pre-wrap;overflow-wrap:anywhere}input,button{font:inherit}</style></head><body>"
              "<h1>OTA-ONLY RECOVERY</h1>"
              "<p>Normal measurement mode is locked. IDE_OTA remains available on LAN. "
              "A successful OTA clears Recovery and the new firmware starts normally after reboot. "
              "Physical RST also clears Recovery.</p>"
              "<p><a href='/restart-normal' "
              "onclick=\"return confirm('Clear the recovery incident and restart normal operation?');\">"
              "<b>Restart normally</b></a></p>"
              "<form action='/esp-restart' method='post' "
              "onsubmit=\"return confirm('Restart ESP but KEEP OTA-only Recovery active?');\">"
              "<button type='submit'>ESP Restart (keep Recovery)</button></form>"
              "<a href='/debug.txt'>Copy/download plain-text diagnostics</a><pre>");
    for (size_t i=0; i<report.length(); ++i) {
        char ch = report[i];
        if (ch == '&') html += F("&amp;");
        else if (ch == '<') html += F("&lt;");
        else if (ch == '>') html += F("&gt;");
        else html += ch;
    }
    html += F("</pre></body></html>");
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/html; charset=utf-8", html);
}

// POST only; intentionally keeps persistent recovery latch intact.
void handleRecoveryEspRestart() {
    WebResponseScope responseScope;
    if (!recoveryMode || !requireApWebPin()) return;
    if (recoveryOtaInProgress || recoveryNormalRestartPending) {
        server.send(409, "text/plain; charset=utf-8",
                    "Restart refused: OTA or normal restart in progress.");
        return;
    }
    if (!recoveryEspRestartPending) {
        recoveryEspRestartPending = true;
        recoveryEspRestartRequestedMs = millis();
    }
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/plain; charset=utf-8",
                "ESP restarting, OTA-only Recovery incident preserved.");
}

void handleRecoveryRestartNormal() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    // Fail closed: normal mode is allowed only after the persistent recovery
    // latch has definitely been removed from LittleFS.
    if (!littleFsReady) {
        queueOledDiagnostic(OLED_DIAG_RECOVERY_CLEAR, "RECOVERY CLEAR FAILED", "LittleFS unavailable; Recovery remains latched");
        server.sendHeader("Cache-Control", "no-store");
        server.send(503, "text/plain; charset=utf-8",
                    "Restart refused: LittleFS unavailable; Recovery remains latched.");
        return;
    }

    bool clearSucceeded = true;
    if (LittleFS.exists(RECOVERY_FILE)) {
        clearSucceeded = LittleFS.remove(RECOVERY_FILE);
    }

    if (!clearSucceeded) {
        recoveryResetClearFailed = true;
        queueOledDiagnostic(OLED_DIAG_RECOVERY_CLEAR, "RECOVERY CLEAR FAILED", "Recovery file removal failed; restart refused");
        server.sendHeader("Cache-Control", "no-store");
        server.send(500, "text/plain; charset=utf-8",
                    "Restart refused: recovery incident could not be cleared.");
        return;
    }

    recoveryIncident = RecoveryRecord();
    recoveryIncidentPersistent = false;

    server.sendHeader("Cache-Control", "no-store");
    server.send_P(
        200,
        PSTR("text/html; charset=utf-8"),
        PSTR("<!doctype html><html><head><meta charset='utf-8'>"
             "<meta name='viewport' content='width=device-width,initial-scale=1'>"
             "<title>Restarting</title><style>body{font:22px/1.35 Arial,sans-serif;max-width:760px;margin:auto;padding:10px;color:#142431}h1,h2,small{font-size:inherit}pre{font:inherit;white-space:pre-wrap;overflow-wrap:anywhere}input,button{font:inherit}</style></head><body>"
             "<h2>Recovery cleared</h2>"
             "<p>Restarting in normal measurement mode...</p>"
             "</body></html>")
    );

    recoveryNormalRestartPending = true;
    recoveryNormalRestartRequestedMs = millis();
}


bool writeCredentialHtmlEscaped(WiFiClient& client, const char* value) {
    if (!value) return true;
    for (const char* p = value; *p; ++p) {
        const char* entity = nullptr;
        size_t entityLen = 0;
        switch (*p) {
            case '&': entity = "&amp;"; entityLen = 5; break;
            case '<': entity = "&lt;"; entityLen = 4; break;
            case '>': entity = "&gt;"; entityLen = 4; break;
            case '"': entity = "&quot;"; entityLen = 6; break;
            case '\'': entity = "&#39;"; entityLen = 5; break;
            default:
                if (!writeRootBytes(client, reinterpret_cast<const uint8_t*>(p), 1)) return false;
                continue;
        }
        if (!writeRootBytes(client, reinterpret_cast<const uint8_t*>(entity), entityLen)) {
            return false;
        }
    }
    return true;
}

bool writeCredentialProgmem(WiFiClient& client, PGM_P text) {
    const size_t len = strlen_P(text);
    return writeRootProgmemViaRam(client, text, len);
}

void handleCredentialSetupPage() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    uint32_t freeHeap = 0;
    uint16_t maxBlock = 0;
    uint8_t fragmentation = 0;
    ESP.getHeapStats(&freeHeap, &maxBlock, &fragmentation);
    markCrashStage(CRASH_STAGE_WEB_SETUP, freeHeap);

    // This page is intentionally streamed in small pieces. Do not rebuild it
    // into a String: MQTT/TLS can leave only a few KB of heap on ESP8266.
    if (freeHeap < WEB_ROOT_MIN_HEAP_BYTES || maxBlock < WEB_ROOT_MIN_BLOCK_BYTES) {
        WiFiClient client = server.client();
        sendRootBusyAndClose(client,503,freeHeap,maxBlock);
        return;
    }

    DeviceCredentialRecord stored = {};
    bool haveStored = littleFsReady && readDeviceCredentialRecord(stored);

    WiFiClient client = server.client();
    client.setTimeout(1500);

    static const char header[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: no-store\r\n"
        "Connection: close\r\n\r\n";
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(header),sizeof(header)-1)) return;

    if (!writeCredentialProgmem(client, PSTR(
        "<!doctype html><html><head><meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>" DEVICE_LABEL " Setup</title><base href='" PLATFORM_SETUP_BASE_URL "'><style>"
        "body{font:22px/1.35 Arial,sans-serif;max-width:760px;margin:auto;padding:18px;color:#142431}"
        "input{font:inherit;width:100%;box-sizing:border-box;padding:10px;margin:5px 0 12px}"
        "h1,h2,small{font-size:inherit}button{font:inherit;padding:11px 16px;margin:6px 4px 6px 0}"
        ".card{border:1px solid #dbe3e8;border-radius:12px;padding:14px;margin:12px 0}"
        "</style><style>body{font:22px/1.35 Arial,sans-serif;max-width:760px;margin:auto;padding:10px;color:#142431}h1,h2,small{font-size:inherit}pre{font:inherit;white-space:pre-wrap;overflow-wrap:anywhere}input,button{font:inherit}</style></head><body><h2>" DEVICE_LABEL " v" VERSION " setup</h2>"
        "<div class='card'><p>Stored credentials: <b>"))) return;

    if (!writeCredentialProgmem(client, haveStored ? PSTR("YES") : PSTR("NO"))) return;

    if (!writeCredentialProgmem(client, PSTR(
        "</b></p><form method='post' action='/setup-save'>"
        "<label>Wi-Fi SSID</label><input name='ssid' maxlength='32' value='"))) return;
    if (haveStored && !writeCredentialHtmlEscaped(client, stored.ssid)) return;

    if (!writeCredentialProgmem(client, PSTR(
        "' required><label>Wi-Fi password</label>"
        "<input name='wifi_password' type='password' maxlength='64' placeholder='"))) return;
    if (!writeCredentialProgmem(client,
        haveStored ? PSTR("leave blank to keep current") : PSTR("required"))) return;

    if (!writeCredentialProgmem(client, PSTR(
        "'><label>MQTT username</label><input name='mqtt_user' maxlength='64' value='"))) return;
    if (haveStored && !writeCredentialHtmlEscaped(client, stored.mqttUser)) return;

    if (!writeCredentialProgmem(client, PSTR(
        "'><label>MQTT password</label>"
        "<input name='mqtt_password' type='password' maxlength='64' placeholder='"))) return;
    if (!writeCredentialProgmem(client,
        haveStored ? PSTR("leave blank to keep current") : PSTR("required"))) return;

    writeCredentialProgmem(client, PSTR(
        "'><button type='submit'>Save &amp; restart</button></form>"
        "<p><a href='/network-debug'>Network debug</a> · <a href='/'>Back</a></p></div>"
        "<div class='card'><form method='get' action='/forget-credentials-confirm'>"
        "<button type='submit'>Clear stored credentials...</button></form>"
        "<small>Saved passwords are kept when left blank. MQTT may be left incomplete: router Wi-Fi and this page remain available.</small>"
        "<small>This action requires a second confirmation page. Passwords are never displayed.</small></div></body></html>"));
}

void handleCredentialSave() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    if (!littleFsReady) {
        server.send(503, "text/plain; charset=utf-8", "LittleFS unavailable");
        return;
    }

    DeviceCredentialRecord oldRec = {};
    bool haveOld = readDeviceCredentialRecord(oldRec);

    String newSsid = server.arg("ssid");
    String newWifiPassword = server.arg("wifi_password");
    String newHost = MQTT_BROKER_HOST;
    String newUser = server.arg("mqtt_user");
    String newMqttPassword = server.arg("mqtt_password");
    newSsid.trim(); newUser.trim();

    if (newWifiPassword.length() == 0 && haveOld) newWifiPassword = oldRec.wifiPassword;
    if (newMqttPassword.length() == 0 && haveOld) newMqttPassword = oldRec.mqttPassword;

    const uint16_t portValue = MQTT_BROKER_PORT;
    if (newSsid.length() == 0 || newSsid.length() >= sizeof(oldRec.ssid) ||
        newWifiPassword.length() == 0 || newWifiPassword.length() >= sizeof(oldRec.wifiPassword) ||
        newHost.length() == 0 || newHost.length() >= sizeof(oldRec.mqttHost) ||
        newUser.length() >= sizeof(oldRec.mqttUser) ||
        newMqttPassword.length() >= sizeof(oldRec.mqttPassword) ||
        portValue < 1 || portValue > 65535) {
        server.send(400, "text/plain; charset=utf-8", "Invalid credential fields");
        return;
    }

    DeviceCredentialRecord rec = {};
    rec.magic = DEVICE_CREDENTIAL_MAGIC;
    rec.version = DEVICE_CREDENTIAL_VERSION;
    rec.mqttPort = (uint16_t)portValue;
    if (!copyCredentialField(rec.ssid, sizeof(rec.ssid), newSsid.c_str()) ||
        !copyCredentialField(rec.wifiPassword, sizeof(rec.wifiPassword), newWifiPassword.c_str()) ||
        !copyCredentialField(rec.mqttHost, sizeof(rec.mqttHost), newHost.c_str()) ||
        !copyOptionalCredentialField(rec.mqttUser, sizeof(rec.mqttUser), newUser.c_str()) ||
        !copyOptionalCredentialField(rec.mqttPassword, sizeof(rec.mqttPassword), newMqttPassword.c_str())) {
        server.send(400, "text/plain; charset=utf-8", "Credential field too long");
        return;
    }
    rec.checksum = deviceCredentialChecksum(rec);

    if (!writeDeviceCredentialRecord(rec)) {
        server.send(500, "text/plain; charset=utf-8", "Could not save credentials");
        return;
    }

    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/html; charset=utf-8",
                "<!doctype html><html><body><h2>Saved</h2>"
                "<p>Saved and read-back verified. Restarting with the stored Wi-Fi settings.</p>"
                "<p>MQTT rejection will not erase settings or disable router Wi-Fi.</p></body></html>");
    credentialRestartPending = true;
    credentialRestartRequestedMs = millis();
}

void handleForgetCredentialsConfirm() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/html; charset=utf-8",
                "<!doctype html><html><head><meta charset='utf-8'>"
                "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<title>Confirm credential clear</title><style>body{font:22px/1.35 Arial,sans-serif;max-width:760px;margin:auto;padding:10px;color:#142431}h1,h2,small{font-size:inherit}pre{font:inherit;white-space:pre-wrap;overflow-wrap:anywhere}input,button{font:inherit}</style></head><body>"
                "<h2>Confirm credential clear</h2>"
                "<p><b>Warning:</b> this will erase the stored Wi-Fi and MQTT credentials.</p>"
                "<p>After clearing, the ESP8266 will restart into Setup AP mode.</p>"
                "<form method='post' action='/forget-credentials'>"
                "<label><input type='checkbox' name='confirm' value='ERASE' required> "
                "I understand and want to erase the stored credentials.</label><br><br>"
                "<button type='submit'>YES - ERASE CREDENTIALS</button>"
                "</form>"
                "<p><a href='/setup'>Cancel and go back</a></p>"
                "</body></html>");
}

void handleForgetCredentials() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    if (server.arg("confirm") != "ERASE") {
        server.sendHeader("Cache-Control", "no-store");
        server.send(400, "text/html; charset=utf-8",
                    "<!doctype html><html><body><h2>Confirmation required</h2>"
                    "<p>Credentials were NOT erased.</p>"
                    "<p><a href='/forget-credentials-confirm'>Open confirmation page</a></p>"
                    "</body></html>");
        return;
    }

    if (!littleFsReady) {
        server.send(503, "text/plain; charset=utf-8", "LittleFS unavailable");
        return;
    }
    bool ok = true;
#if PLATFORM_LOCAL_CONTROL_APPLICATION
    if (!applicationClearLegacyCredentials()) {
        server.send(500, "text/plain; charset=utf-8", "Could not remove legacy credentials");
        return;
    }
#endif
    if (LittleFS.exists(DEVICE_CREDENTIAL_FILE)) ok &= LittleFS.remove(DEVICE_CREDENTIAL_FILE);
    if (LittleFS.exists(DEVICE_CREDENTIAL_TEMP_FILE)) ok &= LittleFS.remove(DEVICE_CREDENTIAL_TEMP_FILE);
    if (!ok) {
        server.send(500, "text/plain; charset=utf-8", "Could not remove credentials");
        return;
    }
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/html; charset=utf-8",
                "<!doctype html><html><body><h2>Credentials cleared</h2>"
                "<p>Restarting into dedicated Setup AP mode...</p>"
                "<p>The OLED will show the open AP name, web PIN and setup address.</p>"
                "</body></html>");
    credentialRestartPending = true;
    credentialRestartRequestedMs = millis();
}

void handleForceOtaRecoveryConfirm() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/html; charset=utf-8",
                "<!doctype html><html><head><meta charset='utf-8'>"
                "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<title>Force OTA Recovery</title><style>body{font:22px/1.35 Arial,sans-serif;max-width:760px;margin:auto;padding:10px;color:#142431}h1,h2,small{font-size:inherit}pre{font:inherit;white-space:pre-wrap;overflow-wrap:anywhere}input,button{font:inherit}</style></head><body>"
                "<h2>Force OTA-ONLY Recovery</h2>"
                "<p>This restarts the ESP8266 into recovery mode. Measurements and MQTT will be disabled until OTA succeeds or Recovery is exited.</p>"
                "<form method='post' action='/force-ota-recovery'>"
                "<label><input type='checkbox' name='confirm' value='ENTER' required> "
                "Enter OTA-ONLY Recovery after restart.</label><br><br>"
                "<button type='submit'>ENTER OTA-ONLY RECOVERY</button></form>"
                "<p><a href='/'>Cancel</a></p></body></html>");
}

void handleForceOtaRecovery() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    if (server.arg("confirm") != "ENTER") {
        server.send(400, "text/plain; charset=utf-8", "Confirmation required");
        return;
    }
    if (!littleFsReady) {
        server.send(503, "text/plain; charset=utf-8", "LittleFS unavailable");
        return;
    }

    recoveryIncident = RecoveryRecord();
    recoveryIncident.reason = RECOVERY_REASON_FORCED_OTA;
    if (!saveRecoveryIncident() || !loadRecoveryIncident()) {
        server.send(500, "text/plain; charset=utf-8", "Could not persist Recovery request");
        return;
    }

    clearRtcCrashTrace();
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/html; charset=utf-8",
                "<!doctype html><html><body><h2>OTA-ONLY Recovery requested</h2>"
                "<p>Restarting. IDE_OTA and mDNS will be enabled in Recovery.</p>"
                "</body></html>");
    forceRecoveryRestartPending = true;
#if PLATFORM_LOCAL_CONTROL_APPLICATION
    applicationPrepareOta();
#endif
    forceRecoveryRestartRequestedMs = millis();
}

void handleNetworkDebugPage() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    uint32_t freeHeap = 0;
    uint16_t maxBlock = 0;
    uint8_t fragmentation = 0;
    ESP.getHeapStats(&freeHeap, &maxBlock, &fragmentation);
    markCrashStage(CRASH_STAGE_WEB_NET_DEBUG, freeHeap);

    if (freeHeap < WEB_ROOT_MIN_HEAP_BYTES || maxBlock < WEB_ROOT_MIN_BLOCK_BYTES) {
        WiFiClient client = server.client();
        sendRootBusyAndClose(client,503,freeHeap,maxBlock);
        return;
    }

    DeviceCredentialRecord stored = {};
    bool haveStored = littleFsReady && readDeviceCredentialRecord(stored);

    IPAddress resolved(0, 0, 0, 0);
    int dnsResult = 0;
    const bool dnsAttempted = mqttHost[0] != '\0' && WiFi.status() == WL_CONNECTED;
    if (dnsAttempted) {
        dnsResult = WiFi.hostByName(mqttHost, resolved, MQTT_DNS_TIMEOUT_MS);
    }

    const bool resolvedAllZero =
        resolved[0] == 0 && resolved[1] == 0 &&
        resolved[2] == 0 && resolved[3] == 0;
    const bool resolvedBroadcast =
        resolved[0] == 255 && resolved[1] == 255 &&
        resolved[2] == 255 && resolved[3] == 255;
    const bool dnsAddressUsable =
        dnsResult == 1 && !resolvedAllZero && !resolvedBroadcast;

    WiFiClient client = server.client();
    client.setTimeout(1500);

    static const char header[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: no-store\r\n"
        "Connection: close\r\n\r\n";
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(header),sizeof(header)-1)) return;

    if (!writeCredentialProgmem(client, PSTR(
        "<!doctype html><html><head><meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Network debug</title><style>body{font:22px/1.35 Arial,sans-serif;max-width:760px;margin:auto;padding:10px;color:#142431}h1,h2,small{font-size:inherit}pre{font:inherit;white-space:pre-wrap;overflow-wrap:anywhere}input,button{font:inherit}</style></head><body><h2>Network debug</h2><pre>"
        "Firmware: " VERSION "\n"
        "Build: " BUILD_DATE "\n"
        "Device: " DEVICE_ID "\n"
        "Meter: " DEVICE_KIND "\n"
    ))) return;

    char line[256];
    int n = snprintf_P(line, sizeof(line), PSTR("MQTT unique device ID: %s\n"), mqttDeviceId);
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;
    n = snprintf_P(line, sizeof(line), PSTR("OLED type: %u\n"
        "Uptime seconds: %lu\n"
        "Recovery mode: %s\n"
        "Credential portal active: %s\n"
        "Credential portal mode: %s\n"
        "IDE_OTA policy: RECOVERY ONLY\n"
        "Recovery mDNS discovery: ENABLED\n"),
        (unsigned)OLED_TYPE,
        (unsigned long)(millis() / 1000UL),
        recoveryMode ? "YES" : "NO",
        credentialPortalActive ? "YES" : "NO",
        credentialPortalActive
            ? (credentialPortalOnly ? "AP-ONLY" : "AP+STA")
            : "OFF");
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

    n = snprintf_P(line, sizeof(line), PSTR("AP attempts: %lu; heap at attempt: %lu\nLast AP startup failure: %u ("),
        (unsigned long)credentialApAttempts, (unsigned long)credentialApHeapAtAttempt,
        (unsigned)credentialApLastFailure);
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;
    if (!writeCredentialProgmem(client,apFailureDescription(credentialApLastFailure))) return;
    n = snprintf_P(line, sizeof(line), PSTR(")\nOLED failure hold: %lums; queued: %u\n"),
                 OLED_DIAGNOSTIC_HOLD_MS,(unsigned)oledDiagnosticCount);
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;
    if (oledDiagnosticCount) {
        n = snprintf_P(line, sizeof(line), PSTR("OLED diagnostic: %s\nCause: %s\n"),
                     oledDiagnostics[0].title, oledDiagnostics[0].detail);
        if (n <= 0 || n >= (int)sizeof(line)) return;
        if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;
    }

    n = snprintf_P(line, sizeof(line), PSTR("Boot checkpoints: %s\nLast boot CP: %s (%lu)\n"
        "Wi-Fi attempts: %lu\nWi-Fi retry delay ms: %lu\n"
        "Offline policy: LOCAL METER + STA RETRIES (no automatic setup AP)\n"),
        bootCheckpointsActive ? "ACTIVE" : "DONE",
        crashStageLabel(bootLastCheckpoint), (unsigned long)bootLastCheckpoint,
        (unsigned long)wifiConnectAttempts, WIFI_RETRY_STEPS_MS[wifiRetryStep]);
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

    if (rtcTraceBeforeResetValid) {
        n = snprintf_P(line, sizeof(line), PSTR("Previous RTC step: %s (%lu), detail=%lu\nPrevious RTC heap: %lu, largest=%lu\n"),
            crashStageLabel(rtcTraceBeforeReset.stage), (unsigned long)rtcTraceBeforeReset.stage,
            (unsigned long)rtcTraceBeforeReset.detail, (unsigned long)rtcTraceBeforeReset.heapFree,
            (unsigned long)rtcTraceBeforeReset.maxFreeBlock);
        if (n <= 0 || n >= (int)sizeof(line)) return;
        if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;
        if (rtcTraceBeforeReset.stage == CRASH_STAGE_BOOT_OLED_WRITE) {
            n = snprintf_P(line, sizeof(line), PSTR("Checkpoint OLED write pending CP: %s (%lu)\n"),
                         crashStageLabel(rtcTraceBeforeReset.detail), (unsigned long)rtcTraceBeforeReset.detail);
            if (n <= 0 || n >= (int)sizeof(line)) return;
            if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;
        }
    }

    if (!writeCredentialProgmem(client, PSTR(
        "\n[Credentials]\n"
        "Stored credential record: "))) return;
    if (!writeCredentialProgmem(client, haveStored ? PSTR("YES\n") : PSTR("NO\n"))) return;

    n = snprintf_P(line, sizeof(line), PSTR("Wi-Fi SSID configured: %s\n"
        "Wi-Fi password configured: %s\n"
        "MQTT host configured: %s\n"
        "MQTT host length: %u\n"
        "MQTT port: %u\n"
        "MQTT username configured: %s\n"
        "MQTT username length: %u\n"
        "MQTT password configured: %s\n"),
        ssid[0] != '\0' ? "YES" : "NO",
        password[0] != '\0' ? "YES" : "NO",
        mqttHost[0] != '\0' ? "YES" : "NO",
        (unsigned)strlen(mqttHost),
        (unsigned)mqttPort,
        mqttUser[0] != '\0' ? "YES" : "NO",
        (unsigned)strlen(mqttUser),
        mqttPassword[0] != '\0' ? "YES" : "NO");
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

    if (!writeCredentialProgmem(client, PSTR("\n[Wi-Fi]\n"))) return;

    n = snprintf_P(line, sizeof(line), PSTR("Wi-Fi connected: %s\nWi-Fi status code: %d ("),
        WiFi.status() == WL_CONNECTED ? "YES" : "NO", (int)WiFi.status());
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;
    if (!writeCredentialProgmem(client,wifiStatusDescription((int)WiFi.status()))) return;
    if (!writeCredentialProgmem(client,PSTR(")\nCurrent operation: "))) return;
    if (!writeCredentialProgmem(client,networkOperationDescription())) return;
    n = snprintf_P(line, sizeof(line), PSTR("\nLast recorded Wi-Fi disconnect: %u ("),(unsigned)wifiLastDisconnectReason);
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;
    if (!writeCredentialProgmem(client,wifiDisconnectDescription(wifiLastDisconnectReason))) return;
    n = snprintf_P(line, sizeof(line), PSTR(")\nRSSI dBm: %d\nChannel: %d\nStatic IP profile: %s\n"),
        WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0,
        WiFi.status() == WL_CONNECTED ? WiFi.channel() : 0, USE_STATIC_IP ? "YES" : "NO");
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

    if (WiFi.status() == WL_CONNECTED) {
        IPAddress lip = WiFi.localIP();
        IPAddress gw = WiFi.gatewayIP();
        IPAddress mask = WiFi.subnetMask();
        IPAddress dns0 = WiFi.dnsIP(0);
        IPAddress dns1 = WiFi.dnsIP(1);

        n = snprintf_P(line, sizeof(line), PSTR("Local IP: %u.%u.%u.%u\n"
            "Gateway: %u.%u.%u.%u\n"
            "Subnet: %u.%u.%u.%u\n"),
            lip[0], lip[1], lip[2], lip[3],
            gw[0], gw[1], gw[2], gw[3],
            mask[0], mask[1], mask[2], mask[3]);
        if (n <= 0 || n >= (int)sizeof(line)) return;
        if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

        n = snprintf_P(line, sizeof(line), PSTR("DNS #1: %u.%u.%u.%u\n"
            "DNS #2: %u.%u.%u.%u\n"),
            dns0[0], dns0[1], dns0[2], dns0[3],
            dns1[0], dns1[1], dns1[2], dns1[3]);
        if (n <= 0 || n >= (int)sizeof(line)) return;
        if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;
    }

    if (!writeCredentialProgmem(client, PSTR("\n[MQTT / DNS]\n"))) return;

    n = snprintf_P(line, sizeof(line), PSTR("MQTT configured: %s\nMQTT connected: %s\nMQTT client state (last result): %d ("),
        mqttSettingsReady() ? "YES" : "NO",mqttClient.connected() ? "YES" : "NO",mqttLastAttemptState);
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;
    if (!writeCredentialProgmem(client,mqttStateDescription(mqttLastAttemptState))) return;
    n = snprintf_P(line, sizeof(line), PSTR(")\nMQTT client state (current): %d ("), mqttClient.state());
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;
    if (!writeCredentialProgmem(client,mqttStateDescription(mqttClient.state()))) return;
    if (!writeCredentialProgmem(client,PSTR(")\nDNS lookup: "))) return;
    if (!writeCredentialProgmem(client,dnsLookupDescription(dnsAttempted,dnsAddressUsable))) return;
    n = snprintf_P(line, sizeof(line), PSTR("\nDNS resolver return code: %d\nMQTT DNS usable: %s\n"),
                 dnsResult,dnsAddressUsable ? "YES" : "NO");
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

    n = snprintf_P(line, sizeof(line), PSTR("MQTT resolved IP: %u.%u.%u.%u\n"
        "DNS address class: %s\n"),
        resolved[0], resolved[1], resolved[2], resolved[3],
        resolvedBroadcast ? "INVALID/BROADCAST"
            : (resolvedAllZero ? "INVALID/ZERO"
                               : (dnsAddressUsable ? "USABLE" : "UNRESOLVED")));
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

    const unsigned long lastAttemptAgeSec =
        mqttLastAttemptMs == 0 ? 0UL : (millis() - mqttLastAttemptMs) / 1000UL;
    n = snprintf_P(line, sizeof(line), PSTR("MQTT connect attempts: %lu\n"
        "MQTT connect failures: %lu\n"
        "MQTT state -2 runtime streak: %u\n"
        "Network stack self-heals: %lu\n"
        "MQTT update retry delay ms: %lu\n"),
        (unsigned long)mqttConnectAttempts,
        (unsigned long)mqttConnectFailures,
        (unsigned)mqttRuntimeConnectFailureStreak,
        (unsigned long)networkStackRecoveryCount,
        (unsigned long)mqttUpdateRetryDelayMs);
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

    n = snprintf_P(line, sizeof(line), PSTR("Last connect succeeded: %s\n"
        "Last attempt age seconds: %lu\n"
        "Last status publish: %s\n"
        "Request subscribe: %s\n"
        "Admin subscribe: %s\n"),
        mqttLastConnectSucceeded ? "YES" : "NO",
        lastAttemptAgeSec,
        mqttLastStatusPublishSucceeded ? "OK" : "NO/NOT YET",
        mqttLastRequestSubscribeSucceeded ? "OK" : "NO/NOT YET",
        mqttLastAdminSubscribeSucceeded ? "OK" : "NO/NOT YET");
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

    if (!writeCredentialProgmem(client, PSTR("\n[TLS / MFLN]\n"))) return;

    n = snprintf_P(line, sizeof(line), PSTR("TLS RX configured bytes: %u\n"
        "TLS TX configured bytes: %u\n"
        "Small TLS RX active: %s\n"
        "Compatibility TLS RX active: %s\n"
        "MFLN checked: %s\n"
        "MFLN supported: %s\n"),
        (unsigned)mqttTlsRecvConfiguredBytes,
        (unsigned)MQTT_TLS_TX_BYTES,
        mqttSmallTlsBufferActive ? "YES" : "NO",
        mqttCompatTlsBufferActive ? "YES" : "NO",
        mqttMflnChecked ? "YES" : "NO",
        mqttMflnSupported ? "YES" : "NO");
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

    n = snprintf_P(line, sizeof(line), PSTR("MFLN negotiated last connect: %s\n"
        "MFLN probe count: %lu\n"
        "MFLN last probe ms: %lu\n"),
        mqttMflnNegotiatedAtLastConnect ? "YES" : "NO",
        (unsigned long)mqttMflnProbeCount,
        (unsigned long)mqttMflnProbeDurationMs);
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

    n = snprintf_P(line, sizeof(line), PSTR("Last SSL error code: %d\n"
        "Last SSL error text present: %s\n"),
        mqttLastSslErrorCode,
        mqttLastSslErrorText[0] != '\0' ? "YES" : "NO");
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

    if (mqttLastSslErrorText[0] != '\0') {
        if (!writeCredentialProgmem(client, PSTR("Last SSL error text: "))) return;
        const size_t errLen = strnlen(mqttLastSslErrorText, sizeof(mqttLastSslErrorText));
        if (!writeRootBytes(client, reinterpret_cast<const uint8_t*>(mqttLastSslErrorText), errLen)) return;
        if (!writeCredentialProgmem(client, PSTR("\n"))) return;
    } else if (mqttLastSslErrorCode == 0) {
        if (!writeCredentialProgmem(client,PSTR("TLS result: no TLS error recorded\n"))) return;
    }

    if (!writeCredentialProgmem(client, PSTR("\n[Memory / Runtime]\n"))) return;

    n = snprintf_P(line, sizeof(line), PSTR("TLS preflight free heap bytes: %lu\n"
        "TLS preflight max block bytes: %u\n"
        "TLS preflight deferred attempts: %lu\n"),
        (unsigned long)mqttConnectHeapAtAttempt,
        (unsigned)mqttConnectBlockAtAttempt,
        (unsigned long)mqttConnectPreflightDeferrals);
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

    n = snprintf_P(line, sizeof(line), PSTR("Free heap bytes: %lu\n"
        "Max free block bytes: %u\n"
        "Heap fragmentation percent: %u\n"
        "Free contiguous stack bytes: %lu\n"
        "Current boot reason: %s (%u)\n"),
        (unsigned long)ESP.getFreeHeap(),
        (unsigned)ESP.getMaxFreeBlockSize(),
        (unsigned)ESP.getHeapFragmentation(),
        (unsigned long)ESP.getFreeContStack(),
        resetReasonLabel(currentBootReset.reason),
        (unsigned)currentBootReset.reason);
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

    if (!applicationAppendDebug(client)) return;
    writeCredentialProgmem(client, PSTR(
        "\nNo Wi-Fi or MQTT passwords are shown in this report.\n"
        "</pre><p><a href='/setup'>Edit credentials</a> · "
        "<a href='/'>Back</a></p></body></html>"));
}
void handleMqttStatusJson() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    char body[512];
    int n = snprintf_P(body, sizeof(body), PSTR(
        "{\"device\":\"%s\",\"firmware\":\"%s\",\"wifi_connected\":%s,"
        "\"mqtt_configured\":%s,\"mqtt_connected\":%s,\"mqtt_state\":%d,"
        "\"mqtt_last_state\":%d,\"mqtt_attempts\":%lu,\"mqtt_failures\":%lu,"
        "\"tls_error\":%d,\"heap_free\":%lu,\"heap_largest\":%u,"
        "\"connect_heap\":%lu,\"connect_block\":%u,\"preflight_deferrals\":%lu,\"tls_rx\":%u}"),
        mqttDeviceId, VERSION, WiFi.status() == WL_CONNECTED ? "true" : "false",
        mqttSettingsReady() ? "true" : "false", mqttClient.connected() ? "true" : "false",
        mqttClient.state(), mqttLastAttemptState,
        (unsigned long)mqttConnectAttempts, (unsigned long)mqttConnectFailures,
        mqttLastSslErrorCode, (unsigned long)ESP.getFreeHeap(), (unsigned)ESP.getMaxFreeBlockSize(),
        (unsigned long)mqttConnectHeapAtAttempt, (unsigned)mqttConnectBlockAtAttempt,
        (unsigned long)mqttConnectPreflightDeferrals, (unsigned)mqttTlsRecvConfiguredBytes);
    server.sendHeader("Cache-Control", "no-store");
    if (n <= 0 || n >= (int)sizeof(body)) {
        server.send(503, "application/json", "{\"error\":\"diagnostics_overflow\"}");
        return;
    }
    server.send(200, "application/json; charset=utf-8", body);
}
void configureCredentialRoutes() {
    if (credentialRoutesConfigured) return;
    const char* headers[] = {"Cookie"};
    server.collectHeaders(headers, 1);
    server.on("/ap-login", HTTP_POST, handleApWebLogin);
    server.on("/setup", HTTP_GET, handleCredentialSetupPage);
    server.on("/setup-save", HTTP_POST, handleCredentialSave);
    server.on("/forget-credentials-confirm", HTTP_GET, handleForgetCredentialsConfirm);
    server.on("/forget-credentials", HTTP_POST, handleForgetCredentials);
    server.on("/network-debug", HTTP_GET, handleNetworkDebugPage);
    server.on("/mqtt-status", HTTP_GET, handleMqttStatusJson);
    credentialRoutesConfigured = true;
}

bool configureSetupApAddress() {
    // Core 2.7.2 softAPConfig chooses last octet +99..+199 for DHCP.
    // AP .80 consequently produces .179.. .23 after uint8 wrap and fails.
    // Use the profile's STA address, with a DHCP pool above the AP itself.
    const IPAddress setupIp(192, 168, 1, SITE_IP_D);
    const IPAddress setupMask(255, 255, 255, 0);
    struct ip_info info = {};
    info.ip.addr = setupIp.v4();
    info.gw.addr = setupIp.v4();
    info.netmask.addr = setupMask.v4();
    wifi_softap_dhcps_stop(); // Already stopped is harmless.
    if (!wifi_set_ip_info(SOFTAP_IF, &info)) return false;
    struct dhcps_lease lease = {};
    lease.enable = true;
    lease.start_ip.addr = IPAddress(192, 168, 1, SITE_IP_D + 1).v4();
    lease.end_ip.addr = IPAddress(192, 168, 1, SITE_IP_D + 20).v4();
    if (!wifi_softap_set_dhcps_lease(&lease) ||
        !wifi_softap_set_dhcps_lease_time(720)) return false;
    uint8_t offerRouter = 1;
    if (!wifi_softap_set_dhcps_offer_option(OFFER_ROUTER, &offerRouter) ||
        !wifi_softap_dhcps_start()) return false;
    struct ip_info actual = {};
    return wifi_get_ip_info(SOFTAP_IF, &actual) &&
           actual.ip.addr == info.ip.addr && actual.netmask.addr == info.netmask.addr;
}

void startCredentialAccessPoint(bool portalOnly) {
    if (credentialPortalActive) return;
    credentialPortalLastAttemptMs = millis();
    ++credentialApAttempts;
    credentialApHeapAtAttempt = ESP.getFreeHeap();
    markCrashStage(CRASH_STAGE_BOOT_AP_START, ESP.getFreeHeap());
    bootCheckpoint(CRASH_STAGE_BOOT_AP_START);

    // Setup/recovery uses the profile's configured STA address. AP+STA with both
    // interfaces on the same address/subnet is invalid/unstable on ESP8266.
    // Therefore every setup AP is deliberately AP-only.
    (void)portalOnly;
    credentialPortalOnly = true;

    // Fully reset the Wi-Fi radio before configuring the AP. This clears
    // stale STA/AP state left by a failed static-IP connection attempt.
    credentialDns.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(180);
    bool apModeReady = WiFi.mode(WIFI_AP);
    delay(120);

    credentialApName = String(DEVICE_LABEL "-") + DEVICE_ID + "-" +
                       String(ESP.getChipId() & 0xFFFFFFUL, HEX);
    credentialApName.toUpperCase();
    credentialApPassword = ""; // Open Wi-Fi; access is gated by the web PIN.
    resetApWebSession();

    bool apConfigured = apModeReady && configureSetupApAddress();
    bool apStarted = false;
    if (apConfigured) {
        apStarted = WiFi.softAP(
            credentialApName.c_str()
        );
    }
    if (!apStarted) {
        delay(250);
        WiFi.softAPdisconnect(true);
        delay(120);
        WiFi.mode(WIFI_OFF);
        delay(180);
        apModeReady = WiFi.mode(WIFI_AP);
        delay(120);
        apConfigured = apModeReady && configureSetupApAddress();
        apStarted = apConfigured && WiFi.softAP(
            credentialApName.c_str()
        );
    }

    credentialPortalActive = apStarted;

    if (!apStarted) {
        credentialApLastFailure = !apModeReady ? 1 : !apConfigured ? 2 : 3;
        char detail[64];
        snprintf(detail, sizeof(detail), "%s; mode=%u heap=%lu. Retry 30s",
                 !apModeReady ? "Radio mode failed" : !apConfigured ? "IP config failed" : "AP start failed",
                 (unsigned)WiFi.getMode(), (unsigned long)credentialApHeapAtAttempt);
        queueOledDiagnostic(OLED_DIAG_SETUP_AP, "SETUP AP FAILED", detail);
        drawOledDiagnostic(millis());
        return;
    }

    IPAddress apIp = WiFi.softAPIP();
    credentialDns.start(53, "*", apIp);

    configureCredentialRoutes();
    server.begin();

    if (drawOledDiagnostic(millis())) return;
    cs();
    doublePrintf(420, 1, 1, "SETUP AP / v%s", VERSION);
    doublePrintf(420, 2, 1, "%s", credentialApName.c_str());
    doublePrintf(420, 3, 1, "WiFi open PIN:%s", AP_WEB_PIN);
    doublePrintf(420, 4, 1, "Open: %s", apIp.toString().c_str());
}

void stopCredentialAccessPoint() {
    if (!credentialPortalActive || credentialPortalOnly) return;
    credentialDns.stop();
    WiFi.softAPdisconnect(true);
    credentialPortalActive = false;
    resetApWebSession();
    credentialApName = "";
    credentialApPassword = "";
    if (WiFi.status() == WL_CONNECTED) WiFi.mode(WIFI_STA);
}

// Save/clear can be requested over LAN, AP or Recovery, including failed AP starts.
bool serviceCredentialRestart(unsigned long now) {
    if (!credentialRestartPending ||
        now - credentialRestartRequestedMs < CREDENTIAL_RESTART_DELAY_MS) return false;
    credentialRestartPending = false;
#if PLATFORM_LOCAL_CONTROL_APPLICATION
    applicationPrepareOta();
#endif
    delay(20); // Allow the already-sent response to leave the socket.
    ESP.restart();
    return true;
}

void serviceCredentialPortal(unsigned long now) {
    if (!credentialPortalActive) {
        // A failed AP start is not a permanent dead end in provisioning/Recovery.
        if (credentialPortalOnly && now - credentialPortalLastAttemptMs >= CREDENTIAL_AP_RETRY_MS) {
            startCredentialAccessPoint(true);
        }
        return;
    }
    credentialDns.processNextRequest();
    server.handleClient();

}

void configureProvisioningRoutes() {
    configureCredentialRoutes();
    server.on("/", HTTP_GET, handleCredentialSetupPage);
    server.onNotFound([]() {
        WebResponseScope responseScope;
        if (!requireApWebPin()) return;
        server.sendHeader("Location", PLATFORM_SETUP_BASE_URL "setup", true);
        server.send(302, "text/plain", "");
    });
}

void configureRecoveryRoutes() {
    configureCredentialRoutes();
    server.on("/", HTTP_GET, handleRecoveryPage);
    server.on("/debug", HTTP_GET, handleRecoveryPage);
    server.on("/debug.txt", HTTP_GET, handleRecoveryText);
    server.on("/restart-normal", HTTP_GET, handleRecoveryRestartNormal);
    server.on("/esp-restart", HTTP_POST, handleRecoveryEspRestart);
    server.onNotFound([]() {
        WebResponseScope responseScope;
        if (!requireApWebPin()) return;
        server.sendHeader("Cache-Control", "no-store");
        server.send(503, "text/plain; charset=utf-8",
                    "OTA-ONLY RECOVERY: open /debug or /restart-normal; POST /esp-restart for safe restart.");
    });
}

// The recovery loop never calls applicationReadMeasurements(), PubSubClient,
// tariff, OLED measurement drawing, EEPROM accounting or admin commands.
void serviceRecoveryOnly(unsigned long now) {
    if (recoveryEspRestartPending &&
        now - recoveryEspRestartRequestedMs >= RECOVERY_ESP_RESTART_DELAY_MS) {
        // Never clear RTC trace or the stored Recovery incident here.
        delay(20);
        ESP.restart();
        return;
    }
    if (recoveryNormalRestartPending &&
        now - recoveryNormalRestartRequestedMs >= RECOVERY_NORMAL_RESTART_DELAY_MS) {
        clearRtcCrashTrace();
        delay(20);
        ESP.restart();
        return;
    }

    // Recovery OTA works on the normal Wi-Fi station or on the dedicated
    // setup AP. In both cases IDE_OTA is advertised through mDNS.
    if (!networkServicesStarted &&
        (WiFi.status() == WL_CONNECTED || credentialPortalActive)) {
        markCrashStage(CRASH_STAGE_NETWORK_SERVICES, ESP.getFreeHeap());
        startNetworkServices();
    }

    if (networkServicesStarted) {
        if (!credentialPortalActive) server.handleClient();
        ArduinoOTA.handle();
    }

    if (WiFi.status() != WL_CONNECTED && !credentialPortalOnly &&
        (recoveryWifiRetryMs == 0 ||
         now - recoveryWifiRetryMs >= RECOVERY_WIFI_RETRY_MS)) {
        recoveryWifiRetryMs = now;
        WiFi.reconnect();
    }

    if (recoveryDisplayMs == 0 || now - recoveryDisplayMs >= 3000UL) {
        recoveryDisplayMs = now;
        if (!recoveryOtaInProgress) showRecoveryScreen();
    }
    yield();
}

void configureWebRoutes() {
    configureCredentialRoutes();
    server.on("/live", HTTP_GET, handleRoot);
    server.on("/", HTTP_GET, []() {
        if (credentialPortalOnly) handleCredentialSetupPage();
        else handleRoot();
    });
    server.on("/force-ota-recovery-confirm", HTTP_GET, handleForceOtaRecoveryConfirm);
    server.on("/force-ota-recovery", HTTP_POST, handleForceOtaRecovery);
    server.onNotFound([]() {
        WebResponseScope responseScope;
        if (!requireApWebPin()) return;
        if (credentialPortalOnly) {
            server.sendHeader("Location", PLATFORM_SETUP_BASE_URL "setup", true);
            server.send(302, "text/plain", "");
            return;
        }
        server.send(404, "text/plain; charset=utf-8", "Not found");
    });
#if PLATFORM_LOCAL_CONTROL_APPLICATION
    applicationConfigureWebRoutes();
#endif
}

void startNetworkServices() {
    const bool networkReady =
        WiFi.status() == WL_CONNECTED || (recoveryMode && credentialPortalActive);
    if (networkServicesStarted || !networkReady) return;

    server.begin();

    if (recoveryMode) {
        // IDE_OTA exists only in OTA-ONLY Recovery. Passing true enables
        // mDNS advertisement so Arduino IDE clients can discover the ESP8266.
        otaHostname = mqttDeviceId;
        otaHostname.replace("_", "-");
        ArduinoOTA.setHostname(otaHostname.c_str());
        ArduinoOTA.onStart([]() {
            recoveryOtaInProgress = true;
            cs();
            doublePrintf(420, 1, 1, "RECOVERY / OTA");
            doublePrintf(420, 2, 1, "Uploading firmware");
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            if (total > 0) {
                platformDisplay.font(DisplayFont::Normal10);
                char percentText[24];
                snprintf(percentText, sizeof(percentText), "Progress %u%%",
                         (unsigned)((uint64_t)progress * 100UL / total));
                platformDisplay.text(0, 45, percentText);
                platformDisplay.present();
            }
        });
        ArduinoOTA.onEnd([]() {
            bool clearSucceeded = true;
            if (littleFsReady && LittleFS.exists(RECOVERY_FILE)) {
                clearSucceeded = LittleFS.remove(RECOVERY_FILE);
            }
            recoveryOtaClearFailed = !clearSucceeded;
            if (clearSucceeded) recoveryIncidentPersistent = false;

            if (!clearSucceeded) {
                queueOledDiagnostic(OLED_DIAG_RECOVERY_CLEAR, "RECOVERY CLEAR FAILED", "Firmware received; Recovery record was not cleared");
                drawOledDiagnostic(millis());
                return;
            }

            cs();
            doublePrintf(420, 1, 1, "OTA SUCCESS");
            doublePrintf(420, 2, 1,
                         clearSucceeded ? "Recovery cleared" : "Recovery NOT cleared");
            doublePrintf(420, 3, 1,
                         clearSucceeded ? "Reboot -> NORMAL" : "Use physical RST");
        });
        ArduinoOTA.onError([](ota_error_t error) {
            recoveryOtaInProgress = false;
            recoveryOtaClearFailed = false;
            char detail[64];
            snprintf(detail, sizeof(detail), "IDE_OTA error=%u. Try OTA again", (unsigned)error);
            queueOledDiagnostic(OLED_DIAG_OTA, "RECOVERY OTA FAILED", detail);
            drawOledDiagnostic(millis());
        });
        ArduinoOTA.begin(true);
    } else {
        configTime(GREECE_TZ, "pool.ntp.org", "time.nist.gov");
    }

    networkServicesStarted = true;
}

void observeWiFiDisconnect(const WiFiEventStationModeDisconnected& event) {
    // Explicit retry disconnects and entry to AP must not overwrite an actual
    // association failure. Never infer a changed password from an outage alone.
    if (credentialPortalOnly || WiFi.getMode() != WIFI_STA) return;
    if (event.reason == WIFI_DISCONNECT_REASON_AUTH_LEAVE ||
        event.reason == WIFI_DISCONNECT_REASON_ASSOC_LEAVE) return;
    wifiLastDisconnectReason = (uint16_t)event.reason;
    char detail[64];
    snprintf(detail, sizeof(detail), "SDK reason=%u. Saved settings kept; retrying", (unsigned)event.reason);
    queueOledDiagnostic(OLED_DIAG_WIFI, wifiFailureLabel(), detail);
}

void connectWiFiInitial(bool waitForConnection) {
    bootCheckpoint(CRASH_STAGE_WIFI_CONNECT);
    if (!wifiDisconnectHandler) {
        // Keep the handler alive; dropping its shared_ptr unsubscribes it.
        wifiDisconnectHandler = WiFi.onStationModeDisconnected(observeWiFiDisconnect);
    }
    WiFi.persistent(false); // Repeated retries must not wear SDK flash sectors.
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false); // Explicit, bounded-cadence background attempts.

#if USE_STATIC_IP
    IPAddress local_IP(SITE_IP_A, SITE_IP_B, SITE_IP_C, SITE_IP_D);
    IPAddress gateway(SITE_GW_A, SITE_GW_B, SITE_GW_C, SITE_GW_D);
    IPAddress subnet(SITE_MASK_A, SITE_MASK_B, SITE_MASK_C, SITE_MASK_D);
    IPAddress dns(SITE_DNS_A, SITE_DNS_B, SITE_DNS_C, SITE_DNS_D);
    WiFi.config(local_IP, gateway, subnet, dns);
#endif

    WiFi.begin(ssid, password);
    wifiConnectAttempts++;
    lastWifiRetry = millis();
    wifiRetryStep = 0;

    if (!waitForConnection) return; // Normal metering must not wait for a router.

    unsigned long wifiStart = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 10000UL) {
        delay(250);
        yield();
    }
}

bool serviceNetworkStackRecovery(unsigned long now) {
    if (!networkStackRecoveryPending || credentialPortalOnly || recoveryMode) return false;

    networkStackRecoveryPending = false;
    ++networkStackRecoveryCount;
    queueOledDiagnostic(OLED_DIAG_MQTT, "NETWORK SELF-HEAL",
                        "MQTT state -2 persisted; recycling Wi-Fi/TCP without reboot");

    // Release TLS and all HTTP/TCP state, then re-associate using the same
    // saved credentials/static IP. Meter/accounting state remains untouched.
    if (mqttClient.connected()) mqttClient.disconnect();
    mqttNet.stop(1);
    server.stop();
    networkServicesStarted = false;

    WiFi.disconnect(false);
    WiFi.mode(WIFI_OFF);
    delay(50);

    wifiWasConnected = false;
    wifiOfflineSinceMs = now;
    lastMqttReconnectMs = 0;
    mqttRetryStep = 0;
    mqttRetryDelayMs = MQTT_RUNTIME_RETRY_STEPS_MS[0];
    mqttMflnChecked = false;
    mqttMflnSupported = false;
    mqttSmallTlsBufferActive = false;
    mqttCompatTlsBufferActive = false;
    mqttTlsRecvConfiguredBytes = 16384;
    resetMqttRuntimeFailureStreak();

    connectWiFiInitial(false);
    return true;
}

void serviceWiFiReconnect(unsigned long now) {
    if (credentialPortalOnly || ssid[0] == '\0') return;
    if (WiFi.status() == WL_CONNECTED) {
        if (!wifiWasConnected) {
            lastMqttReconnectMs = 0;
            mqttRetryStep = 0;
            mqttRetryDelayMs = mqttEverConnected
                ? MQTT_RUNTIME_RETRY_STEPS_MS[0]
                : MQTT_RETRY_STEPS_MS[0];
        }
        wifiWasConnected = true;
        wifiRetryStep = 0;
        wifiOfflineSinceMs = 0;
        wifiLastDisconnectReason = 0;
        return;
    }

    if (wifiOfflineSinceMs == 0) wifiOfflineSinceMs = now;
    if (wifiWasConnected) {
        wifiWasConnected = false;
        mqttNet.stop(0); // Release an obsolete TLS transport once, not each loop.
        lastWifiRetry = millis();
        wifiRetryStep = 0;
        return; // now predates stop(); do not underflow the retry-age calculation.
    }
    if (now - lastWifiRetry < WIFI_RETRY_STEPS_MS[wifiRetryStep]) return;

    markCrashStage(CRASH_STAGE_WIFI_CONNECT, ESP.getFreeHeap());
    WiFi.disconnect(false);
    WiFi.begin(ssid, password); // Starts association; no WL_CONNECTED wait loop.
    wifiConnectAttempts++;
    lastWifiRetry = millis();
    if (wifiRetryStep + 1 < WIFI_RETRY_STEP_COUNT) wifiRetryStep++;
}

// Both millis() and digitalRead() are IRAM-safe in the pinned ESP8266 core.
// Latch a debounced PRESS+RELEASE even while a bounded TLS/Modbus call is busy.
// No Wi-Fi, display, allocation or flash operation is allowed in this ISR.
void ICACHE_RAM_ATTR flashSetupButtonIsr() {
    const uint32_t now = millis();
    if (digitalRead(FLASH_SETUP_PIN) == LOW) {
        if (!flashPressActive) {
            flashPressStartedMs = now;
            flashPressActive = true;
        }
    } else if (flashPressActive) {
        if (now - flashPressStartedMs >= FLASH_DEBOUNCE_MS) flashSetupRequested = true;
        flashPressActive = false;
    }
}

void configureFlashSetupButton() {
#if PLATFORM_FLASH_BUTTON_ENABLED
    pinMode(FLASH_SETUP_PIN, INPUT_PULLUP);
    flashPressActive = false;
    flashSetupRequested = false;
    attachInterrupt(digitalPinToInterrupt(FLASH_SETUP_PIN), flashSetupButtonIsr, CHANGE);
#endif
}

void exitManualSetupAccessPoint() {
    if (recoveryMode || !normalMeterInitialized || !credentialPortalOnly) return;
    credentialDns.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    credentialPortalActive = false;
    resetApWebSession();
    credentialPortalOnly = false;
    credentialApName = "";
    credentialApPassword = "";
    wifiWasConnected = false;
    wifiLastDisconnectReason = 0;
    wifiOfflineSinceMs = millis();
    networkServicesStarted = false;
    wifiHintVisible = false;
    wifiHintDrawPending = false;
    wifiHintStartedMs = millis();
    connectWiFiInitial(false);
    applicationRenderMeasurements(lastMeterReadOk);
}

void serviceFlashSetupButton(unsigned long now) {
#if !PLATFORM_FLASH_BUTTON_ENABLED
    (void)now;
    return;
#endif
    if (!normalMeterInitialized && !recoveryMode) return;
    noInterrupts();
    const bool requested = flashSetupRequested;
    flashSetupRequested = false;
    interrupts();
    if (!requested) return;
    if (flashActionTaken && now - flashLastActionMs < FLASH_ACTION_COOLDOWN_MS) return;

    if (credentialRestartPending || forceRecoveryRestartPending) return;
    if (recoveryMode) {
        if (recoveryOtaInProgress || recoveryNormalRestartPending) return;
        // Retry/open setup without clearing the Recovery latch or enabling meters.
        if (!credentialPortalActive) startCredentialAccessPoint(true);
        showRecoveryScreen();
        flashActionTaken = true;
        flashLastActionMs = millis();
        return;
    }
    if (credentialPortalOnly) {
        exitManualSetupAccessPoint();
    } else {
        // Manual setup is AP-only because STA and AP share 192.168.1.80.
        // Local reads/accounting remain enabled; saved credentials are untouched.
        mqttNet.stop(0);
        wifiWasConnected = false;
        networkServicesStarted = false;
        startCredentialAccessPoint(true);
        wifiHintVisible = true;
        wifiHintDrawPending = true;
        wifiHintStartedMs = millis();
    }
    flashActionTaken = true;
    flashLastActionMs = millis(); // Cooldown begins after the radio change finishes.
}

const char* wifiFailureLabel() {
    if (wifiLastDisconnectReason == WIFI_DISCONNECT_REASON_NO_AP_FOUND ||
        WiFi.status() == WL_NO_SSID_AVAIL) return "SSID=Not found";
    if (wifiLastDisconnectReason == WIFI_DISCONNECT_REASON_AUTH_FAIL ||
        wifiLastDisconnectReason == WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED) {
        return "WiFi auth failed";
    }
    if (wifiLastDisconnectReason == WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT ||
        wifiLastDisconnectReason == WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT) {
        return "WiFi handshake timeout";
    }
    return "WiFi offline";
}

void drawWiFiStatusHint(unsigned long now) {
    char line[32];
    platformDisplay.clear();
    platformDisplay.font(DisplayFont::Medium6x12);
    if (credentialPortalOnly) {
        platformDisplay.text(0, 12, credentialPortalActive ? "AP SETUP / LOCAL" : "SETUP AP failed");
        if (credentialPortalActive) {
            platformDisplay.font(DisplayFont::Tiny4x6); // Long JSY SSID still fits all 128 pixels.
            platformDisplay.text(0, 28, credentialApName.c_str());
            platformDisplay.font(DisplayFont::Small5x7);
            snprintf(line, sizeof(line), "WiFi open PIN:%s", AP_WEB_PIN);
            platformDisplay.text(0, 44, line);
            platformDisplay.font(DisplayFont::Tiny4x6);
            platformDisplay.text(0, 60, PLATFORM_SETUP_IP_TEXT "/setup FLASH:exit");
        } else {
            platformDisplay.font(DisplayFont::Small5x7);
            platformDisplay.text(0, 28, "Retrying AP every 30s");
            platformDisplay.text(0, 44, "Local meter active");
            platformDisplay.text(0, 60, "FLASH: return to WiFi");
        }
    } else {
        platformDisplay.font(DisplayFont::Small5x7);
        platformDisplay.text(0, 12, wifiFailureLabel());
        const bool checkPassword =
            wifiLastDisconnectReason == WIFI_DISCONNECT_REASON_AUTH_FAIL ||
            wifiLastDisconnectReason == WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED ||
            wifiLastDisconnectReason == WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT ||
            wifiLastDisconnectReason == WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT;
        platformDisplay.text(0, 28, checkPassword ? "Check WiFi/password" : "Local meter active");
        const unsigned long age = now - lastWifiRetry;
        const unsigned long gap = WIFI_RETRY_STEPS_MS[wifiRetryStep];
        const unsigned long remaining = age >= gap ? 0 : gap - age;
        snprintf(line, sizeof(line), "Retry in %lus", (remaining + 999UL) / 1000UL);
        platformDisplay.text(0, 44, line);
        platformDisplay.text(0, 60, "FLASH: AP setup");
    }
    platformDisplay.present();
}

void serviceWiFiStatusDisplay(unsigned long now, bool meterDisplayUpdated) {
    if (!normalMeterInitialized || recoveryMode || bootCheckpointsActive) return;
    if (oledDiagnosticCount) return; // A sticky failure takes precedence over hints.
    if (!credentialPortalOnly && WiFi.status() == WL_CONNECTED) {
        if (wifiHintVisible && !meterDisplayUpdated) applicationRenderMeasurements(lastMeterReadOk);
        wifiHintVisible = false;
        wifiHintDrawPending = false;
        wifiHintStartedMs = now;
        return;
    }

    if (wifiHintVisible && now - wifiHintStartedMs >= WIFI_HINT_DURATION_MS) {
        wifiHintVisible = false;
        wifiHintDrawPending = false;
        if (!meterDisplayUpdated) applicationRenderMeasurements(lastMeterReadOk);
        return;
    }
    if (!wifiHintVisible && now - wifiHintStartedMs >= WIFI_HINT_INTERVAL_MS) {
        wifiHintVisible = true;
        wifiHintDrawPending = true;
        wifiHintStartedMs = now;
    }
    if (wifiHintVisible && (wifiHintDrawPending || meterDisplayUpdated ||
        now - wifiHintLastFrameMs >= 1000UL)) {
        drawWiFiStatusHint(now);
        wifiHintLastFrameMs = millis();
        wifiHintDrawPending = false;
    }
}

void serviceOledDiagnostic(unsigned long now) {
    if (!oledDiagnosticCount || !bootOledReady) return;
    if (!oledDiagnosticStarted || now - oledDiagnosticLastFrameMs >= 1000UL) {
        if (!drawOledDiagnostic(now)) {
            if (recoveryMode) showRecoveryScreen();
            else if (credentialPortalOnly) drawWiFiStatusHint(now);
            else if (normalMeterInitialized) applicationRenderMeasurements(lastMeterReadOk);
        }
    }
}

// ==========================================================
// END MODULE: RECOVERY / WIFI / NETWORK
// ==========================================================

// Shared dashboard presentation and request lifecycle. Application modules
// contribute only their domain content, never a second page shell or footer.
bool applicationRenderWebContent(WiFiClient& client);
bool applicationWebLiveUpdates();

bool platformWebWriteHttpHeader(WiFiClient& client) {
    PGM_P header = PSTR(
        "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: no-store\r\nConnection: close\r\n\r\n");
    return writeRootProgmemViaRam(client, header, strlen_P(header));
}

bool platformWebWritePageStart(WiFiClient& client, PGM_P title) {
    PGM_P head = PSTR(
        "<!doctype html><html><head><meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>");
    if (!writeRootProgmemViaRam(client, head, strlen_P(head)) ||
        !writeRootProgmemViaRam(client, title, strlen_P(title))) return false;
    PGM_P style = PSTR(
        "</title><style>"
        "body{font:22px/1.35 Arial,sans-serif;max-width:760px;margin:auto;padding:4px;color:#142431;background:#fff}"
        "h1,h2{font-size:34px;margin:0 0 6px}h3{font-size:inherit;margin:16px 0 6px}"
        ".dim{color:#64727c}pre{font:inherit;white-space:pre-wrap;overflow-wrap:anywhere;margin:6px 0}"
        ".utility-title{display:flex;flex-wrap:wrap;align-items:baseline;gap:6px 16px;margin-top:10px}"
        ".utility-title h3{margin:0}.utility-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:12px;margin-top:0}"
        ".utility-grid div{overflow-wrap:anywhere}.table-wrap,.scroll{overflow-x:auto}"
        "table{border-collapse:collapse;width:100%;max-width:400px;font:inherit}"
        ".scroll table{max-width:none}canvas{max-width:100%;background:#fff}"
        "th,td{padding:5px 7px;border-bottom:1px solid #e5eaed;text-align:right;white-space:nowrap}"
        "th:first-child,td:first-child{text-align:left}a{margin-right:10px}p{margin:10px 0}"
        "button,input{font:inherit;padding:6px;max-width:100%;box-sizing:border-box}"
        "button{margin:4px 0}form{margin:9px 0}fieldset{margin:13px 0;min-width:0}"
        "@media(max-width:480px){.utility-grid{grid-template-columns:1fr}}"
        "</style></head><body><h2>");
    if (!writeRootProgmemViaRam(client, style, strlen_P(style)) ||
        !writeRootProgmemViaRam(client, title, strlen_P(title))) return false;
    PGM_P end = PSTR("</h2>");
    return writeRootProgmemViaRam(client, end, strlen_P(end));
}

bool platformWebWriteStatus(WiFiClient& client) {
    PGM_P links = PSTR(
        "<section class='platform-status'><p><a href='/network-debug'>Network Debug</a>"
        "<a href='/setup'>Network Setup</a></p>");
    if (!writeRootProgmemViaRam(client, links, strlen_P(links))) return false;
    char line[192];
    const int n = snprintf_P(line, sizeof(line),
        PSTR("<pre>Wi-Fi: %s\nMQTT: %s\nRSSI: %d dBm\nFree heap: %lu bytes</pre></section>"),
        WiFi.status() == WL_CONNECTED ? "CONNECTED" : "OFFLINE",
        mqttClient.connected() ? "CONNECTED" : "DISCONNECTED",
        WiFi.RSSI(), (unsigned long)ESP.getFreeHeap());
    return n > 0 && n < (int)sizeof(line) &&
        writeRootBytes(client, reinterpret_cast<const uint8_t*>(line), (size_t)n);
}

bool platformWebWriteFooter(WiFiClient& client, const char* dateTime, bool liveUpdates) {
    PGM_P footer = PSTR(
        "<p><a href='/force-ota-recovery-confirm'>Force OTA-ONLY Recovery...</a></p>"
        "<div class='dim'>Device: " DEVICE_ID " · Meter: " DEVICE_KIND "</div>");
    if (!writeRootProgmemViaRam(client, footer, strlen_P(footer))) return false;
    char line[192];
    int n = snprintf_P(line, sizeof(line), PSTR("<div class='dim'>MQTT ID: %s</div>"), mqttDeviceId);
    if (n <= 0 || n >= (int)sizeof(line) ||
        !writeRootBytes(client, reinterpret_cast<const uint8_t*>(line), (size_t)n)) return false;
    n = snprintf_P(line, sizeof(line),
        PSTR("<div id='device-time' class='dim'>ESP8266 date/time: %s</div>"), dateTime);
    if (n <= 0 || n >= (int)sizeof(line) ||
        !writeRootBytes(client, reinterpret_cast<const uint8_t*>(line), (size_t)n)) return false;
    // Poll only read-only application content. Ventilation forms must retain
    // operator edits until submission, so its application opts out of polling.
    if (liveUpdates) {
        PGM_P script = PSTR(
            "<script>(function(){"
            "const readings=document.getElementById('live-readings'),state=document.getElementById('live-state'),clock=document.getElementById('device-time');"
            "async function update(){const started=Date.now();"
            "if(!document.hidden){const controller=new AbortController();"
            "const timeout=setTimeout(()=>controller.abort(),4000);"
            "try{const response=await fetch('/live',{cache:'no-store',signal:controller.signal});"
            "const text=await response.text();"
            "if(!response.ok||!text.endsWith('<!--live-end-->'))throw new Error('incomplete');"
            "const stamp=text.match(/<!--device-time:([^<]*)-->/);if(!stamp)throw new Error('clock');readings.innerHTML=text.slice(0,-15).replace(stamp[0],'');clock.textContent='ESP8266 date/time: '+stamp[1];state.textContent='';"
            "}catch(error){state.textContent='Waiting for live measurements...';}"
            "finally{clearTimeout(timeout);}}"
            "setTimeout(update,Math.max(0,5000-(Date.now()-started)));}"
            "setTimeout(update,5000);})();</script>");
        if (!writeRootProgmemViaRam(client, script, strlen_P(script))) return false;
    }
    PGM_P end = PSTR("</body></html>");
    return writeRootProgmemViaRam(client, end, strlen_P(end));
}

void handleRoot() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    const bool liveRequest = server.uri() == "/live";
    WiFiClient client = server.client();
    client.setTimeout(700);
    client.setNoDelay(true);
    ++webRootRequestCount;
    const unsigned long now = millis();
    if (webRootRequestCount > 1 && now - webRootLastRequestMs < WEB_ROOT_MIN_REQUEST_GAP_MS) {
        ++webRootRateLimitedCount;
        sendRootBusyAndClose(client,429,ESP.getFreeHeap(),ESP.getMaxFreeBlockSize());
        return;
    }
    webRootLastRequestMs = now;
    uint32_t freeHeap = 0;
    uint16_t maxBlock = 0;
    if (!webRootHeapHealthy(&freeHeap, &maxBlock, nullptr) ||
        freeHeap < PLATFORM_WEB_MIN_HEAP_BYTES || maxBlock < PLATFORM_WEB_MIN_BLOCK_BYTES) {
        ++webRootBusyCount;
        markCrashStage(CRASH_STAGE_WEB_ROOT, freeHeap);
        sendRootBusyAndClose(client,503,freeHeap,maxBlock);
        return;
    }
    markCrashStage(CRASH_STAGE_WEB_ROOT, freeHeap);
    if (!platformWebWriteHttpHeader(client)) return;
    if (!liveRequest) {
        if (!platformWebWritePageStart(client, PSTR(DEVICE_LABEL " v" VERSION))) return;
        PGM_P start = PSTR("<div id='live-readings'>");
        if (!writeRootProgmemViaRam(client, start, strlen_P(start))) return;
    }
    if (!applicationRenderWebContent(client) || !platformWebWriteStatus(client)) return;
    char dateTime[32];
    formatCurrentDateTime(dateTime, sizeof(dateTime));
    if (liveRequest) {
        char line[80];
        const int n = snprintf_P(line, sizeof(line), PSTR("<!--device-time:%s-->"), dateTime);
        if (n <= 0 || n >= (int)sizeof(line) ||
            !writeRootBytes(client, reinterpret_cast<const uint8_t*>(line), (size_t)n)) return;
        PGM_P end = PSTR("<!--live-end-->");
        if (!writeRootProgmemViaRam(client, end, strlen_P(end))) return;
    } else {
        PGM_P end = PSTR("</div><div id='live-state' class='dim' aria-live='polite'></div>");
        if (!writeRootProgmemViaRam(client, end, strlen_P(end)) ||
            !platformWebWriteFooter(client, dateTime, applicationWebLiveUpdates())) return;
    }
    closeRootClient(client, true);
}

// BEGIN APPLICATION IMPLEMENTATION
bool isDualZoneTimeValid() {
    return isSystemTimeValid();
}

bool isWinterTariffMonth(int month1to12) {
    return month1to12 >= 11 || month1to12 <= 3;
}

bool isZ2AtEpoch(time_t epochValue) {
    struct tm localTm;
    if (!localtime_r(&epochValue, &localTm)) return false;

    int month = localTm.tm_mon + 1;
    int minuteOfDay = localTm.tm_hour * 60 + localTm.tm_min;

    if (isWinterTariffMonth(month)) {
        return (minuteOfDay >= 2 * 60 && minuteOfDay < 5 * 60) ||
               (minuteOfDay >= 12 * 60 && minuteOfDay < 15 * 60);
    }

    return (minuteOfDay >= 2 * 60 && minuteOfDay < 4 * 60) ||
           (minuteOfDay >= 11 * 60 && minuteOfDay < 15 * 60);
}

const char* currentTariffZone() {
    if (!dualZoneEnabled || !isDualZoneTimeValid()) return "-";
    return isZ2AtEpoch(time(nullptr)) ? "Z2" : "Z1";
}

uint32_t v3ChecksumBytes(const void* data, size_t length) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    uint32_t h = 2166136261UL;
    for (size_t i = 0; i < length; ++i) {
        h ^= p[i];
        h *= 16777619UL;
    }
    return h;
}

uint32_t v3StateChecksum(const V3AccountingState& rec) {
    return v3ChecksumBytes(&rec, offsetof(V3AccountingState, checksum));
}

uint32_t v3CheckpointChecksum(const V3CheckpointRecord& rec) {
    return v3ChecksumBytes(&rec, offsetof(V3CheckpointRecord, checksum));
}

uint32_t v3UtilityChecksum(const V3UtilityReading& rec) {
    return v3ChecksumBytes(&rec, offsetof(V3UtilityReading, checksum));
}

bool v3StateValid(const V3AccountingState& rec) {
    return rec.magic == V3_STATE_MAGIC &&
           rec.version == V3_DATA_VERSION &&
           rec.checksum == v3StateChecksum(rec);
}

bool v3CheckpointValid(const V3CheckpointRecord& rec) {
    return rec.magic == V3_CHECKPOINT_MAGIC &&
           rec.version == V3_DATA_VERSION &&
           rec.checksum == v3CheckpointChecksum(rec);
}

bool v3UtilityValid(const V3UtilityReading& rec) {
    return rec.magic == V3_UTILITY_MAGIC &&
           rec.version == V3_DATA_VERSION &&
           rec.checksum == v3UtilityChecksum(rec);
}

void initializeV3CleanStart() {
    if (!littleFsReady) return;
    if (!LittleFS.exists(V3_CHECKPOINT_FILE) && LittleFS.exists("/energy_v3_cp.bak")) {
        LittleFS.rename("/energy_v3_cp.bak", V3_CHECKPOINT_FILE);
    }
    if (LittleFS.exists(V3_CLEAN_MARKER_FILE)) return;

    // Never delete existing V3 history merely because the marker is missing.
    if (LittleFS.exists(V3_STATE_FILE) || LittleFS.exists(V3_CHECKPOINT_FILE) ||
        LittleFS.exists(V3_UTILITY_FILE)) {
        File marker = LittleFS.open(V3_CLEAN_MARKER_FILE, "w");
        if (marker) { marker.print("3.00"); marker.close(); }
        return;
    }

    // Explicit v3 clean start: no legacy energy history survives.
    if (LittleFS.exists(LEGACY_STATS_FILE)) LittleFS.remove(LEGACY_STATS_FILE);
    if (LittleFS.exists(LEGACY_REGISTER_FILE)) LittleFS.remove(LEGACY_REGISTER_FILE);

    // A partially-created pre-v3 test file must not become production history.
    if (LittleFS.exists(V3_STATE_FILE)) LittleFS.remove(V3_STATE_FILE);
    if (LittleFS.exists(V3_CHECKPOINT_FILE)) LittleFS.remove(V3_CHECKPOINT_FILE);
    if (LittleFS.exists(V3_UTILITY_FILE)) LittleFS.remove(V3_UTILITY_FILE);

    File marker = LittleFS.open(V3_CLEAN_MARKER_FILE, "w");
    if (marker) {
        marker.print("3.00");
        marker.close();
    }
}

bool saveV3State() {
    if (!littleFsReady) return false;
    v3State.magic = V3_STATE_MAGIC;
    v3State.version = V3_DATA_VERSION;
    v3State.checksum = v3StateChecksum(v3State);

    File f = LittleFS.open(V3_STATE_FILE, "w");
    if (!f) return false;
    bool ok = f.write(reinterpret_cast<const uint8_t*>(&v3State),
                      sizeof(v3State)) == sizeof(v3State);
    f.close();
    return ok;
}

bool loadV3State() {
    memset(&v3State, 0, sizeof(v3State));
    if (!littleFsReady || !LittleFS.exists(V3_STATE_FILE)) return false;

    File f = LittleFS.open(V3_STATE_FILE, "r");
    if (!f) return false;
    bool ok = f.size() == (size_t)sizeof(v3State) &&
              f.readBytes(reinterpret_cast<char*>(&v3State),
                          sizeof(v3State)) == sizeof(v3State);
    f.close();

    if (!ok || !v3StateValid(v3State)) {
        memset(&v3State, 0, sizeof(v3State));
        return false;
    }
    return true;
}

void loadV3TariffMode() {
    uint32_t magic = 0;
    uint8_t mode = 0;
    EEPROM.get(EEPROM_ADDR_TARIFF_MAGIC, magic);
    EEPROM.get(EEPROM_ADDR_TARIFF_MODE, mode);

    dualZoneEnabled = (magic == TARIFF_MAGIC && mode == 1);
}

void saveV3TariffMode() {
    uint8_t mode = dualZoneEnabled ? 1 : 0;
    EEPROM.put(EEPROM_ADDR_TARIFF_MODE, mode);
    EEPROM.put(EEPROM_ADDR_TARIFF_MAGIC, TARIFF_MAGIC);
    EEPROM.commit();
}


bool compactV3Checkpoints(uint32_t nowEpoch) {
    if (!littleFsReady || !LittleFS.exists(V3_CHECKPOINT_FILE) || nowEpoch < 1600000000UL) return true;

    const char* tmpPath = "/energy_v3_cp.tmp";
    File in = LittleFS.open(V3_CHECKPOINT_FILE, "r");
    if (!in) return false;
    File out = LittleFS.open(tmpPath, "w");
    if (!out) { in.close(); return false; }

    uint32_t cutoff = nowEpoch > V3_CHECKPOINT_RETENTION_SECONDS
        ? nowEpoch - V3_CHECKPOINT_RETENTION_SECONDS : 0;
    V3CheckpointRecord rec;
    bool ok = true;
    while (in.readBytes(reinterpret_cast<char*>(&rec), sizeof(rec)) == sizeof(rec)) {
        yield(); // Invalid/expired records must also service the SDK/watchdog.
        if (!v3CheckpointValid(rec)) continue;
        if (rec.epoch < cutoff) continue;
        if (out.write(reinterpret_cast<const uint8_t*>(&rec), sizeof(rec)) != sizeof(rec)) {
            ok = false;
            break;
        }
        yield();
    }
    in.close();
    out.close();

    if (!ok) { LittleFS.remove(tmpPath); return false; }
    const char* backupPath = "/energy_v3_cp.bak";
    if (LittleFS.exists(backupPath)) {
        LittleFS.remove(tmpPath);
        return false;
    }
    if (!LittleFS.rename(V3_CHECKPOINT_FILE, backupPath)) {
        LittleFS.remove(tmpPath);
        return false;
    }
    if (!LittleFS.rename(tmpPath, V3_CHECKPOINT_FILE)) {
        LittleFS.rename(backupPath, V3_CHECKPOINT_FILE);
        return false;
    }
    LittleFS.remove(backupPath);
    return true;
}

bool appendV3Checkpoint(uint32_t epoch) {
    if (!littleFsReady || !v3State.rawBaselineValid || epoch < 1600000000UL) {
        return false;
    }

    V3CheckpointRecord rec = {};
    rec.magic = V3_CHECKPOINT_MAGIC;
    rec.version = V3_DATA_VERSION;
    rec.epoch = epoch;
    rec.continuityGeneration = v3State.continuityGeneration;
    rec.internalImportWh = v3State.internalImportWh;
    rec.internalExportWh = v3State.internalExportWh;
    rec.z1ImportWh = v3State.z1ImportWh;
    rec.z2ImportWh = v3State.z2ImportWh;
    rec.z1ExportWh = v3State.z1ExportWh;
    rec.z2ExportWh = v3State.z2ExportWh;
    rec.checksum = v3CheckpointChecksum(rec);

    File f = LittleFS.open(V3_CHECKPOINT_FILE, "a");
    if (!f) return false;
    bool ok = f.write(reinterpret_cast<const uint8_t*>(&rec),
                      sizeof(rec)) == sizeof(rec);
    f.close();

    if (ok && rec.epoch >= 1600000000UL &&
        ((rec.epoch / V3_CHECKPOINT_SECONDS) % V3_CHECKPOINT_COMPACT_INTERVAL) == 0) {
        compactV3Checkpoints(rec.epoch);
    }
    return ok;
}

void v3PointFromState(V3AccumPoint& p) {
    p.epoch = v3State.lastEpoch;
    p.continuityGeneration = v3State.continuityGeneration;
    p.importWh = v3State.internalImportWh;
    p.exportWh = v3State.internalExportWh;
    p.z1ImportWh = v3State.z1ImportWh;
    p.z2ImportWh = v3State.z2ImportWh;
    p.z1ExportWh = v3State.z1ExportWh;
    p.z2ExportWh = v3State.z2ExportWh;
    p.sampleEpoch = p.epoch;
    p.mappingSource = 1;
}

void v3PointFromCheckpoint(const V3CheckpointRecord& r, V3AccumPoint& p) {
    p.epoch = r.epoch;
    p.continuityGeneration = r.continuityGeneration;
    p.importWh = r.internalImportWh;
    p.exportWh = r.internalExportWh;
    p.z1ImportWh = r.z1ImportWh;
    p.z2ImportWh = r.z2ImportWh;
    p.z1ExportWh = r.z1ExportWh;
    p.z2ExportWh = r.z2ExportWh;
    p.sampleEpoch = p.epoch;
    p.mappingSource = 1;
}

void v3ConsiderPoint(const V3AccumPoint& p, uint32_t target,
                    V3AccumPoint& before, bool& haveBefore,
                    V3AccumPoint& after, bool& haveAfter) {
    if (p.epoch <= target && (!haveBefore || p.epoch > before.epoch)) { before = p; haveBefore = true; }
    if (p.epoch >= target && (!haveAfter || p.epoch < after.epoch)) { after = p; haveAfter = true; }
}

uint64_t v3InterpolateU64(uint64_t a, uint64_t b,
                          uint32_t elapsed, uint32_t total) {
    if (total == 0 || a == b) return a;
    if (b >= a) {
        return a + ((b - a) * (uint64_t)elapsed) / (uint64_t)total;
    }
    return a - ((a - b) * (uint64_t)elapsed) / (uint64_t)total;
}

bool getV3AccumAtEpoch(uint32_t targetEpoch, V3AccumPoint& out) {
    if (!littleFsReady || targetEpoch < 1600000000UL) return false;
    V3AccumPoint before = {}, after = {};
    bool haveBefore = false, haveAfter = false;

    if (LittleFS.exists(V3_CHECKPOINT_FILE)) {
        File f = LittleFS.open(V3_CHECKPOINT_FILE, "r");
        if (f) {
            V3CheckpointRecord rec;
            while (f.readBytes(reinterpret_cast<char*>(&rec), sizeof(rec)) == sizeof(rec)) {
                yield();
                if (!v3CheckpointValid(rec)) continue;
                V3AccumPoint p; v3PointFromCheckpoint(rec, p);
                v3ConsiderPoint(p, targetEpoch, before, haveBefore, after, haveAfter);
            }
            f.close();
        }
    }

    if (v3State.rawBaselineValid && v3State.lastEpoch >= 1600000000UL) {
        V3AccumPoint current; v3PointFromState(current);
        v3ConsiderPoint(current, targetEpoch, before, haveBefore, after, haveAfter);
    }

    if (haveBefore && before.epoch == targetEpoch) { out = before; return true; }
    if (haveAfter && after.epoch == targetEpoch) { out = after; return true; }

    if (haveBefore && haveAfter && after.epoch > before.epoch) {
        if (before.continuityGeneration != after.continuityGeneration) return false;
        uint32_t gap = after.epoch - before.epoch;
        if (gap > V3_INTERPOLATION_MAX_GAP) return false;
        uint32_t elapsed = targetEpoch - before.epoch;
        out.epoch = targetEpoch; out.continuityGeneration = before.continuityGeneration;
        out.importWh = v3InterpolateU64(before.importWh, after.importWh, elapsed, gap);
        out.exportWh = v3InterpolateU64(before.exportWh, after.exportWh, elapsed, gap);
        out.z1ImportWh = v3InterpolateU64(before.z1ImportWh, after.z1ImportWh, elapsed, gap);
        out.z2ImportWh = v3InterpolateU64(before.z2ImportWh, after.z2ImportWh, elapsed, gap);
        out.z1ExportWh = v3InterpolateU64(before.z1ExportWh, after.z1ExportWh, elapsed, gap);
        out.z2ExportWh = v3InterpolateU64(before.z2ExportWh, after.z2ExportWh, elapsed, gap);
        out.sampleEpoch = targetEpoch;
        out.mappingSource = 2;
        return true;
    }

    if (haveBefore && targetEpoch - before.epoch <= V3_NEAR_POINT_MAX_GAP) { out = before; out.epoch = targetEpoch; out.mappingSource = 3; return true; }
    if (haveAfter && after.epoch - targetEpoch <= V3_NEAR_POINT_MAX_GAP) { out = after; out.epoch = targetEpoch; out.mappingSource = 3; return true; }
    return false;
}


bool parseV3DateEpoch(const char* dateText, uint16_t minuteOfDay,
                      uint32_t& epochOut) {
    if (!dateText || strlen(dateText) != 10 || minuteOfDay >= 24U * 60U) {
        return false;
    }

    for (uint8_t i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) { if (dateText[i] != '-') return false; }
        else if (dateText[i] < '0' || dateText[i] > '9') return false;
    }
    int year = 0, month = 0, day = 0;
    if (sscanf(dateText, "%d-%d-%d", &year, &month, &day) != 3) return false;
    if (year < 2024 || year > 2099 ||
        month < 1 || month > 12 || day < 1 || day > 31) {
        return false;
    }

    struct tm t = {};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = minuteOfDay / 60U;
    t.tm_min = minuteOfDay % 60U;
    t.tm_sec = 0;
    t.tm_isdst = -1;

    time_t value = mktime(&t);
    if (value < 1600000000) return false;

    struct tm verify;
    if (!localtime_r(&value, &verify)) return false;
    if (verify.tm_year != year - 1900 ||
        verify.tm_mon != month - 1 ||
        verify.tm_mday != day ||
        verify.tm_hour != (int)(minuteOfDay / 60U) || verify.tm_min != (int)(minuteOfDay % 60U)) {
        return false;
    }

    epochOut = (uint32_t)value;
    return true;
}

void formatV3Date(uint32_t epoch, char* out, size_t outSize) {
    if (!out || outSize < 11) return;
    struct tm t;
    time_t v = (time_t)epoch;
    if (!localtime_r(&v, &t)) {
        strncpy(out, "1970-01-01", outSize);
        out[outSize - 1] = '\0';
        return;
    }
    snprintf(out, outSize, "%04d-%02d-%02d",
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
}

void allocateV3DeltaToZones(uint64_t deltaWh,
                            uint32_t startEpoch,
                            uint32_t endEpoch,
                            bool isImport) {
    if (deltaWh == 0) return;

    if (endEpoch <= startEpoch || startEpoch < 1600000000UL) {
        bool z2 = isZ2AtEpoch((time_t)endEpoch);
        uint64_t* target = nullptr;
        if (isImport) target = z2 ? &v3State.z2ImportWh : &v3State.z1ImportWh;
        else target = z2 ? &v3State.z2ExportWh : &v3State.z1ExportWh;
        *target += deltaWh;
        return;
    }

    uint32_t totalSeconds = endEpoch - startEpoch;
    uint32_t cursor = startEpoch;
    uint64_t allocated = 0;
    uint8_t slicesSinceYield = 0;

    while (cursor < endEpoch) {
        uint32_t next = cursor + 60UL;
        if (next > endEpoch || next < cursor) next = endEpoch;

        uint32_t span = next - cursor;
        uint64_t part = (next == endEpoch)
            ? (deltaWh - allocated)
            : (deltaWh * (uint64_t)span) / (uint64_t)totalSeconds;

        time_t midpoint = (time_t)(cursor + span / 2UL);
        bool z2 = isZ2AtEpoch(midpoint);

        uint64_t* target = nullptr;
        if (isImport) target = z2 ? &v3State.z2ImportWh : &v3State.z1ImportWh;
        else target = z2 ? &v3State.z2ExportWh : &v3State.z1ExportWh;

        *target += part;
        allocated += part;
        cursor = next;

        // A +60s cursor may NEVER hit the old bitmask condition (e.g. odd epochs).
        if (++slicesSinceYield >= 32) {
            slicesSinceYield = 0;
            yield();
        }
    }
}

uint32_t v3UtilityAnchorEpoch(const V3UtilityReading& rec) {
    return rec.reserved2[0] == V3_READING_INTERMEDIATE &&
        (rec.reserved2[1] & V3_EK_CORRECTION)
        ? rec.enteredEpoch : rec.measuredEpoch;
}

// Preserve full physical anchors independently of the unchanged V1 reading file.
// Both import and export are required; old history is never assumed to export 0.
bool loadV3UtilityPoint(const V3UtilityReading& rec, V3AccumPoint& point) {
    if (!littleFsReady || !LittleFS.exists(V3_UTILITY_POINT_FILE)) return false;
    File f = LittleFS.open(V3_UTILITY_POINT_FILE, "r");
    if (!f) return false;
    V3UtilityPointRecord stored;
    bool found = false;
    const uint32_t epoch = v3UtilityAnchorEpoch(rec);
    while (f.readBytes(reinterpret_cast<char*>(&stored), sizeof(stored)) == sizeof(stored)) {
        yield();
        if (stored.magic != V3_UTILITY_POINT_MAGIC || stored.version != V3_DATA_VERSION ||
            stored.utilityId != rec.id || stored.anchorEpoch != epoch ||
            stored.anchorImportWh != rec.anchorImportWh ||
            stored.anchorZ1ImportWh != rec.anchorZ1ImportWh ||
            stored.anchorZ2ImportWh != rec.anchorZ2ImportWh ||
            stored.point.mappingSource < 1 || stored.point.mappingSource > 4 ||
            stored.checksum != v3ChecksumBytes(&stored, offsetof(V3UtilityPointRecord, checksum))) continue;
        point = stored.point;
        found = true;
    }
    f.close();
    return found;
}

bool getV3UtilityPoint(const V3UtilityReading& rec, V3AccumPoint& point) {
    // v4.00: a dated "Καταχώρηση Μέτρησης ΔΕΗ" is comparison-only.
    // Only a current "Καταχώρηση Ένδειξης Μετρητή ΔΕΗ" may be tied
    // to the internal Import/Export counters.
    if (rec.reserved2[0] != V3_READING_INTERMEDIATE) return false;
    if (loadV3UtilityPoint(rec, point)) return true;
    if ((rec.reserved2[1] & V3_EK_CORRECTION) && v3State.rawBaselineValid &&
        (v3State.lastEpoch > rec.enteredEpoch ? v3State.lastEpoch - rec.enteredEpoch : rec.enteredEpoch - v3State.lastEpoch) <= 5UL &&
        rec.anchorImportWh == v3State.internalImportWh &&
        rec.anchorZ1ImportWh == v3State.z1ImportWh && rec.anchorZ2ImportWh == v3State.z2ImportWh) {
        v3PointFromState(point); // exact counters read immediately before saving E.K.
        return true;
    }
    if (rec.minuteOfDay != V3_TIME_WINDOW) {
        return getV3AccumAtEpoch(v3UtilityAnchorEpoch(rec), point);
    }
    char date[16] = {};
    formatV3Date(rec.measuredEpoch, date, sizeof(date));
    uint32_t start = 0, end = 0;
    V3AccumPoint a = {}, b = {};
    if (!parseV3DateEpoch(date, V3_WINDOW_MINUTE_START, start) ||
        !parseV3DateEpoch(date, V3_WINDOW_MINUTE_END, end) ||
        !getV3AccumAtEpoch(start, a) || !getV3AccumAtEpoch(end, b) ||
        a.continuityGeneration != b.continuityGeneration) return false;
    point = {};
    point.epoch = rec.measuredEpoch;
    point.continuityGeneration = a.continuityGeneration;
    point.importWh = (a.importWh + b.importWh) / 2ULL;
    point.exportWh = (a.exportWh + b.exportWh) / 2ULL;
    point.z1ImportWh = (a.z1ImportWh + b.z1ImportWh) / 2ULL;
    point.z2ImportWh = (a.z2ImportWh + b.z2ImportWh) / 2ULL;
    point.z1ExportWh = (a.z1ExportWh + b.z1ExportWh) / 2ULL;
    point.z2ExportWh = (a.z2ExportWh + b.z2ExportWh) / 2ULL;
    point.sampleEpoch = rec.measuredEpoch;
    point.mappingSource = 4;
    return true;
}

bool saveV3UtilityPoint(const V3UtilityReading& rec, const V3AccumPoint& point) {
    if (!littleFsReady) return false;
    V3AccumPoint existing = {};
    if (loadV3UtilityPoint(rec, existing)) return true;
    V3UtilityPointRecord stored = {};
    stored.magic = V3_UTILITY_POINT_MAGIC;
    stored.version = V3_DATA_VERSION;
    stored.utilityId = rec.id;
    stored.anchorEpoch = v3UtilityAnchorEpoch(rec);
    stored.anchorImportWh = rec.anchorImportWh;
    stored.anchorZ1ImportWh = rec.anchorZ1ImportWh;
    stored.anchorZ2ImportWh = rec.anchorZ2ImportWh;
    stored.point = point;
    stored.checksum = v3ChecksumBytes(&stored, offsetof(V3UtilityPointRecord, checksum));
    File f = LittleFS.open(V3_UTILITY_POINT_FILE, "a");
    if (!f) return false;
    const bool ok = f.write(reinterpret_cast<const uint8_t*>(&stored), sizeof(stored)) == sizeof(stored);
    f.close();
    return ok;
}

void refreshV3LatestUtility() {
    v3LatestUtilityValid = v3EstimateUtilityValid = false;
    v3EstimatePointValid = false;
    v3EstimatePoint = {};
    v3LatestIntermediateId = 0;
    memset(&v3LatestUtility, 0, sizeof(v3LatestUtility));
    memset(&v3EstimateUtility, 0, sizeof(v3EstimateUtility));
    if (!littleFsReady || !LittleFS.exists(V3_UTILITY_FILE)) return;
    File f = LittleFS.open(V3_UTILITY_FILE, "r");
    if (!f) return;
    V3UtilityReading rec;
    while (f.readBytes(reinterpret_cast<char*>(&rec), sizeof(rec)) == sizeof(rec)) {
        yield();
        if (!v3UtilityValid(rec) || !rec.active || rec.id == v3UtilityExcludedId) continue;

        const bool indication = rec.reserved2[0] == V3_READING_INTERMEDIATE;
        if (indication && rec.id > v3LatestIntermediateId) {
            v3LatestIntermediateId = rec.id;
        }

        // Latest DEH measurement is selected by the date/time the utility
        // actually measured, not by the day the user later entered it.
        if (!indication && (!v3LatestUtilityValid ||
            rec.measuredEpoch > v3LatestUtility.measuredEpoch ||
            (rec.measuredEpoch == v3LatestUtility.measuredEpoch && rec.id > v3LatestUtility.id))) {
            v3LatestUtility = rec;
            v3LatestUtilityValid = true;
        }

        // Estimate anchor: ONLY a current indication that was physically tied
        // to the ESP internal counters. A DEH measurement can never replace it.
        if (!indication || !(rec.reserved2[1] & V3_EK_CORRECTION)) continue;
        const uint32_t recAnchorEpoch = v3UtilityAnchorEpoch(rec);
        const uint32_t currentAnchorEpoch = v3EstimateUtilityValid
            ? v3UtilityAnchorEpoch(v3EstimateUtility) : 0;
        if (!v3EstimateUtilityValid ||
            recAnchorEpoch > currentAnchorEpoch ||
            (recAnchorEpoch == currentAnchorEpoch && rec.id > v3EstimateUtility.id)) {
            v3EstimateUtility = rec;
            v3EstimateUtilityValid = true;
        }
    }
    f.close();

    if (v3EstimateUtilityValid) {
        v3EstimatePointValid = getV3UtilityPoint(v3EstimateUtility, v3EstimatePoint);
        if (v3EstimatePointValid) saveV3UtilityPoint(v3EstimateUtility, v3EstimatePoint);
    }
}


int64_t v3SignedDeltaWh(uint64_t current, uint64_t anchor) {
    if (current >= anchor) {
        uint64_t d = current - anchor;
        return d > (uint64_t)INT64_MAX ? INT64_MAX : (int64_t)d;
    }
    uint64_t d = anchor - current;
    return d > (uint64_t)INT64_MAX ? INT64_MIN : -(int64_t)d;
}

void refreshV3EstimateView() {
    tariffZReference = 0.0f;
    tariffZNow = 0.0f;
    tariffZ1Reference = 0.0f;
    tariffZ1Now = 0.0f;
    tariffZ2Reference = 0.0f;
    tariffZ2Now = 0.0f;

    // "ΔΕΗ τελευταία": the newest user-entered DEH measurement by its
    // measurement date/time. It is a comparison value only.
    if (v3LatestUtilityValid) {
        if (v3LatestUtility.mode == V3_MODE_DUAL) {
            tariffZ1Reference = (double)v3LatestUtility.officialZ1Wh / 1000.0;
            tariffZ2Reference = (double)v3LatestUtility.officialZ2Wh / 1000.0;
            tariffZReference = tariffZ1Reference + tariffZ2Reference;
        } else {
            tariffZReference = (double)v3LatestUtility.officialZWh / 1000.0;
        }
    }

    // "Εκτίμηση τώρα": latest current indication + signed internal
    // Import-Export movement from that exact physical anchor until now.
    if (!v3EstimateUtilityValid) return;
    const V3UtilityReading& anchor = v3EstimateUtility;

    if (anchor.mode == V3_MODE_DUAL) {
        tariffZ1Now = (double)anchor.officialZ1Wh / 1000.0;
        tariffZ2Now = (double)anchor.officialZ2Wh / 1000.0;
        if (v3EstimatePointValid) {
            tariffZ1Now +=
                (double)(v3SignedDeltaWh(v3State.z1ImportWh, v3EstimatePoint.z1ImportWh) -
                       v3SignedDeltaWh(v3State.z1ExportWh, v3EstimatePoint.z1ExportWh)) / 1000.0;
            tariffZ2Now +=
                (double)(v3SignedDeltaWh(v3State.z2ImportWh, v3EstimatePoint.z2ImportWh) -
                       v3SignedDeltaWh(v3State.z2ExportWh, v3EstimatePoint.z2ExportWh)) / 1000.0;
        }
        tariffZNow = tariffZ1Now + tariffZ2Now;
    } else {
        tariffZNow = (double)anchor.officialZWh / 1000.0;
        if (v3EstimatePointValid) {
            tariffZNow +=
                (double)(v3SignedDeltaWh(v3State.internalImportWh, v3EstimatePoint.importWh) -
                       v3SignedDeltaWh(v3State.internalExportWh, v3EstimatePoint.exportWh)) / 1000.0;
        }
    }
}

// V5: bounded daily storage, two checksum-protected current-state banks.
// Energy order: Z1 import/export, Z2 night import/export, Z2 midday import/export.
struct V5Day {
    uint32_t magic, day, firstEpoch, lastEpoch, flags, coveredSeconds;
    uint64_t wh[6];
    uint32_t checksum, reserved;
};
struct V5State {
    V5Day today;
    uint64_t rawImportWh, rawExportWh;
    uint32_t startEpoch, sequence, checksum, reserved;
};
static_assert(sizeof(V5Day) == 80, "V5 daily layout changed");
static_assert(sizeof(V5State) == 112, "V5 state layout changed");
const uint32_t V5_MAGIC = 0x35445945UL;
const uint32_t V5_DAYS = 730;
const char* V5_DAY_FILE = "/v5_days.bin";
V5State v5History = {};
bool v5StorageFailed = false;
bool v5RingFailed = false;
uint32_t v5MqttMinHeap = 0, v5MqttMinBlock = 0;
uint32_t v5HistoryToken = 0, v5HistoryOffset = 0, v5HistoryCursor = 0;
uint8_t v5HistoryMode = 0, v5HistoryRows = 0;
bool v5HistorySending = false, v5HistoryBegun = false;
unsigned long v5HistoryLastSend = 0;
char V5_HISTORY_TOPIC[MQTT_TOPIC_BASE_BYTES + sizeof("/admin/history") - 1] = {};

uint32_t v5Checksum(const void* data, size_t size) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint32_t hash = 2166136261UL;
    for (size_t i = 0; i < size; ++i) hash = (hash ^ bytes[i]) * 16777619UL;
    return hash;
}

uint32_t v5LocalDay(time_t epoch, uint8_t* band = nullptr) {
    struct tm t;
    if (!localtime_r(&epoch, &t)) return 0;
    int year = t.tm_year + 1900, month = t.tm_mon + 1;
    int minute = t.tm_hour * 60 + t.tm_min;
    bool winter = month >= 11 || month <= 3;
    if (band) *band = minute >= 120 && minute < (winter ? 300 : 240) ? 1 :
        (minute >= (winter ? 720 : 660) && minute < 900 ? 2 : 0);
    year -= month <= 2;
    int era = year / 400;
    unsigned yoe = year - era * 400;
    unsigned doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + t.tm_mday - 1;
    return era * 146097 + yoe * 365 + yoe / 4 - yoe / 100 + doy - 719468;
}

bool v5DayValid(const V5Day& d) {
    return d.magic == V5_MAGIC && d.firstEpoch >= 1600000000UL &&
        d.lastEpoch >= d.firstEpoch && d.day == v5LocalDay(d.firstEpoch) &&
        d.checksum == v5Checksum(&d, offsetof(V5Day, checksum));
}

bool v5StateValid(const V5State& s) {
    return v5DayValid(s.today) && s.startEpoch >= 1600000000UL &&
        s.checksum == v5Checksum(&s, offsetof(V5State, checksum));
}

void loadV5History() {
    if (!littleFsReady) return;
    for (uint8_t bank = 0; bank < 2; ++bank) {
        File f = LittleFS.open(bank ? "/v5_state1.bin" : "/v5_state0.bin", "r");
        if (!f) continue;
        V5State s = {};
        bool ok = f.size() == sizeof(s) && f.readBytes(reinterpret_cast<char*>(&s), sizeof(s)) == sizeof(s);
        f.close();
        if (ok && v5StateValid(s) && (!v5History.sequence ||
            (int32_t)(s.sequence - v5History.sequence) > 0)) v5History = s;
    }
}

bool saveV5History() {
    if (!littleFsReady) { v5StorageFailed = true; return false; }
    FSInfo info;
    if (!LittleFS.info(info) || info.totalBytes < info.usedBytes + 32768UL) {
        v5StorageFailed = true; return false;
    }
    v5History.today.checksum = v5Checksum(&v5History.today, offsetof(V5Day, checksum));
    ++v5History.sequence;
    v5History.checksum = v5Checksum(&v5History, offsetof(V5State, checksum));
    File f = LittleFS.open(v5History.sequence & 1 ? "/v5_state1.bin" : "/v5_state0.bin", "w");
    bool ok = f && f.write(reinterpret_cast<const uint8_t*>(&v5History), sizeof(v5History)) == sizeof(v5History);
    if (f) f.close();
    // Verify the new bank; the previous valid bank is never removed.
    if (ok) {
        f = LittleFS.open(v5History.sequence & 1 ? "/v5_state1.bin" : "/v5_state0.bin", "r");
        V5State check = {};
        ok = f && f.readBytes(reinterpret_cast<char*>(&check), sizeof(check)) == sizeof(check) &&
            v5StateValid(check) && check.sequence == v5History.sequence;
        if (f) f.close();
    }
    if (!ok) --v5History.sequence; // Retry the failed bank, preserving the last valid one.
    v5StorageFailed = !ok || v5RingFailed;
    return ok;
}

bool writeV5Day() {
    if (!littleFsReady) return false;
    FSInfo available;
    if (!LittleFS.info(available) || available.totalBytes < available.usedBytes + 400000UL) return false;
    File f = LittleFS.open(V5_DAY_FILE, "r+");
    if (!f) {
        FSInfo info;
        if (!LittleFS.info(info) || info.totalBytes < info.usedBytes + 400000UL + V5_DAYS * sizeof(V5Day)) return false;
        f = LittleFS.open(V5_DAY_FILE, "w+");
        if (!f) return false;
    }
    // Fill only as far as needed. File size is strictly bounded by 730 records.
    size_t position = (v5History.today.day % V5_DAYS) * sizeof(V5Day);
    V5Day zero = {};
    f.seek(f.size(), SeekSet);
    while (f.size() < position + sizeof(V5Day)) {
        if (f.write(reinterpret_cast<const uint8_t*>(&zero), sizeof(zero)) != sizeof(zero)) { f.close(); return false; }
        yield();
    }
    v5History.today.checksum = v5Checksum(&v5History.today, offsetof(V5Day, checksum));
    bool ok = f.seek(position, SeekSet) &&
        f.write(reinterpret_cast<const uint8_t*>(&v5History.today), sizeof(V5Day)) == sizeof(V5Day);
    f.close();
    if (ok) {
        f = LittleFS.open(V5_DAY_FILE, "r");
        V5Day check = {};
        ok = f && f.seek(position, SeekSet) &&
            f.readBytes(reinterpret_cast<char*>(&check), sizeof(check)) == sizeof(check) && v5DayValid(check);
        if (f) f.close();
    }
    return ok;
}

void startV5Day(uint32_t epoch, uint32_t flags) {
    memset(&v5History.today, 0, sizeof(v5History.today));
    v5History.today.magic = V5_MAGIC;
    v5History.today.day = v5LocalDay(epoch);
    v5History.today.firstEpoch = v5History.today.lastEpoch = epoch;
    v5History.today.flags = flags;
}

void updateV5History(uint64_t importWh, uint64_t exportWh, uint32_t epoch) {
    if (epoch < 1600000000UL) return;
    if (!v5History.startEpoch) {
        startV5Day(epoch, 1); // First upgrade day is partial; never backfill old energy.
        v5History.startEpoch = epoch;
        v5History.rawImportWh = importWh; v5History.rawExportWh = exportWh;
        saveV5History(); return;
    }
    uint32_t previous = v5History.today.lastEpoch;
    if (epoch <= previous) return; // Clock correction: wait; retain the physical baseline.
    bool reset = importWh < v5History.rawImportWh || exportWh < v5History.rawExportWh;
    uint64_t di = reset ? 0 : importWh - v5History.rawImportWh;
    uint64_t de = reset ? 0 : exportWh - v5History.rawExportWh;
    uint32_t span = epoch - previous;
    bool rollover = false;
    if (span > 7200 || reset) {
        // No trustworthy time profile for a long outage. Attribute recovered
        // energy to the return day, explicitly estimated; do not invent a curve.
        if (v5LocalDay(epoch) != v5History.today.day) {
            v5History.today.flags |= 2;
            v5RingFailed = !writeV5Day();
            if (v5RingFailed) v5StorageFailed = true;
            startV5Day(epoch, 1 | 2); rollover = true;
        }
        uint8_t band = 0; v5LocalDay(epoch, &band);
        v5History.today.wh[band * 2] += di; v5History.today.wh[band * 2 + 1] += de;
        v5History.today.flags |= reset ? 4 : 2;
    } else {
        uint32_t cursor = previous;
        uint64_t ai = 0, ae = 0;
        while (cursor < epoch) {
            uint32_t next = ((cursor / 60) + 1) * 60;
            if (next > epoch) next = epoch;
            uint8_t band = 0;
            uint32_t day = v5LocalDay(cursor, &band);
            if (day != v5History.today.day) {
                v5RingFailed = !writeV5Day();
            if (v5RingFailed) v5StorageFailed = true;
                startV5Day(cursor, span > 90 ? 2 : 0); rollover = true;
            }
            uint64_t ci = di * (next - previous) / span;
            uint64_t ce = de * (next - previous) / span;
            v5History.today.wh[band * 2] += ci - ai;
            v5History.today.wh[band * 2 + 1] += ce - ae;
            if (span <= 90) v5History.today.coveredSeconds += next - cursor;
            else v5History.today.flags |= 2;
            v5History.today.lastEpoch = next;
            ai = ci; ae = ce; cursor = next;
            yield();
        }
    }
    v5History.today.lastEpoch = epoch;
    v5History.rawImportWh = importWh; v5History.rawExportWh = exportWh;
    if (rollover || reset || previous / 1800 != epoch / 1800) saveV5History();
}

bool stageV5History(const String& text) {
    unsigned long token = 0, offset = 0; unsigned mode = 0; char extra;
    if (sscanf(text.c_str(), "history_get|%lu|%u|%lu%c", &token, &mode, &offset, &extra) != 3 ||
        !token || mode > 1 || offset > 10000) return false;
    v5HistoryToken = token; v5HistoryOffset = offset;
    v5HistoryMode = mode;
    return true;
}

void beginV5HistoryPage() {
    v5HistoryCursor = v5HistoryOffset;
    v5HistoryRows = 0; v5HistoryBegun = false; v5HistorySending = true;
    if (v5HistoryMode == 0 && v5HistoryOffset == 0 && littleFsReady) {
        File f = LittleFS.open(V3_CHECKPOINT_FILE, "r");
        V3CheckpointRecord rec;
        uint32_t nowEpoch = (uint32_t)time(nullptr);
        uint32_t cutoff = nowEpoch > 86400 ? nowEpoch - 86400 : 0;
        if (cutoff < v5History.startEpoch) cutoff = v5History.startEpoch;
        while (f && f.readBytes(reinterpret_cast<char*>(&rec), sizeof(rec)) == sizeof(rec)) {
            if (v3CheckpointValid(rec) && rec.epoch >= cutoff) break;
            ++v5HistoryCursor; yield();
        }
        if (f) f.close();
    }
}

void serviceV5History(unsigned long now) {
    if (mqttClient.connected()) {
        uint32_t heap = ESP.getFreeHeap(), block = ESP.getMaxFreeBlockSize();
        if (!v5MqttMinHeap || heap < v5MqttMinHeap) v5MqttMinHeap = heap;
        if (!v5MqttMinBlock || block < v5MqttMinBlock) v5MqttMinBlock = block;
    }
    if (!v5HistorySending) return;
    if (!mqttClient.connected()) { v5HistorySending = false; return; }
    if (now - v5HistoryLastSend < 75) return;
    v5HistoryLastSend = now;
    char message[256]; int n = 0;
    if (!v5HistoryBegun) {
        FSInfo info = {}; bool mounted = littleFsReady && LittleFS.info(info);
        n = snprintf(message, sizeof(message), "B|%lu|%u|%lu|%lu|%lu|%lu|%lu|%lu|%lu|%u|%lu",
            (unsigned long)v5HistoryToken, (unsigned)v5HistoryMode,
            (unsigned long)v5History.startEpoch, (unsigned long)info.totalBytes,
            (unsigned long)info.usedBytes, (unsigned long)(info.totalBytes - info.usedBytes),
            (unsigned long)ESP.getFreeHeap(), (unsigned long)ESP.getMaxFreeBlockSize(),
            (unsigned long)v5MqttMinHeap, (unsigned)(!mounted || v5StorageFailed), (unsigned long)time(nullptr));
        if (n > 0 && n < (int)sizeof(message) && mqttClient.publish(V5_HISTORY_TOPIC, message, false)) v5HistoryBegun = true;
        else v5HistorySending = false;
        return;
    }
    bool end = v5HistoryRows >= 16 || !v5History.startEpoch;
    bool more = false;
    if (!end && v5HistoryMode == 1) {
        uint32_t today = v5LocalDay(time(nullptr));
        while (v5HistoryCursor < 30) {
            uint32_t day = today - 29 + v5HistoryCursor++;
            V5Day d = {};
            if (day == v5History.today.day) d = v5History.today;
            else if (littleFsReady) {
                File f = LittleFS.open(V5_DAY_FILE, "r");
                if (f && f.seek((day % V5_DAYS) * sizeof(d), SeekSet)) f.readBytes(reinterpret_cast<char*>(&d), sizeof(d));
                if (f) f.close();
                if (!v5DayValid(d)) continue;
            }
            if (d.magic != V5_MAGIC || d.day != day) continue;
            n = snprintf(message, sizeof(message), "D|%lu|%lu|%lu|%lu|%lu|%lu|%llu|%llu|%llu|%llu|%llu|%llu",
                (unsigned long)v5HistoryToken, (unsigned long)d.day,
                (unsigned long)d.firstEpoch, (unsigned long)d.lastEpoch,
                (unsigned long)(d.flags | (day == today ? 16 : 0) | (v5StorageFailed ? 8 : 0)),
                (unsigned long)d.coveredSeconds,
                (unsigned long long)d.wh[0], (unsigned long long)d.wh[1],
                (unsigned long long)d.wh[2], (unsigned long long)d.wh[3],
                (unsigned long long)d.wh[4], (unsigned long long)d.wh[5]);
            break;
        }
        if (!n) end = true;
        more = v5HistoryCursor < 30;
    } else if (!end) {
        V3CheckpointRecord rec = {};
        File f = littleFsReady ? LittleFS.open(V3_CHECKPOINT_FILE, "r") : File();
        size_t count = f ? f.size() / sizeof(rec) : 0;
        bool found = false;
        while (f && v5HistoryCursor < count) {
            f.seek(v5HistoryCursor++ * sizeof(rec), SeekSet);
            if (f.readBytes(reinterpret_cast<char*>(&rec), sizeof(rec)) == sizeof(rec) &&
                v3CheckpointValid(rec) && rec.epoch >= v5History.startEpoch) { found = true; break; }
            yield();
        }
        if (f) f.close();
        if (!found && v5HistoryCursor == count && v3State.lastEpoch >= v5History.startEpoch && v5History.startEpoch) {
            ++v5HistoryCursor;
            rec.epoch = v3State.lastEpoch; rec.continuityGeneration = v3State.continuityGeneration;
            rec.internalImportWh = v3State.internalImportWh; rec.internalExportWh = v3State.internalExportWh;
            found = true;
        }
        if (found) n = snprintf(message, sizeof(message), "P|%lu|%lu|%lu|%llu|%llu",
            (unsigned long)v5HistoryToken, (unsigned long)rec.epoch,
            (unsigned long)rec.continuityGeneration,
            (unsigned long long)rec.internalImportWh, (unsigned long long)rec.internalExportWh);
        else end = true;
        more = v5HistoryCursor <= count;
    }
    if (end) {
        if (v5HistoryMode == 1) more = v5HistoryCursor < 30;
        else {
            File f = littleFsReady ? LittleFS.open(V3_CHECKPOINT_FILE, "r") : File();
            more = f && v5HistoryCursor <= f.size() / sizeof(V3CheckpointRecord);
            if (f) f.close();
        }
        snprintf(message, sizeof(message), "E|%lu|%ld|%u", (unsigned long)v5HistoryToken,
            more ? (long)v5HistoryCursor : -1L, (unsigned)v5HistoryRows);
        mqttClient.publish(V5_HISTORY_TOPIC, message, false);
        v5HistorySending = false;
    } else if (n > 0 && n < (int)sizeof(message) && mqttClient.publish(V5_HISTORY_TOPIC, message, false)) ++v5HistoryRows;
    else v5HistorySending = false;
}

void updateV3Accounting(uint64_t rawImportWh, uint64_t rawExportWh) {
    bootCheckpoint(CRASH_STAGE_BOOT_ACCOUNTING);
    v3RawImportWh = rawImportWh;
    v3RawExportWh = rawExportWh;

    if (!isDualZoneTimeValid()) {
        // Keep raw live values visible, but do not create timestamped accounting
        // until a valid local clock exists.
        return;
    }

    uint32_t nowEpoch = (uint32_t)time(nullptr);
    if (nowEpoch < 1600000000UL) return;
    updateV5History(rawImportWh, rawExportWh, nowEpoch);

    if (!v3State.rawBaselineValid) {
        v3State.magic = V3_STATE_MAGIC;
        v3State.version = V3_DATA_VERSION;
        v3State.lastRawImportWh = rawImportWh;
        v3State.lastRawExportWh = rawExportWh;
        v3State.lastEpoch = nowEpoch;
        v3State.continuityGeneration = 1;
        v3State.lastCheckpointSlot = nowEpoch / V3_CHECKPOINT_SECONDS;
        v3State.rawBaselineValid = 1;
        saveV3State();
        appendV3Checkpoint(nowEpoch);
        refreshV3EstimateView();
        return;
    }

    if (rawImportWh < v3State.lastRawImportWh ||
        rawExportWh < v3State.lastRawExportWh) {
        // Physical counter reset/wrap: start a new continuity generation and
        // never turn the backward jump into negative consumption.
        v3State.continuityGeneration++;
        v3State.lastRawImportWh = rawImportWh;
        v3State.lastRawExportWh = rawExportWh;
        v3State.lastEpoch = nowEpoch;
        v3State.lastCheckpointSlot = nowEpoch / V3_CHECKPOINT_SECONDS;
        saveV3State();
        appendV3Checkpoint(nowEpoch);
        refreshV3EstimateView();
        return;
    }

    uint64_t deltaImportWh = rawImportWh - v3State.lastRawImportWh;
    uint64_t deltaExportWh = rawExportWh - v3State.lastRawExportWh;
    uint32_t previousEpoch = v3State.lastEpoch;

    v3State.internalImportWh += deltaImportWh;
    v3State.internalExportWh += deltaExportWh;

    // v3 keeps Z1/Z2 accumulators continuously, independent of UI tariff mode.
    allocateV3DeltaToZones(deltaImportWh, previousEpoch, nowEpoch, true);
    allocateV3DeltaToZones(deltaExportWh, previousEpoch, nowEpoch, false);

    v3State.lastRawImportWh = rawImportWh;
    v3State.lastRawExportWh = rawExportWh;
    v3State.lastEpoch = nowEpoch;

    uint32_t slot = nowEpoch / V3_CHECKPOINT_SECONDS;
    if (slot != v3State.lastCheckpointSlot || v5History.startEpoch == nowEpoch) {
        v3State.lastCheckpointSlot = slot;
        saveV3State();
        appendV3Checkpoint(nowEpoch);
    }

    refreshV3EstimateView();
}

uint64_t v3AbsDiffU64(uint64_t a, uint64_t b) { return a >= b ? a - b : b - a; }

void v3SetUtilityAnchor(V3UtilityReading& rec, const V3AccumPoint& p) {
    rec.anchorImportWh = p.importWh;
    rec.anchorZ1ImportWh = p.z1ImportWh;
    rec.anchorZ2ImportWh = p.z2ImportWh;
    rec.mappingValid = 1;
}

bool buildV3UtilityRecord(V3UtilityReading& rec,
                          uint32_t id, uint8_t readingKind,
                          bool dual, const char* dateText,
                          bool exactTime, uint16_t minuteOfDay,
                          double z, double z1, double z2) {
    memset(&rec, 0, sizeof(rec));
    uint32_t measuredEpoch = 0;
    uint16_t effectiveMinute = exactTime ? minuteOfDay : V3_WINDOW_MINUTE_MID;
    if (!parseV3DateEpoch(dateText, effectiveMinute, measuredEpoch)) return false;

    rec.magic = V3_UTILITY_MAGIC; rec.version = V3_DATA_VERSION; rec.active = 1;
    rec.mode = dual ? V3_MODE_DUAL : V3_MODE_MONO;
    rec.reserved2[0] = readingKind == V3_READING_INTERMEDIATE
        ? V3_READING_INTERMEDIATE : V3_READING_FINAL;
    rec.id = id; rec.measuredEpoch = measuredEpoch;
    rec.enteredEpoch = isDualZoneTimeValid() ? (uint32_t)time(nullptr) : 0;
    rec.minuteOfDay = exactTime ? minuteOfDay : V3_TIME_WINDOW;

    const bool ek = rec.reserved2[0] == V3_READING_INTERMEDIATE;
    // v4.01: official DEH measurements are supplied by the utility in whole
    // kWh. Keep current indications at 0.01 kWh precision, but reject a
    // comparison-only DEH measurement that contains fractional kWh.
    if (!ek) {
        if (dual) {
            if (fabs(z1 - round(z1)) > 0.000001 ||
                fabs(z2 - round(z2)) > 0.000001) return false;
        } else if (fabs(z - round(z)) > 0.000001) {
            return false;
        }
    }

    if (dual) {
        rec.officialZ1Wh = (uint64_t)llround((double)z1 * 1000.0);
        rec.officialZ2Wh = (uint64_t)llround((double)z2 * 1000.0);
    } else {
        rec.officialZWh = (uint64_t)llround((double)z * 1000.0);
    }

    // v4.00 DEH measurements are stored for history/reference only. Their
    // historical date must never be mapped back onto ESP counters.
    if (!ek) {
        rec.mappingValid = 0;
        rec.checksum = v3UtilityChecksum(rec);
        return true;
    }

    const bool nearNow = exactTime && rec.enteredEpoch >= 1600000000UL &&
        v3AbsDiffU64(measuredEpoch, rec.enteredEpoch) <= V3_NEAR_POINT_MAX_GAP &&
        v3State.rawBaselineValid &&
        v3AbsDiffU64(v3State.lastEpoch, rec.enteredEpoch) <= 5UL;
    if (nearNow) {
        rec.reserved2[1] = V3_EK_EVALUATED;
        rec.reserved2[2] = V3_EK_NET_SNAPSHOT;
        V3AccumPoint nowPoint = {};
        v3PointFromState(nowPoint);
        v3SetUtilityAnchor(rec, nowPoint);
        rec.reserved2[1] |= V3_EK_CORRECTION;
        // Freeze signed net consumption BEFORE applying this E.K. correction.
        if (v3EstimateUtilityValid && v3EstimatePointValid &&
            v3EstimateUtility.mode == rec.mode &&
            v3EstimatePoint.continuityGeneration == nowPoint.continuityGeneration &&
            v3UtilityAnchorEpoch(v3EstimateUtility) <= rec.enteredEpoch) {
            const V3UtilityReading& base = v3EstimateUtility;
            const uint64_t official = dual ? rec.officialZ1Wh + rec.officialZ2Wh : rec.officialZWh;
            const uint64_t previous = dual ? base.officialZ1Wh + base.officialZ2Wh : base.officialZWh;
            const uint64_t currentImport = dual ? nowPoint.z1ImportWh + nowPoint.z2ImportWh : nowPoint.importWh;
            const uint64_t currentExport = dual ? nowPoint.z1ExportWh + nowPoint.z2ExportWh : nowPoint.exportWh;
            const uint64_t initialImport = dual ? v3EstimatePoint.z1ImportWh + v3EstimatePoint.z2ImportWh : v3EstimatePoint.importWh;
            const uint64_t initialExport = dual ? v3EstimatePoint.z1ExportWh + v3EstimatePoint.z2ExportWh : v3EstimatePoint.exportWh;
            if (currentImport >= initialImport && currentExport >= initialExport) {
                const int64_t predicted = v3SignedDeltaWh(currentImport, initialImport) -
                    v3SignedDeltaWh(currentExport, initialExport);
                const int64_t actual = v3SignedDeltaWh(official, previous);
                rec.spanImportWh = static_cast<uint64_t>(predicted);
                rec.spanZ1Wh = static_cast<uint64_t>(actual);
                rec.spanZ2Wh = previous;
                rec.reserved2[1] |= V3_EK_BASE_KNOWN;
                if (actual != 0) rec.reserved2[1] |= V3_EK_DEVIATION;
            }
        }
        rec.checksum = v3UtilityChecksum(rec);
        return true;
    }

    V3AccumPoint a = {}, b = {};
    if (exactTime) {
        if (getV3AccumAtEpoch(measuredEpoch, a)) v3SetUtilityAnchor(rec, a);
    } else {
        uint32_t epoch08 = 0, epoch18 = 0;
        if (parseV3DateEpoch(dateText, V3_WINDOW_MINUTE_START, epoch08) &&
            parseV3DateEpoch(dateText, V3_WINDOW_MINUTE_END, epoch18) &&
            getV3AccumAtEpoch(epoch08, a) && getV3AccumAtEpoch(epoch18, b) &&
            a.continuityGeneration == b.continuityGeneration) {
            V3AccumPoint mid = {};
            mid.importWh = (a.importWh + b.importWh) / 2ULL;
            mid.z1ImportWh = (a.z1ImportWh + b.z1ImportWh) / 2ULL;
            mid.z2ImportWh = (a.z2ImportWh + b.z2ImportWh) / 2ULL;
            v3SetUtilityAnchor(rec, mid);
            rec.spanImportWh = v3AbsDiffU64(a.importWh, b.importWh);
            rec.spanZ1Wh = v3AbsDiffU64(a.z1ImportWh, b.z1ImportWh);
            rec.spanZ2Wh = v3AbsDiffU64(a.z2ImportWh, b.z2ImportWh);
        }
    }
    rec.checksum = v3UtilityChecksum(rec);
    return true;
}


uint32_t nextV3UtilityId() {
    uint32_t maxId = 0;
    if (!littleFsReady || !LittleFS.exists(V3_UTILITY_FILE)) return 1;

    File f = LittleFS.open(V3_UTILITY_FILE, "r");
    if (!f) return 1;

    V3UtilityReading rec;
    while (f.readBytes(reinterpret_cast<char*>(&rec), sizeof(rec)) ==
           sizeof(rec)) {
        yield();
        if (v3UtilityValid(rec) && rec.id > maxId) maxId = rec.id;
    }
    f.close();
    return maxId + 1;
}

bool appendV3Utility(const V3UtilityReading& rec) {
    if (!littleFsReady) return false;
    V3AccumPoint point = {};
    if (getV3UtilityPoint(rec, point) && !saveV3UtilityPoint(rec, point)) return false;
    File f = LittleFS.open(V3_UTILITY_FILE, "a");
    if (!f) return false;
    bool ok = f.write(reinterpret_cast<const uint8_t*>(&rec),
                      sizeof(rec)) == sizeof(rec);
    f.close();
    if (ok) refreshV3LatestUtility();
    return ok;
}

bool findV3UtilityRecord(File& f, uint32_t id, V3UtilityReading& rec, uint32_t& offset) {
    offset = 0;
    while (f.readBytes(reinterpret_cast<char*>(&rec), sizeof(rec)) == sizeof(rec)) {
        yield();
        if (v3UtilityValid(rec) && rec.id == id) return true;
        offset += sizeof(rec);
    }
    return false;
}

bool updateV3UtilityRecord(uint32_t id, const V3UtilityReading& replacement) {
    if (!littleFsReady || !LittleFS.exists(V3_UTILITY_FILE)) return false;
    File f = LittleFS.open(V3_UTILITY_FILE, "r+");
    if (!f) return false;
    V3UtilityReading rec; uint32_t offset = 0;
    bool allowed = findV3UtilityRecord(f, id, rec, offset) && rec.active;
    V3UtilityReading stored = replacement;
    if (allowed && (rec.reserved2[1] & V3_EK_CORRECTION) &&
        replacement.reserved2[0] == V3_READING_INTERMEDIATE && replacement.mode == rec.mode && replacement.measuredEpoch == rec.measuredEpoch &&
        replacement.minuteOfDay == rec.minuteOfDay) {
        // Editing only the value retains the original physical/time anchor and
        // pre-correction prediction, even when it is no longer within five minutes.
        stored.enteredEpoch = rec.enteredEpoch;
        stored.anchorImportWh = rec.anchorImportWh;
        stored.anchorZ1ImportWh = rec.anchorZ1ImportWh;
        stored.anchorZ2ImportWh = rec.anchorZ2ImportWh;
        stored.mappingValid = rec.mappingValid;
        stored.reserved2[1] = rec.reserved2[1] & ~V3_EK_DEVIATION;
        stored.reserved2[2] = rec.reserved2[2];
        stored.spanImportWh = rec.spanImportWh;
        stored.spanZ1Wh = 0;
        stored.spanZ2Wh = rec.spanZ2Wh;
        if (rec.reserved2[1] & V3_EK_BASE_KNOWN) {
            uint64_t newOfficial = stored.mode == V3_MODE_DUAL
                ? stored.officialZ1Wh + stored.officialZ2Wh : stored.officialZWh;
            if ((rec.reserved2[2] & V3_EK_NET_SNAPSHOT) ? newOfficial != rec.spanZ2Wh : newOfficial > rec.spanZ2Wh) {
                stored.spanZ1Wh = (rec.reserved2[2] & V3_EK_NET_SNAPSHOT)
                    ? static_cast<uint64_t>(v3SignedDeltaWh(newOfficial, rec.spanZ2Wh))
                    : newOfficial - rec.spanZ2Wh;
                stored.reserved2[1] |= V3_EK_DEVIATION;
            }
        }
        stored.checksum = v3UtilityChecksum(stored);
    }
    V3AccumPoint point = {};
    if (allowed && getV3UtilityPoint(stored, point) && !saveV3UtilityPoint(stored, point)) allowed = false;
    bool ok = allowed && f.seek(offset, SeekSet) &&
              f.write(reinterpret_cast<const uint8_t*>(&stored), sizeof(stored)) == sizeof(stored);
    f.close();
    if (ok) refreshV3LatestUtility();
    return ok;
}


bool deleteV3UtilityRecord(uint32_t id) {
    if (!littleFsReady || !LittleFS.exists(V3_UTILITY_FILE)) return false;
    File f = LittleFS.open(V3_UTILITY_FILE, "r+");
    if (!f) return false;
    V3UtilityReading rec; uint32_t offset = 0;
    refreshV3LatestUtility();
    bool found = findV3UtilityRecord(f, id, rec, offset) && rec.active &&
        (rec.reserved2[0] != V3_READING_INTERMEDIATE || id == v3LatestIntermediateId);
    if (found) { rec.active = 0; rec.checksum = v3UtilityChecksum(rec); }
    bool ok = found && f.seek(offset, SeekSet) &&
              f.write(reinterpret_cast<const uint8_t*>(&rec), sizeof(rec)) == sizeof(rec);
    f.close();
    if (ok) refreshV3LatestUtility();
    return ok;
}








// EnergyMeter driver extraction from source release 0c874d3 (v5.59).
// This fragment is concatenated after EnergySnapshot.h by tools/generate.py.
// No application state, storage schema, accounting or display dependency lives here.

#if METER_TYPE == METER_DDS238
#include <ModbusMaster.h>
ModbusMaster node;

void preTransmission() {
    digitalWrite(DE_RE, HIGH);
    delayMicroseconds(1000);
}

void postTransmission() {
    digitalWrite(DE_RE, LOW);
    delayMicroseconds(1000);
}

void modbusIdle() { yield(); }

bool readDDS238(uint16_t addr, uint16_t count, float scale, float& outVal) {
    uint8_t result = node.readHoldingRegisters(addr, count);
    if (result != node.ku8MBSuccess) return false;

    if (count == 1) {
        int16_t val = (int16_t)node.getResponseBuffer(0);
        outVal = (float)val * scale;
        return true;
    }

    if (count == 2) {
        uint32_t raw = ((uint32_t)node.getResponseBuffer(0) << 16) |
                       node.getResponseBuffer(1);
        outVal = (float)raw * scale;
        return true;
    }

    return false;
}

bool readDDS238Raw32(uint16_t addr, uint32_t& outRaw) {
    uint8_t result = node.readHoldingRegisters(addr, 2);
    if (result != node.ku8MBSuccess) return false;

    outRaw = ((uint32_t)node.getResponseBuffer(0) << 16) |
             (uint32_t)node.getResponseBuffer(1);
    return true;
}

bool readDDS238Measurements(EnergySnapshot& out) {
    node.begin(ID, Serial);
    EnergySnapshot next = {};
    uint32_t rawEnergyRev = 0;
    uint32_t rawEnergyFwd = 0;

    // Preserve the signed 16-bit/scaled DDS register map and stop immediately
    // on the first failure. The caller's previous sample remains unchanged.
    if (!readDDS238(0x000C, 1, 0.1f, next.voltage) ||
        !readDDS238(0x000D, 1, 0.01f, next.current) ||
        !readDDS238(0x000E, 1, 1.0f, next.power) ||
        !readDDS238(0x0010, 1, 0.001f, next.pf) ||
        !readDDS238(0x0011, 1, 0.01f, next.freq) ||
        !readDDS238Raw32(0x0008, rawEnergyRev) ||
        !readDDS238Raw32(0x000A, rawEnergyFwd)) return false;

    next.totalEnergyFwd = (float)rawEnergyFwd / 100.0f;
    next.totalEnergyRev = (float)rawEnergyRev / 100.0f;
    next.rawImportWh = (uint64_t)rawEnergyFwd * 10ULL;
    next.rawExportWh = (uint64_t)rawEnergyRev * 10ULL;
    out = next;
    return true;
}

void beginMeasurementDriver() {
    pinMode(DE_RE, OUTPUT);
    postTransmission();
    Serial.begin(9600, SERIAL_8N1);
    node.begin(ID, Serial);
    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);
    node.idle(modbusIdle);
}

bool readMeasurementDriver(EnergySnapshot& out) {
    return readDDS238Measurements(out);
}

#elif METER_TYPE == METER_JSY_MK333
#include <SoftwareSerial.h>
// JSY TXD -> D5 / GPIO14; JSY RXD -> D6 / GPIO12.
SoftwareSerial jsy(D5, D6);

uint16_t modbusCRC(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t pos = 0; pos < len; pos++)
    {
        crc ^= data[pos];

        for (uint8_t i = 0; i < 8; i++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

bool readHoldingRegisters(uint8_t slave, uint16_t startRegister,
                          uint16_t quantity, uint16_t *values)
{
    if (values == nullptr || quantity == 0 || quantity > 29) return false;
    uint8_t request[8];

    request[0] = slave;
    request[1] = 0x03;
    request[2] = highByte(startRegister);
    request[3] = lowByte(startRegister);
    request[4] = highByte(quantity);
    request[5] = lowByte(quantity);

    uint16_t crc = modbusCRC(request, 6);
    request[6] = lowByte(crc);
    request[7] = highByte(crc);

    // Καθαρισμός RX buffer
    unsigned long drainStart = millis();
    uint16_t drained = 0;
    while (jsy.available() && drained < 128 && millis() - drainStart < 25UL)
    {
        jsy.read();
        ++drained;
        yield();
    }
    if (jsy.available()) return false; // Continuous RX noise cannot trap this task.

    // Αποστολή request
    if (jsy.write(request, 8) != 8) return false;
    // Core 2.7.2 SoftwareSerial::write() transmits synchronously. Its flush()
    // clears RX, not TX; calling it here can discard an early meter response.

    uint16_t expectedLength = 5 + (quantity * 2);
    uint8_t response[64];

    if (expectedLength > sizeof(response))
    {
        return false;
    }

    uint16_t count = 0;
    unsigned long startTime = millis();

    while ((millis() - startTime) < 500)
    {
        while (jsy.available() && millis() - startTime < 500UL)
        {
            if (count < sizeof(response))
            {
                response[count++] = jsy.read();
            }
            else
            {
                jsy.read();
            }

            if (count >= expectedLength)
            {
                break;
            }
        }

        if (count >= expectedLength)
        {
            break;
        }

        yield();
    }

    if (count != expectedLength)
    {
        Serial.println("MODBUS: No/short response");
        return false;
    }

    if (response[0] != slave)
    {
        Serial.println("MODBUS: Wrong slave");
        return false;
    }

    if (response[1] != 0x03)
    {
        Serial.println("MODBUS: Wrong function");
        return false;
    }

    if (response[2] != quantity * 2)
    {
        Serial.println("MODBUS: Wrong byte count");
        return false;
    }

    uint16_t calculatedCRC = modbusCRC(response, expectedLength - 2);
    uint16_t receivedCRC = response[expectedLength - 2] |
                           ((uint16_t)response[expectedLength - 1] << 8);

    if (calculatedCRC != receivedCRC)
    {
        Serial.println("MODBUS: CRC error");
        return false;
    }

    for (uint16_t i = 0; i < quantity; i++)
    {
        values[i] = ((uint16_t)response[3 + (i * 2)] << 8) |
                    response[4 + (i * 2)];
    }

    return true;
}

uint32_t makeJSYUint32(uint16_t highReg, uint16_t lowReg)
{
    return ((uint32_t)highReg << 16) | (uint32_t)lowReg;
}

bool readJSYMeasurements(EnergySnapshot& out)
{
    uint16_t voltageRegs[3];
    uint16_t currentRegs[3];
    uint16_t powerRegs[3];
    uint16_t freqPfRegs[5];
    uint16_t totalPowerRegs[2];
    uint16_t forwardEnergyRegs[2];
    uint16_t reverseEnergyRegs[2];
    uint16_t directionRegs[1];

    if (!readHoldingRegisters(ID, 0x0100, 3, voltageRegs)) return false;
    if (!readHoldingRegisters(ID, 0x0103, 3, currentRegs)) return false;
    if (!readHoldingRegisters(ID, 0x0106, 3, powerRegs)) return false;
    if (!readHoldingRegisters(ID, 0x0115, 5, freqPfRegs)) return false;
    if (!readHoldingRegisters(ID, 0x0109, 2, totalPowerRegs)) return false;
    if (!readHoldingRegisters(ID, 0x0132, 1, directionRegs)) return false;
    // Dedicated directional cumulative active-energy registers.
    if (!readHoldingRegisters(ID, 0x013A, 2, forwardEnergyRegs)) return false;
    if (!readHoldingRegisters(ID, 0x0142, 2, reverseEnergyRegs)) return false;

    float newVoltage1 = voltageRegs[0] / 100.0f;
    float newVoltage2 = voltageRegs[1] / 100.0f;
    float newVoltage3 = voltageRegs[2] / 100.0f;

    float newCurrent1 = currentRegs[0] / 100.0f;
    float newCurrent2 = currentRegs[1] / 100.0f;
    float newCurrent3 = currentRegs[2] / 100.0f;

    uint16_t newPowerDirection = directionRegs[0];
    bool newReverse1 = (newPowerDirection & 0x0001) != 0;
    bool newReverse2 = (newPowerDirection & 0x0002) != 0;
    bool newReverse3 = (newPowerDirection & 0x0004) != 0;
    bool newReverseTotal = (newPowerDirection & 0x0008) != 0;

    // Τα current registers είναι unsigned magnitudes.
    // Το πρόσημο που εμφανίζουμε προκύπτει από το 0x0132.
    if (newReverse1) newCurrent1 = -newCurrent1;
    if (newReverse2) newCurrent2 = -newCurrent2;
    if (newReverse3) newCurrent3 = -newCurrent3;

    float newPower1 = powerRegs[0];
    float newPower2 = powerRegs[1];
    float newPower3 = powerRegs[2];

    float newFreq = freqPfRegs[0] / 100.0f;
    float newPf1 = freqPfRegs[1] / 1000.0f;
    float newPf2 = freqPfRegs[2] / 1000.0f;
    float newPf3 = freqPfRegs[3] / 1000.0f;
    float newPfTotal = freqPfRegs[4] / 1000.0f;

    float newTotalPower = (float)makeJSYUint32(totalPowerRegs[0], totalPowerRegs[1]);
    uint32_t rawForwardEnergy = makeJSYUint32(forwardEnergyRegs[0], forwardEnergyRegs[1]);
    uint32_t rawReverseEnergy = makeJSYUint32(reverseEnergyRegs[0], reverseEnergyRegs[1]);

    EnergySnapshot next = {};
    next.voltage1 = newVoltage1;
    next.voltage2 = newVoltage2;
    next.voltage3 = newVoltage3;
    next.current1 = newCurrent1;
    next.current2 = newCurrent2;
    next.current3 = newCurrent3;
    next.jsyPowerDirection = newPowerDirection;
    next.jsyReverse1 = newReverse1;
    next.jsyReverse2 = newReverse2;
    next.jsyReverse3 = newReverse3;
    next.jsyReverseTotal = newReverseTotal;
    next.power1 = newPower1;
    next.power2 = newPower2;
    next.power3 = newPower3;
    next.totalPowerJSY = newTotalPower;
    next.pf1 = newPf1;
    next.pf2 = newPf2;
    next.pf3 = newPf3;
    next.voltage = (next.voltage1 + next.voltage2 + next.voltage3) / 3.0f;
    next.current = next.current1 + next.current2 + next.current3;
    next.power = next.totalPowerJSY;
    next.pf = newPfTotal;
    next.freq = newFreq;
    next.totalEnergyFwd = rawForwardEnergy / 100.0f;
    next.totalEnergyRev = rawReverseEnergy / 100.0f;
    next.rawImportWh = (uint64_t)rawForwardEnergy * 10ULL;
    next.rawExportWh = (uint64_t)rawReverseEnergy * 10ULL;
    out = next;
    return true;
}

void beginMeasurementDriver() {
    Serial.begin(115200);
    jsy.begin(9600);
    delay(500);
}

bool readMeasurementDriver(EnergySnapshot& out) {
    return readJSYMeasurements(out);
}

#else
#error "Unsupported measurement driver (select DDS238 or JSY-MK333)"
#endif


bool parseV3TimeToken(const String& token, bool& exact, uint16_t& minuteOut) {
    String t = token;
    t.trim();

    if (t.equalsIgnoreCase("window")) {
        exact = false;
        minuteOut = V3_WINDOW_MINUTE_MID;
        return true;
    }

    if (t.length() != 5 || t[2] != ':') return false;
    for (uint8_t i = 0; i < 5; ++i) {
        if (i != 2 && (t[i] < '0' || t[i] > '9')) return false;
    }
    int hour = t.substring(0, 2).toInt();
    int minute = t.substring(3, 5).toInt();

    if (hour < 0 || hour > 23 || minute < 0 || minute > 59) return false;

    exact = true;
    minuteOut = (uint16_t)(hour * 60 + minute);
    return true;
}

bool parseV3OfficialKwh(const char* text, double& value) {
    if (!text || !*text) return false;
    char* end = nullptr;
    value = strtod(text, &end);
    if (end == text) return false;
    while (*end == ' ' || *end == '\t') ++end;
    return *end == '\0' && value > 0.0 && value < 999999.0;
}

bool stageV3UtilityReadingCommand(const String& message, bool updateExisting) {
    const char* prefix = updateExisting
        ? "utility_reading_update|"
        : "utility_reading_add|";

    String rest = message.substring(strlen(prefix));

    String fields[8];
    uint8_t count = 0;
    int from = 0;

    while (count < 8) {
        int sep = rest.indexOf('|', from);
        if (sep < 0) {
            fields[count++] = rest.substring(from);
            break;
        }
        fields[count++] = rest.substring(from, sep);
        from = sep + 1;
    }

    uint8_t offset = 0;
    uint32_t id = 0;
    if (updateExisting) {
        if (count < 5) return false;
        id = (uint32_t)strtoul(fields[0].c_str(), nullptr, 10);
        if (id == 0) return false;
        offset = 1;
    }

    if (count < offset + 4) return false;

    uint8_t readingKind = V3_READING_FINAL; // old v3 commands remain T.K.
    String maybeKind = fields[offset];
    maybeKind.trim();
    if (maybeKind.equalsIgnoreCase("tk") || maybeKind.equalsIgnoreCase("ek")) {
        readingKind = maybeKind.equalsIgnoreCase("ek")
            ? V3_READING_INTERMEDIATE : V3_READING_FINAL;
        offset++;
        if (count < offset + 4) return false;
    }

    String mode = fields[offset + 0];
    String date = fields[offset + 1];
    String timeToken = fields[offset + 2];
    mode.trim();
    date.trim();
    timeToken.trim();

    bool dual = mode.equalsIgnoreCase("dual");
    bool mono = mode.equalsIgnoreCase("mono");
    if (!dual && !mono) return false;

    bool exact = false;
    uint16_t minute = 0;
    if (readingKind == V3_READING_INTERMEDIATE && !updateExisting) {
        // New E.K. is always current; the ESP chooses the timestamp after the read.
        exact = true;
        date = "2024-01-01";
    } else {
        if (!parseV3TimeToken(timeToken, exact, minute)) return false;
        if (date.length() != 10) return false;
    }

    double z = 0.0, z1 = 0.0, z2 = 0.0;
    if (dual) {
        if (count != offset + 5 ||
            !parseV3OfficialKwh(fields[offset + 3].c_str(), z1) ||
            !parseV3OfficialKwh(fields[offset + 4].c_str(), z2)) return false;
    } else {
        if (count != offset + 4 || !parseV3OfficialKwh(fields[offset + 3].c_str(), z)) return false;
    }

    mqttAdminUtilityId = id;
    mqttAdminUtilityDual = dual;
    mqttAdminUtilityExactTime = exact;
    mqttAdminUtilityMinute = minute;
    date.toCharArray(mqttAdminUtilityDate, sizeof(mqttAdminUtilityDate));
    mqttAdminUtilityZ = z;
    mqttAdminUtilityZ1 = z1;
    mqttAdminUtilityZ2 = z2;
    mqttAdminUtilityKind = readingKind;
    mqttAdminCommand = updateExisting
        ? MQTT_ADMIN_UTILITY_UPDATE
        : MQTT_ADMIN_UTILITY_ADD;
    mqttAdminCommandPending = true;
    return true;
}

bool publishMqttSample() {
    if (!mqttClient.connected()) return false;

    char dateTime[32] = {};
    formatCurrentDateTime(dateTime, sizeof(dateTime));

    refreshV3EstimateView();

    const char* tariffZone = currentTariffZone();
    const bool hasUtility =
        v3LatestUtilityValid &&
        ((dualZoneEnabled && v3LatestUtility.mode == V3_MODE_DUAL) ||
         (!dualZoneEnabled && v3LatestUtility.mode == V3_MODE_MONO));
    const bool estimateModeMatches =
        v3EstimateUtilityValid &&
        ((dualZoneEnabled && v3EstimateUtility.mode == V3_MODE_DUAL) ||
         (!dualZoneEnabled && v3EstimateUtility.mode == V3_MODE_MONO));
    const bool estimateValid =
        estimateModeMatches && v3EstimatePointValid &&
        v3EstimatePoint.continuityGeneration == v3State.continuityGeneration;
    const bool differenceValid = hasUtility && estimateValid;

    char utilityDate[11] = {};
    if (hasUtility) {
        formatV3Date(v3LatestUtility.measuredEpoch, utilityDate, sizeof(utilityDate));
    }

    const double zRef = hasUtility && !dualZoneEnabled ? tariffZReference : 0.0f;
    const double zNow = estimateValid && !dualZoneEnabled ? tariffZNow : 0.0f;
    const double zDiff = differenceValid && !dualZoneEnabled ? zNow - zRef : 0.0f;

    const double z1Ref = hasUtility && dualZoneEnabled ? tariffZ1Reference : 0.0f;
    const double z2Ref = hasUtility && dualZoneEnabled ? tariffZ2Reference : 0.0f;
    const double z1Now = estimateValid && dualZoneEnabled ? tariffZ1Now : 0.0f;
    const double z2Now = estimateValid && dualZoneEnabled ? tariffZ2Now : 0.0f;
    const double z1Diff = differenceValid && dualZoneEnabled ? z1Now - z1Ref : 0.0f;
    const double z2Diff = differenceValid && dualZoneEnabled ? z2Now - z2Ref : 0.0f;

    markCrashStage(CRASH_STAGE_MQTT_JSON_BUILD, ESP.getFreeHeap());
    char payload[MQTT_STATE_PAYLOAD_BYTES];
    int n = 0;
#if METER_TYPE == METER_DDS238
    n = snprintf_P(
        payload, MQTT_STATE_PAYLOAD_BYTES,
        PSTR("{"
             "\"meter\":\"%s\",\"device\":\"%s\",\"firmware\":\"%s\","
             "\"dual_zone\":%s,\"tariff_mode\":\"%s\","
             "\"energy_basis\":\"net\",\"has_utility\":%s,\"estimate_valid\":%s,"
             "\"utility_date\":\"%s\","
             "\"z_ref\":%.2f,\"z_now\":%.2f,\"z_diff\":%.2f,"
             "\"z1_ref\":%.2f,\"z1_now\":%.2f,\"z1_diff\":%.2f,"
             "\"z2_ref\":%.2f,\"z2_now\":%.2f,\"z2_diff\":%.2f,"
             "\"tariff_zone\":\"%s\","
             "\"voltage\":%.1f,\"current\":%.2f,\"power\":%.0f,"
             "\"pf\":%.3f,\"frequency\":%.2f,"
             "\"datetime\":\"%s\",\"rssi\":%d}"),
        METER_NAME, mqttDeviceId, VERSION,
        dualZoneEnabled ? "true" : "false",
        dualZoneEnabled ? "dual" : "mono",
        hasUtility ? "true" : "false",
        estimateValid ? "true" : "false",
        utilityDate,
        zRef, zNow, zDiff,
        z1Ref, z1Now, z1Diff,
        z2Ref, z2Now, z2Diff,
        tariffZone,
        voltage, current, power, pf, freq,
        dateTime, WiFi.RSSI());
#else
    const float signedP1 = jsyReverse1 ? -fabs(power1) : fabs(power1);
    const float signedP2 = jsyReverse2 ? -fabs(power2) : fabs(power2);
    const float signedP3 = jsyReverse3 ? -fabs(power3) : fabs(power3);
    const float signedPTotal =
        jsyReverseTotal ? -fabs(totalPowerJSY) : fabs(totalPowerJSY);

    n = snprintf_P(
        payload, MQTT_STATE_PAYLOAD_BYTES,
        PSTR("{"
             "\"meter\":\"%s\",\"device\":\"%s\",\"firmware\":\"%s\","
             "\"dual_zone\":%s,\"tariff_mode\":\"%s\","
             "\"energy_basis\":\"net\",\"has_utility\":%s,\"estimate_valid\":%s,"
             "\"utility_date\":\"%s\","
             "\"z_ref\":%.2f,\"z_now\":%.2f,\"z_diff\":%.2f,"
             "\"z1_ref\":%.2f,\"z1_now\":%.2f,\"z1_diff\":%.2f,"
             "\"z2_ref\":%.2f,\"z2_now\":%.2f,\"z2_diff\":%.2f,"
             "\"tariff_zone\":\"%s\","
             "\"v1\":%.2f,\"v2\":%.2f,\"v3\":%.2f,"
             "\"i1\":%.2f,\"i2\":%.2f,\"i3\":%.2f,"
             "\"dir1\":\"%s\",\"dir2\":\"%s\",\"dir3\":\"%s\","
             "\"power_direction\":%u,"
             "\"p1\":%.0f,\"p2\":%.0f,\"p3\":%.0f,\"power_total\":%.0f,"
             "\"pf1\":%.3f,\"pf2\":%.3f,\"pf3\":%.3f,"
             "\"frequency\":%.2f,"
             "\"datetime\":\"%s\",\"rssi\":%d}"),
        METER_NAME, mqttDeviceId, VERSION,
        dualZoneEnabled ? "true" : "false",
        dualZoneEnabled ? "dual" : "mono",
        hasUtility ? "true" : "false",
        estimateValid ? "true" : "false",
        utilityDate,
        zRef, zNow, zDiff,
        z1Ref, z1Now, z1Diff,
        z2Ref, z2Now, z2Diff,
        tariffZone,
        voltage1, voltage2, voltage3,
        current1, current2, current3,
        jsyReverse1 ? "REV" : "FWD",
        jsyReverse2 ? "REV" : "FWD",
        jsyReverse3 ? "REV" : "FWD",
        (unsigned int)jsyPowerDirection,
        signedP1, signedP2, signedP3, signedPTotal,
        pf1, pf2, pf3,
        freq,
        dateTime, WiFi.RSSI());
#endif

    if (n <= 0 || n >= (int)MQTT_STATE_PAYLOAD_BYTES) return false;

    // Do not keep these as globals: they are needed only for this decision.
    // This also avoids storing stale "last publish" values that nothing reads.
    uint32_t publishHeapFree = 0;
    uint16_t publishMaxBlock = 0;
    uint8_t publishFragmentation = 0;
    ESP.getHeapStats(&publishHeapFree, &publishMaxBlock, &publishFragmentation);
    const bool publishBlockedLowMemory =
        publishHeapFree < 2400UL || publishMaxBlock < 1600U;
    markCrashStage(CRASH_STAGE_MQTT_PUBLISH, (uint32_t)n);

    if (publishBlockedLowMemory) {
        meterHealthDirty = true;
        return false;
    }

    return mqttClient.publish(MQTT_STATE_TOPIC, payload, false);
}

void handleRoot() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;

    const bool liveRequest = server.uri() == "/live";
    WiFiClient client = server.client();
    client.setTimeout(700);
    client.setNoDelay(true);
    webRootRequestCount++;

    const unsigned long rootNow = millis();
    if (webRootRequestCount > 1 &&
        rootNow - webRootLastRequestMs < WEB_ROOT_MIN_REQUEST_GAP_MS) {
        webRootRateLimitedCount++;
        sendRootBusyAndClose(client,429,ESP.getFreeHeap(),ESP.getMaxFreeBlockSize());
        return;
    }
    webRootLastRequestMs = rootNow;

    uint32_t rootFreeHeap = 0;
    uint16_t rootMaxBlock = 0;
    uint8_t rootFragmentation = 0;
    if (!webRootHeapHealthy(&rootFreeHeap, &rootMaxBlock, &rootFragmentation)) {
        markCrashStage(CRASH_STAGE_WEB_ROOT, rootFreeHeap);
        ++webRootBusyCount;
        sendRootBusyAndClose(client,503,rootFreeHeap,rootMaxBlock);
        return;
    }

    markCrashStage(CRASH_STAGE_WEB_ROOT, rootFreeHeap);
    refreshV3EstimateView();
    static const char header[] =
        "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: no-store\r\nConnection: close\r\n\r\n";
    if (!writeRootBytes(client, reinterpret_cast<const uint8_t*>(header), sizeof(header)-1)) return;
    PGM_P head = PSTR(
        "<!doctype html><html><head><meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>EnergyMeter</title><style>"
        "body{font:22px/1.35 Arial,sans-serif;max-width:760px;margin:auto;padding:4px;color:#142431}"
        "h2{font-size:34px;margin:0 0 6px}h3{font-size:inherit;margin:16px 0 6px}"
        ".dim{color:#64727c}pre{font:inherit;white-space:pre-wrap;overflow-wrap:anywhere;margin:6px 0}"
        ".utility-title{display:flex;flex-wrap:wrap;align-items:baseline;gap:6px 16px;margin-top:10px}"
        ".utility-title h3{margin:0}.utility-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:12px;margin-top:0}"
        ".utility-grid div{overflow-wrap:anywhere}.table-wrap{overflow-x:auto}"
        "table{border-collapse:collapse;width:100%;max-width:400px;font:inherit}"
        "th,td{padding:5px 7px;border-bottom:1px solid #e5eaed;text-align:right;white-space:nowrap}"
        "th:first-child,td:first-child{text-align:left}a{margin-right:10px}p{margin:10px 0}"
        "@media(max-width:480px){.utility-grid{grid-template-columns:1fr}}"
        "</style></head><body><h2>EnergyMeter v" VERSION "</h2>"
        "<div id='live-readings'>");
    if (!liveRequest && !writeRootProgmemViaRam(client,head,strlen_P(head))) return;
    PGM_P readingsHead=PSTR("<section><div class='utility-grid'>");
    if (!writeRootProgmemViaRam(client,readingsHead,strlen_P(readingsHead))) return;
    char line[384];
#if METER_TYPE == METER_DDS238
    const float signedTotalPower = power;
#else
    const float signedTotalPower = jsyReverseTotal ? -fabs(totalPowerJSY) : fabs(totalPowerJSY);
#endif
    int n=0;
    const uint8_t rootMode = dualZoneEnabled ? V3_MODE_DUAL : V3_MODE_MONO;
    const bool rootEstimateValid =
        v3EstimateUtilityValid && v3EstimatePointValid &&
        v3EstimateUtility.mode == rootMode &&
        v3EstimatePoint.continuityGeneration == v3State.continuityGeneration;
    const bool rootReferenceValid =
        v3LatestUtilityValid && v3LatestUtility.mode == rootMode;
    if (rootEstimateValid || rootReferenceValid) {
        const bool dual = dualZoneEnabled;
        for (uint8_t zone=0; zone<(dual ? 2 : 1); ++zone) {
            const char* label = dual ? (zone==0 ? "Z1" : "Z2") : "Z";
            const double latest = dual
                ? (zone==0 ? tariffZ1Reference : tariffZ2Reference) : tariffZReference;
            const double now = dual
                ? (zone==0 ? tariffZ1Now : tariffZ2Now) : tariffZNow;
            char diffText[24] = "--";
            char nowText[24] = "--";
            char latestText[24] = "--";
            if (rootEstimateValid) snprintf(nowText, sizeof(nowText), "%.2f", now);
            if (rootReferenceValid) snprintf(latestText, sizeof(latestText), "%.0f", latest);
            if (rootEstimateValid && rootReferenceValid)
                snprintf(diffText, sizeof(diffText), "%+.2f", now-latest);
            n=snprintf(line,sizeof(line),
                "<div>%s diff: %s kWh<br>%s now: %s kWh<br>%s latest: %s kWh</div>",
                label,diffText,label,nowText,label,latestText);
            if (n<=0 || n>=(int)sizeof(line)) {closeRootClient(client,false);return;}
            if (!writeRootBytes(client, reinterpret_cast<const uint8_t*>(line), (size_t)n)) return;
        }
    } else if (!writeRootProgmemViaRam(client,
        PSTR("<div>No DEH indication or measurement recorded.</div>"),
        strlen_P(PSTR("<div>No DEH indication or measurement recorded.</div>")))) return;
    n=snprintf(line,sizeof(line),"</div></section><section><div class='utility-title'><h3>Measurements</h3><span>Total power: %.0f W</span></div><div class='table-wrap'><table>",signedTotalPower);
    if (n<=0 || n>=(int)sizeof(line)) {closeRootClient(client,false);return;}
    if (!writeRootBytes(client, reinterpret_cast<const uint8_t*>(line), (size_t)n)) return;
#if METER_TYPE == METER_DDS238
    n=snprintf(line,sizeof(line),
        "<tr><td>Voltage</td><td>%.1f V</td></tr><tr><td>Current</td><td>%.2f A</td></tr>"
        "<tr><td>Power</td><td>%.0f W</td></tr><tr><td>PF</td><td>%.3f</td></tr>",
        voltage,fabs(current),power,pf);
    if (n<=0 || n>=(int)sizeof(line)) {closeRootClient(client,false);return;}
    if (!writeRootBytes(client, reinterpret_cast<const uint8_t*>(line), (size_t)n)) return;
#else
    PGM_P tableHead=PSTR("<tr><th>Phase</th><th>W</th><th>V</th><th>A</th><th>PF</th></tr>");
    if (!writeRootProgmemViaRam(client,tableHead,strlen_P(tableHead))) return;
    const float signedP1=jsyReverse1 ? -fabs(power1) : fabs(power1);
    const float signedP2=jsyReverse2 ? -fabs(power2) : fabs(power2);
    const float signedP3=jsyReverse3 ? -fabs(power3) : fabs(power3);
    n=snprintf(line,sizeof(line),
        "<tr><td>R</td><td>%.0f</td><td>%.1f</td><td>%.2f</td><td>%.3f</td></tr>"
        "<tr><td>S</td><td>%.0f</td><td>%.1f</td><td>%.2f</td><td>%.3f</td></tr>"
        "<tr><td>T</td><td>%.0f</td><td>%.1f</td><td>%.2f</td><td>%.3f</td></tr>",
        signedP1,voltage1,fabs(current1),pf1,
        signedP2,voltage2,fabs(current2),pf2,
        signedP3,voltage3,fabs(current3),pf3);
    if (n<=0 || n>=(int)sizeof(line)) {closeRootClient(client,false);return;}
    if (!writeRootBytes(client, reinterpret_cast<const uint8_t*>(line), (size_t)n)) return;
#endif
    n=snprintf(line,sizeof(line),
        "</table></div><div class='utility-grid'><div>Frequency: %.2f Hz"
        "<p><a href='/network-debug'>Network Debug</a><a href='/setup'>Netword Setup</a></p></div>"
        "<div>Import: %.2f kWh<br>Export: %.2f kWh</div></div></section>",
        freq,(double)v3RawImportWh/1000.0,(double)v3RawExportWh/1000.0);
    if (n<=0 || n>=(int)sizeof(line)) {closeRootClient(client,false);return;}
    if (!writeRootBytes(client, reinterpret_cast<const uint8_t*>(line), (size_t)n)) return;
    n=snprintf(line,sizeof(line),
        "<section><pre>Wi-Fi: %s\nMQTT: %s\nRSSI: %d dBm\nFree heap: %lu bytes</pre></section>",
        WiFi.status()==WL_CONNECTED ? "CONNECTED" : "OFFLINE",
        mqttClient.connected() ? "CONNECTED" : "DISCONNECTED",WiFi.RSSI(),(unsigned long)ESP.getFreeHeap());
    if (n<=0 || n>=(int)sizeof(line)) {closeRootClient(client,false);return;}
    if (!writeRootBytes(client, reinterpret_cast<const uint8_t*>(line), (size_t)n)) return;
    char deviceDateTime[32];
    formatCurrentDateTime(deviceDateTime,sizeof(deviceDateTime));
    if (liveRequest) {
        n=snprintf_P(line,sizeof(line),PSTR("<!--device-time:%s-->"),deviceDateTime);
        if (n<=0 || n>=(int)sizeof(line)) {closeRootClient(client,false);return;}
        if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;
        PGM_P end=PSTR("<!--live-end-->");
        if (!writeRootProgmemViaRam(client,end,strlen_P(end))) return;
        closeRootClient(client,true);
        return;
    }
    PGM_P footer=PSTR(
        "</div><div id='live-state' class='dim' aria-live='polite'></div>"
        "<p><a href='/force-ota-recovery-confirm'>Force OTA-ONLY Recovery...</a></p>"
        "<div class='dim'>Device: " DEVICE_ID " · Meter: " METER_NAME "</div>");
    if (!writeRootProgmemViaRam(client,footer,strlen_P(footer))) return;
    n=snprintf_P(line,sizeof(line),PSTR("<div class='dim'>MQTT ID: %s</div>"),mqttDeviceId);
    if (n<=0 || n>=(int)sizeof(line)) {closeRootClient(client,false);return;}
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;
    n=snprintf_P(line,sizeof(line),PSTR("<div id='device-time' class='dim'>ESP8266 date/time: %s</div>"),deviceDateTime);
    if (n<=0 || n>=(int)sizeof(line)) {closeRootClient(client,false);return;}
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;
    PGM_P tail=PSTR(
        "<script>(function(){"
        "const readings=document.getElementById('live-readings'),state=document.getElementById('live-state'),clock=document.getElementById('device-time');"
        "async function update(){const started=Date.now();"
        "if(!document.hidden){const controller=new AbortController();"
        "const timeout=setTimeout(()=>controller.abort(),4000);"
        "try{const response=await fetch('/live',{cache:'no-store',signal:controller.signal});"
        "const text=await response.text();"
        "if(!response.ok||!text.endsWith('<!--live-end-->'))throw new Error('incomplete');"
        "const stamp=text.match(/<!--device-time:([^<]*)-->/);if(!stamp)throw new Error('clock');readings.innerHTML=text.slice(0,-15).replace(stamp[0],'');clock.textContent='ESP8266 date/time: '+stamp[1];state.textContent='';"
        "}catch(error){state.textContent='Waiting for live measurements...';}"
        "finally{clearTimeout(timeout);}}"
        "setTimeout(update,Math.max(0,5000-(Date.now()-started)));}"
        "setTimeout(update,5000);})();</script></body></html>");
    if (!writeRootProgmemViaRam(client,tail,strlen_P(tail))) return;
    closeRootClient(client, true);
}

bool prepareV3UtilityTimestamp(uint8_t kind, bool updateExisting, char* date, size_t dateSize,
                               bool& exact, uint16_t& minute) {
    if (kind != V3_READING_INTERMEDIATE || updateExisting) return true;
    if (!isDualZoneTimeValid() || dateSize < 11) return false;
    const time_t now = time(nullptr);
    struct tm local;
    if (!localtime_r(&now, &local)) return false;
    formatV3Date((uint32_t)now, date, dateSize);
    minute = (uint16_t)(local.tm_hour * 60 + local.tm_min);
    exact = true;
    return true;
}

void processV3UtilityReading(bool updateExisting) {
    const char* cmd = updateExisting
        ? "utility_reading_update"
        : "utility_reading_add";

    if (!isDualZoneTimeValid()) {
        char response[128];
        snprintf(response, sizeof(response),
                 "{\"ok\":false,\"cmd\":\"%s\",\"error\":\"time_not_valid\"}",
                 cmd);
        publishMqttAdminResponse(response);
        return;
    }

    if (mqttAdminUtilityDual != dualZoneEnabled) {
        char response[144];
        snprintf(response, sizeof(response),
                 "{\"ok\":false,\"cmd\":\"%s\",\"error\":\"tariff_mode_mismatch\"}",
                 cmd);
        publishMqttAdminResponse(response);
        return;
    }

    // A new "Καταχώρηση Ένδειξης Μετρητή ΔΕΗ" must snapshot a fresh
    // physical-meter read immediately before the ESP chooses the timestamp.
    // A dated "Καταχώρηση Μέτρησης ΔΕΗ" is comparison-only and therefore
    // must remain storable even if the physical meter is temporarily offline.
    const bool needsCurrentAnchor =
        mqttAdminUtilityKind == V3_READING_INTERMEDIATE && !updateExisting;
    if (needsCurrentAnchor && !readMeterWithHealth()) {
        char response[144];
        snprintf(response, sizeof(response),
                 "{\"ok\":false,\"cmd\":\"%s\",\"error\":\"" METER_READ_ERROR "\"}",
                 cmd);
        publishMqttAdminResponse(response);
        return;
    }

    if (!prepareV3UtilityTimestamp(mqttAdminUtilityKind, updateExisting,
            mqttAdminUtilityDate, sizeof(mqttAdminUtilityDate),
            mqttAdminUtilityExactTime, mqttAdminUtilityMinute)) {
        publishMqttAdminResponse("{\"ok\":false,\"error\":\"time_not_valid\"}");
        return;
    }

    uint32_t id = updateExisting
        ? mqttAdminUtilityId
        : nextV3UtilityId();

    if (id == 0) {
        publishMqttAdminResponse(
            "{\"ok\":false,\"error\":\"invalid_reading_id\"}"
        );
        return;
    }

    if (updateExisting) {
        refreshV3LatestUtility();
        File f = LittleFS.open(V3_UTILITY_FILE, "r");
        V3UtilityReading old = {}; uint32_t offset = 0;
        bool allowed = f && findV3UtilityRecord(f, id, old, offset) && old.active;
        if (f) f.close();
        if (!allowed) {
            publishMqttAdminResponse(
                "{\"ok\":false,\"cmd\":\"utility_reading_update\",\"error\":\"reading_update_not_allowed\"}");
            return;
        }
        v3UtilityExcludedId = id;
        refreshV3LatestUtility();
    }
    V3UtilityReading rec;
    bool built = buildV3UtilityRecord(
            rec,
            id,
            mqttAdminUtilityKind,
            mqttAdminUtilityDual,
            mqttAdminUtilityDate,
            mqttAdminUtilityExactTime,
            mqttAdminUtilityMinute,
            mqttAdminUtilityZ,
            mqttAdminUtilityZ1,
            mqttAdminUtilityZ2);
    v3UtilityExcludedId = 0;
    refreshV3LatestUtility();
    if (!built) {
        char response[128];
        snprintf(response, sizeof(response),
                 "{\"ok\":false,\"cmd\":\"%s\",\"error\":\"invalid_reading\"}",
                 cmd);
        publishMqttAdminResponse(response);
        return;
    }

    if (!updateExisting && mqttAdminUtilityKind == V3_READING_INTERMEDIATE &&
        (!rec.mappingValid || !(rec.reserved2[1] & V3_EK_CORRECTION))) {
        publishMqttAdminResponse(
            "{\"ok\":false,\"cmd\":\"utility_reading_add\",\"error\":\"indication_anchor_failed\"}");
        return;
    }

    bool stored = updateExisting
        ? updateV3UtilityRecord(id, rec)
        : appendV3Utility(rec);

    if (!stored) {
        char response[128];
        snprintf(response, sizeof(response),
                 "{\"ok\":false,\"cmd\":\"%s\",\"error\":\"storage_failed\"}",
                 cmd);
        publishMqttAdminResponse(response);
        return;
    }

    if (updateExisting) {
        File f = LittleFS.open(V3_UTILITY_FILE, "r");
        uint32_t offset = 0;
        if (f) { findV3UtilityRecord(f, id, rec, offset); f.close(); }
    }
    refreshV3EstimateView();

    char response[320];
    if (rec.mode == V3_MODE_DUAL) {
        snprintf(
            response,
            sizeof(response),
            "{\"ok\":true,\"cmd\":\"%s\",\"id\":%lu,"
            "\"kind\":\"%s\",\"mapped\":%s,\"z1\":%.2f,\"z2\":%.2f,"
            "\"z1_span\":%.2f,\"z2_span\":%.2f}",
            cmd,
            (unsigned long)rec.id,
            rec.reserved2[0] == V3_READING_INTERMEDIATE ? "ek" : "tk",
            rec.mappingValid ? "true" : "false",
            (double)rec.officialZ1Wh / 1000.0,
            (double)rec.officialZ2Wh / 1000.0,
            (rec.reserved2[1] & V3_EK_EVALUATED) ? 0.0 : (double)rec.spanZ1Wh / 1000.0,
            (rec.reserved2[1] & V3_EK_EVALUATED) ? 0.0 : (double)rec.spanZ2Wh / 1000.0
        );
    } else {
        snprintf(
            response,
            sizeof(response),
            "{\"ok\":true,\"cmd\":\"%s\",\"id\":%lu,"
            "\"kind\":\"%s\",\"mapped\":%s,\"z\":%.2f,\"span\":%.2f}",
            cmd,
            (unsigned long)rec.id,
            rec.reserved2[0] == V3_READING_INTERMEDIATE ? "ek" : "tk",
            rec.mappingValid ? "true" : "false",
            (double)rec.officialZWh / 1000.0,
            (rec.reserved2[1] & V3_EK_EVALUATED) ? 0.0 : (double)rec.spanImportWh / 1000.0
        );
    }

    publishMqttAdminResponse(response);
    publishMqttMeasurements();
}

void processV3UtilityDelete() {
    uint32_t id = mqttAdminUtilityId;
    if (id == 0 || !deleteV3UtilityRecord(id)) {
        publishMqttAdminResponse(
            "{\"ok\":false,\"cmd\":\"utility_reading_delete\",\"error\":\"reading_not_found_or_not_latest_ek\"}"
        );
        return;
    }

    refreshV3EstimateView();

    char response[112];
    snprintf(response, sizeof(response),
             "{\"ok\":true,\"cmd\":\"utility_reading_delete\",\"id\":%lu}",
             (unsigned long)id);
    publishMqttAdminResponse(response);
    publishMqttMeasurements();
}

void processV3UtilityList() {
    if (!mqttClient.connected()) return;

    uint16_t count = 0;
    if (littleFsReady && LittleFS.exists(V3_UTILITY_FILE)) {
        File f = LittleFS.open(V3_UTILITY_FILE, "r");
        if (f) {
            V3UtilityReading rec;
            while (f.readBytes(reinterpret_cast<char*>(&rec), sizeof(rec)) ==
                   sizeof(rec)) {
                yield();
                if (v3UtilityValid(rec) && rec.active) count++;
            }
            f.close();
        }
    }

    char beginMessage[48];
    snprintf(beginMessage, sizeof(beginMessage), "BEGIN|UTILITY|%u",
             (unsigned int)count);
    mqttClient.publish(MQTT_ADMIN_CSV_TOPIC, beginMessage, false);
    mqttClient.publish(
        MQTT_ADMIN_CSV_TOPIC,
        "ROW|0|ID,Date,Mode,Kind,Z,Z1,Z2,TimeToken,Mapped,Anchor,AnchorZ1,AnchorZ2,EKFlags,PredictedWh,ActualWh,CanDelete,NetSnapshot,PointEpoch,PointSource,InternalImport,InternalExport,Z1Import,Z1Export,Z2Import,Z2Export,Generation,SampleEpoch",
        false
    );

    refreshV3LatestUtility();
    uint16_t rowIndex = 0;
    if (littleFsReady && LittleFS.exists(V3_UTILITY_FILE)) {
        File f = LittleFS.open(V3_UTILITY_FILE, "r");
        if (f) {
            V3UtilityReading rec;
            while (f.readBytes(reinterpret_cast<char*>(&rec), sizeof(rec)) ==
                   sizeof(rec)) {
                yield();
                if (!mqttClient.connected()) break;
                if (!v3UtilityValid(rec) || !rec.active) continue;

                char date[16] = {};
                formatV3Date(rec.measuredEpoch, date, sizeof(date));

                char timeToken[12] = "window";
                if (rec.minuteOfDay != V3_TIME_WINDOW &&
                    rec.minuteOfDay < 24U * 60U) {
                    snprintf(timeToken, sizeof(timeToken), "%02u:%02u",
                             (unsigned int)(rec.minuteOfDay / 60U),
                             (unsigned int)(rec.minuteOfDay % 60U));
                }

                V3AccumPoint point = {};
                const bool pointValid = getV3UtilityPoint(rec, point);
                char row[640];
                snprintf(
                    row,
                    sizeof(row),
                    "ROW|%u|%lu,%s,%s,%s,%.2f,%.2f,%.2f,%s,%u,%.3f,%.3f,%.3f,%u,%.0f,%.0f,%u,%u,%lu,%u,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%lu,%lu",
                    (unsigned int)(++rowIndex),
                    (unsigned long)rec.id,
                    date,
                    rec.mode == V3_MODE_DUAL ? "dual" : "mono",
                    rec.reserved2[0] == V3_READING_INTERMEDIATE ? "ek" : "tk",
                    (double)rec.officialZWh / 1000.0,
                    (double)rec.officialZ1Wh / 1000.0,
                    (double)rec.officialZ2Wh / 1000.0,
                    timeToken,
                    rec.mappingValid ? 1U : 0U,
                    (double)rec.anchorImportWh / 1000.0,
                    (double)rec.anchorZ1ImportWh / 1000.0,
                    (double)rec.anchorZ2ImportWh / 1000.0,
                    (unsigned int)rec.reserved2[1],
                    (rec.reserved2[2] & V3_EK_NET_SNAPSHOT) ? (double)static_cast<int64_t>(rec.spanImportWh) : (double)rec.spanImportWh,
                    (rec.reserved2[2] & V3_EK_NET_SNAPSHOT) ? (double)static_cast<int64_t>(rec.spanZ1Wh) : (double)rec.spanZ1Wh,
                    rec.reserved2[0] != V3_READING_INTERMEDIATE || rec.id == v3LatestIntermediateId ? 1U : 0U,
                    (rec.reserved2[2] & V3_EK_NET_SNAPSHOT) ? 1U : 0U,
                    (unsigned long)(pointValid ? point.epoch : 0),
                    (unsigned int)(pointValid ? point.mappingSource : 0),
                    (double)point.importWh / 1000.0,
                    (double)point.exportWh / 1000.0,
                    (double)point.z1ImportWh / 1000.0,
                    (double)point.z1ExportWh / 1000.0,
                    (double)point.z2ImportWh / 1000.0,
                    (double)point.z2ExportWh / 1000.0,
                    (unsigned long)(pointValid ? point.continuityGeneration : 0),
                    (unsigned long)(pointValid ? point.sampleEpoch : 0)
                );

                mqttClient.publish(MQTT_ADMIN_CSV_TOPIC, row, false);
                mqttClient.loop();
                yield();
            }
            f.close();
        }
    }

    mqttClient.publish(MQTT_ADMIN_CSV_TOPIC, "END", false);

    char response[112];
    snprintf(response, sizeof(response),
             "{\"ok\":true,\"cmd\":\"utility_readings_list\",\"rows\":%u}",
             (unsigned int)count);
    publishMqttAdminResponse(response);
}

void processV3TariffModeSet() {
    bool changed = dualZoneEnabled != mqttAdminTariffDual;
    dualZoneEnabled = mqttAdminTariffDual;

    if (changed) {
        // v3 tariff mode is presentation/configuration only.
        // Absolute Import/Export and continuous Z1/Z2 accumulators are never reset.
        saveV3State();
        if (v3State.rawBaselineValid && v3State.lastEpoch >= 1600000000UL) {
            appendV3Checkpoint(v3State.lastEpoch);
        }
    }

    saveV3TariffMode();
    refreshV3EstimateView();

    char response[128];
    snprintf(response, sizeof(response),
             "{\"ok\":true,\"cmd\":\"tariff_mode_set\",\"mode\":\"%s\"}",
             dualZoneEnabled ? "dual" : "mono");
    publishMqttAdminResponse(response);
    publishMqttMeasurements();
}

#if METER_TYPE == METER_DDS238
void displayDDS238(uint8_t id, bool ok) {
    if (!ok) queueOledDiagnostic(OLED_DIAG_METER, "DDS238 READ FAILED", "No Modbus response; retrying");
    if (drawOledDiagnostic(millis())) return;
    cs();
    if (ok) {
        const char* powerLabel = power < 0.0f ? "E" : "P";
        const char* wifiSymbol = WiFi.status() == WL_CONNECTED ? "&" : "o";
        doublePrintf(420, 1, 1, "%s%s=%.0fW", wifiSymbol, powerLabel, fabs(power));
        doublePrintf(420, 2, 1, "I=%.1f E=%.1fkWh",
                     (double)v3RawImportWh / 1000.0,
                     (double)v3RawExportWh / 1000.0);

        const uint8_t displayMode = dualZoneEnabled ? V3_MODE_DUAL : V3_MODE_MONO;
        const bool displayEstimate =
            v3EstimateUtilityValid && v3EstimatePointValid &&
            v3EstimateUtility.mode == displayMode &&
            v3EstimatePoint.continuityGeneration == v3State.continuityGeneration;
        const bool displayReference =
            v3LatestUtilityValid && v3LatestUtility.mode == displayMode;
        if (displayEstimate) {
            const float displayNow = dualZoneEnabled
                ? tariffZ1Now + tariffZ2Now : tariffZNow;
            doublePrintf(420, 3, 1, "Now=%.2fkWh", displayNow);
        } else {
            doublePrintf(420, 3, 1, "Now=--");
        }
        if (displayReference) {
            const float displayRef = dualZoneEnabled
                ? tariffZ1Reference + tariffZ2Reference : tariffZReference;
            doublePrintf(420, 4, 1, "DEH=%.0fkWh", displayRef);
        } else {
            doublePrintf(420, 4, 1, "DEH=--");
        }
    } else {
        doublePrintf(420, 1, 1, "DDS238 ID=%u", id);
        doublePrintf(420, 2, 1, "Not found");
    }
}
#else
void displayJSY(bool ok) {
    if (!ok) queueOledDiagnostic(OLED_DIAG_METER, "JSY READ FAILED", "No valid Modbus response; retrying");
    if (drawOledDiagnostic(millis())) return;
    cs();
    if (ok) {
        char line[32];
        platformDisplay.font(DisplayFont::Medium6x12);
        snprintf(line, sizeof(line), "V1=%.0f V2=%.0f V3=%.0f",
                 voltage1, voltage2, voltage3);
        platformDisplay.text(0, 12, line);

        platformDisplay.font(DisplayFont::Small5x7);
        snprintf(line, sizeof(line), "%c1=%.1f %c2=%.1f %c3=%.1f",
                 jsyReverse1 ? 'i' : 'I', fabs(current1),
                 jsyReverse2 ? 'i' : 'I', fabs(current2),
                 jsyReverse3 ? 'i' : 'I', fabs(current3));
        platformDisplay.text(0, 28, line);

        snprintf(line, sizeof(line), "%c1=%.0f %c2=%.0f %c3=%.0f",
                 jsyReverse1 ? 'E' : 'P', fabs(power1),
                 jsyReverse2 ? 'E' : 'P', fabs(power2),
                 jsyReverse3 ? 'E' : 'P', fabs(power3));
        platformDisplay.text(0, 44, line);

        platformDisplay.font(DisplayFont::Small5x7);
        snprintf(line, sizeof(line), "%c F=%.1f R=%.1fkWh",
                 WiFi.status() == WL_CONNECTED ? 'W' : 'o', totalEnergyFwd, totalEnergyRev);
        platformDisplay.text(0, 60, line);
        platformDisplay.present();
    } else {
        doublePrintf(420, 1, 1, "JSY-MK-333 ID=%u", ID);
        doublePrintf(420, 2, 1, "Not found");
    }
}
#endif



bool applicationStageCommand(const String& adminMessage) {
        if (adminMessage.startsWith("utility_reading_add|")) {
            if (!stageV3UtilityReadingCommand(adminMessage, false)) {
                mqttAdminCommand = MQTT_ADMIN_INVALID;
                mqttAdminCommandPending = true;
            }
            return true;
        }

        if (adminMessage.startsWith("utility_reading_update|")) {
            if (!stageV3UtilityReadingCommand(adminMessage, true)) {
                mqttAdminCommand = MQTT_ADMIN_INVALID;
                mqttAdminCommandPending = true;
            }
            return true;
        }

        if (adminMessage.startsWith("utility_reading_delete|")) {
            String rawId = adminMessage.substring(strlen("utility_reading_delete|"));
            rawId.trim();
            mqttAdminUtilityId = (uint32_t)strtoul(rawId.c_str(), nullptr, 10);
            mqttAdminCommand = mqttAdminUtilityId > 0
                ? MQTT_ADMIN_UTILITY_DELETE
                : MQTT_ADMIN_INVALID;
            mqttAdminCommandPending = true;
            return true;
        }

        if (adminMessage.equalsIgnoreCase("utility_readings_list")) {
            mqttAdminCommand = MQTT_ADMIN_UTILITY_LIST;
            mqttAdminCommandPending = true;
            return true;
        }

        if (adminMessage.startsWith("history_get|")) {
            mqttAdminCommand = stageV5History(adminMessage) ? MQTT_ADMIN_HISTORY : MQTT_ADMIN_INVALID;
            mqttAdminCommandPending = true;
            return true;
        }

        if (adminMessage.startsWith("tariff_mode_set|")) {
            String mode = adminMessage.substring(strlen("tariff_mode_set|"));
            mode.trim();

            if (mode.equalsIgnoreCase("dual")) {
                mqttAdminTariffDual = true;
                mqttAdminCommand = MQTT_ADMIN_TARIFF_MODE_SET;
            } else if (mode.equalsIgnoreCase("mono")) {
                mqttAdminTariffDual = false;
                mqttAdminCommand = MQTT_ADMIN_TARIFF_MODE_SET;
            } else {
                mqttAdminCommand = MQTT_ADMIN_INVALID;
            }
            mqttAdminCommandPending = true;
            return true;
        }

    return false;
}

bool applicationProcessCommand(MqttAdminCommand command) {
    switch (command) {
        case MQTT_ADMIN_UTILITY_ADD:
            processV3UtilityReading(false);
            break;

        case MQTT_ADMIN_UTILITY_UPDATE:
            processV3UtilityReading(true);
            break;

        case MQTT_ADMIN_UTILITY_DELETE:
            processV3UtilityDelete();
            break;

        case MQTT_ADMIN_UTILITY_LIST:
            processV3UtilityList();
            break;

        case MQTT_ADMIN_TARIFF_MODE_SET:
            processV3TariffModeSet();
            break;

        case MQTT_ADMIN_HISTORY:
            beginV5HistoryPage();
            break;

        default: return false;
    }
    return true;
}

void applicationResetCommand() {
    mqttAdminUtilityId = 0;
    mqttAdminUtilityDual = false;
    mqttAdminUtilityExactTime = false;
    mqttAdminUtilityMinute = 0;
    mqttAdminUtilityDate[0] = '\0';
    mqttAdminUtilityZ = 0.0f;
    mqttAdminUtilityZ1 = 0.0f;
    mqttAdminUtilityZ2 = 0.0f;
}

void applicationInitializeTopics() {
    snprintf_P(V5_HISTORY_TOPIC, sizeof(V5_HISTORY_TOPIC), PSTR("home/energy/%s/admin/history"), mqttDeviceId);
}

bool applicationAppendDebug(WiFiClient& client) {
    char line[256];
    int n = 0;
    FSInfo fsInfo = {};
    bool fsInfoOk = littleFsReady && LittleFS.info(fsInfo);
    n = snprintf_P(line, sizeof(line), PSTR("\n[LittleFS / V5 History]\n"
        "Mounted/info available: %s\nTotal bytes: %lu\nUsed bytes: %lu\nFree bytes: %lu\n"
        "Daily record bytes: %u; retention days: %lu\nHistory storage failure: %s\n"),
        fsInfoOk ? "YES" : "NO", (unsigned long)fsInfo.totalBytes,
        (unsigned long)fsInfo.usedBytes, (unsigned long)(fsInfo.totalBytes - fsInfo.usedBytes),
        (unsigned)sizeof(V5Day), (unsigned long)V5_DAYS, v5StorageFailed ? "YES" : "NO");
    if (n <= 0 || n >= (int)sizeof(line)) return false;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return false;
    n = snprintf_P(line, sizeof(line), PSTR("MQTT connected during this snapshot: %s\n"
        "Lowest free heap with MQTT connected this boot: %lu\n"
        "Lowest largest block with MQTT connected this boot: %lu\n"
        "V5 history start epoch: %lu\n"), mqttClient.connected() ? "YES" : "NO",
        (unsigned long)v5MqttMinHeap, (unsigned long)v5MqttMinBlock,
        (unsigned long)v5History.startEpoch);
    if (n <= 0 || n >= (int)sizeof(line)) return false;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return false;

    return true;
}

void applicationInitialize() {

    cs();
    doublePrintf(420, 1, 1, "V3 Energy Init");
    bootCheckpoint(CRASH_STAGE_BOOT_EEPROM);
    EEPROM.begin(128);
    bootCheckpoint(CRASH_STAGE_BOOT_CLEAN_START);
    initializeV3CleanStart();
    bootCheckpoint(CRASH_STAGE_BOOT_TARIFF);
    loadV3TariffMode();
    bootCheckpoint(CRASH_STAGE_BOOT_STATE);
    loadV3State();
    loadV5History();
    bootCheckpoint(CRASH_STAGE_BOOT_UTILITY);
    refreshV3LatestUtility();
    bootCheckpoint(CRASH_STAGE_BOOT_ESTIMATE);
    refreshV3EstimateView();

    bootCheckpoint(CRASH_STAGE_BOOT_SERIAL);
    beginMeasurementDriver();
}



bool applicationReadMeasurements() {
    EnergySnapshot sample;
    if (!readMeasurementDriver(sample)) return false;
    voltage = sample.voltage; current = sample.current; power = sample.power;
    pf = sample.pf; freq = sample.freq;
    totalEnergyFwd = sample.totalEnergyFwd; totalEnergyRev = sample.totalEnergyRev;
#if METER_TYPE == METER_JSY_MK333
    voltage1 = sample.voltage1; voltage2 = sample.voltage2; voltage3 = sample.voltage3;
    current1 = sample.current1; current2 = sample.current2; current3 = sample.current3;
    power1 = sample.power1; power2 = sample.power2; power3 = sample.power3;
    totalPowerJSY = sample.totalPowerJSY;
    pf1 = sample.pf1; pf2 = sample.pf2; pf3 = sample.pf3;
    jsyPowerDirection = sample.jsyPowerDirection;
    jsyReverse1 = sample.jsyReverse1; jsyReverse2 = sample.jsyReverse2;
    jsyReverse3 = sample.jsyReverse3; jsyReverseTotal = sample.jsyReverseTotal;
#endif
    updateV3Accounting(sample.rawImportWh, sample.rawExportWh);
    return true;
}

void applicationRenderMeasurements(bool ok) {
#if METER_TYPE == METER_DDS238
    displayDDS238(ID, ok);
#else
    displayJSY(ok);
#endif
}
bool applicationDualZoneEnabled() { return dualZoneEnabled; }
void applicationBackground(unsigned long now) { serviceV5History(now); }
void applicationFormatSummary(char* out, size_t capacity) {
    snprintf(out, capacity, "Meter active P=%.0fW", (double)power);
}
// END APPLICATION IMPLEMENTATION

#if !PLATFORM_CUSTOM_APPLICATION
void applicationInitialize() {}
void applicationInitializeTopics() {}
bool applicationReadMeasurements() { return true; }
bool applicationDualZoneEnabled() { return false; }
void applicationBackground(unsigned long) {}
bool applicationStageCommand(const String&) { return false; }
bool applicationProcessCommand(MqttAdminCommand) { return false; }
void applicationResetCommand() {}
bool applicationAppendDebug(WiFiClient&) { return true; }
void applicationFormatSummary(char* out, size_t capacity) { snprintf(out, capacity, "READY"); }

void applicationRenderMeasurements(bool) {
    if (drawOledDiagnostic(millis())) return;
    char stamp[32]; formatCurrentDateTime(stamp, sizeof(stamp));
    const char* clock = strlen(stamp) > 11 ? stamp + 11 : "--:--:--";
    platformDisplay.clear();
    platformDisplay.font(DisplayFont::Normal10);
    platformDisplay.text(39, 27, "READY");
    platformDisplay.font(DisplayFont::Medium6x12);
    platformDisplay.text(40, 47, clock);
    platformDisplay.present();
}
bool publishMqttSample() {
    char stamp[32]; formatCurrentDateTime(stamp, sizeof(stamp));
    char payload[256];
    int n = snprintf(payload, sizeof(payload), "{\"device\":\"%s\",\"firmware\":\"%s\",\"status\":\"ready\",\"datetime\":\"%s\"}", mqttDeviceId, VERSION, stamp);
    return n > 0 && n < (int)sizeof(payload) && mqttClient.publish(MQTT_STATE_TOPIC, payload, false);
}
bool applicationWebLiveUpdates() { return true; }
bool applicationRenderWebContent(WiFiClient& client) {
    PGM_P ready = PSTR("<h1>READY</h1>");
    return writeRootProgmemViaRam(client, ready, strlen_P(ready));
}
#endif

// ==========================================================
// MODULE: MAIN SETUP / LOOP
// ==========================================================

// =====================================================
// SETUP / LOOP EXECUTION MODEL
//
// setup():
//   reset forensics -> OLED/button -> LittleFS/settings -> accounting/history
//   -> selected meter driver -> Web/MQTT configuration -> non-blocking Wi-Fi.
//
// loop():
//   1. due physical meter read + accounting + OLED
//   2. setup/recovery controls and Wi-Fi maintenance
//   3. Web -> MQTT -> Web network service
//   4. queued MQTT response/admin work
//   5. health/history background work and diagnostics
//
// Η σειρά είναι σκόπιμη: το όργανο πρέπει να συνεχίζει να μετρά ακόμη και
// όταν router, DNS, broker ή TLS δεν είναι διαθέσιμα.
// =====================================================
void setup() {
#if PLATFORM_LOCAL_CONTROL_APPLICATION
    // GPIO actuators must be safe before display, filesystem or network setup.
    applicationEarlyInitialize();
#endif
    // Read the SDK reset structure before any network or sensor startup.
    captureCurrentReset();
    loadRtcCrashTrace();
    loadOledDiagnostic();
    initializeMqttIdentity();
    const bool externalReset = (currentBootReset.reason == REASON_EXT_SYS_RST);
    const bool hardwareOrWatchdogFault = isFaultReset(currentBootReset.reason);

    bootCheckpoint(CRASH_STAGE_BOOT_OLED_INIT);
    platformDisplay.begin();
    bootOledReady = true; // Display service initialized, including no-display mode.
    configureFlashSetupButton(); // FLASH also works in Recovery and setup-only boot.
#if OLED_TYPE == 2
    platformDisplay.contrast(255);
#endif

    // Start LittleFS early so an incident can be latched across power cycles.
    // Existing v1.20 energy/tariff/daily EEPROM addresses are untouched.
    bootCheckpoint(CRASH_STAGE_BOOT_FILESYSTEM);
    littleFsReady = LittleFS.begin();
    if (!littleFsReady) queueOledDiagnostic(OLED_DIAG_FILESYSTEM, "LITTLEFS FAILED", "Settings/history storage could not be mounted");

    // Credentials are stored in one validated LittleFS record. Meter profile
    // metadata is no longer part of credential loading.
    bool deviceCredentialsReady = false;
    if (littleFsReady) {
        bootCheckpoint(CRASH_STAGE_BOOT_CREDENTIALS);
#if PLATFORM_LOCAL_CONTROL_APPLICATION
        if (!LittleFS.exists(DEVICE_CREDENTIAL_FILE)) applicationImportLegacyCredentials();
#endif
        deviceCredentialsReady = loadDeviceCredentials();
        if (!deviceCredentialsReady) queueOledDiagnostic(OLED_DIAG_CREDENTIALS, "SAVED SETTINGS INVALID", "Settings missing or invalid; use setup AP");
    }

    bootCheckpoint(CRASH_STAGE_BOOT_RECOVERY_CHECK);
    bool hasPreviousIncident = loadRecoveryIncident();

    if (externalReset) {
        bootCheckpoint(CRASH_STAGE_BOOT_RECOVERY_CLEAR);
        // External RST is one explicit way out of a crash recovery latch.
        // The Recovery-only /restart-normal action and successful OTA can
        // also clear the persistent incident deliberately.
        bool clearSucceeded = true;
        if (littleFsReady && LittleFS.exists(RECOVERY_FILE)) {
            clearSucceeded = LittleFS.remove(RECOVERY_FILE);
        }
        if (clearSucceeded) {
            recoveryIncident = RecoveryRecord();
            recoveryIncidentPersistent = false;
            hasPreviousIncident = false;
            recoveryMode = false;
        } else {
            // Fail closed on filesystem deletion errors: a stale fault
            // record must not silently unlock normal metering.
            recoveryResetClearFailed = true;
            queueOledDiagnostic(OLED_DIAG_RECOVERY_CLEAR, "RECOVERY CLEAR FAILED", "Stored Recovery could not be removed");
            recoveryMode = hasPreviousIncident;
        }
    } else if (hardwareOrWatchdogFault && !hasPreviousIncident) {
        recoveryIncident = currentBootReset;
        recoveryMode = true;
        saveRecoveryIncident();
    } else if (hasPreviousIncident) {
        recoveryMode = true;
    }

    if (recoveryMode) {
        // Fail closed: no meter, MQTT, EEPROM accounting or normal admin
        // initialization. Only LCD, Wi-Fi, OTA and read-only HTTP debug.
        showRecoveryScreen();
        configureRecoveryRoutes();
        if (deviceCredentialsReady) {
            connectWiFiInitial(true);
            if (WiFi.status() == WL_CONNECTED) startNetworkServices();
            else startCredentialAccessPoint(true);
        } else {
            startCredentialAccessPoint(true);
        }
        showRecoveryScreen();
        finishBootCheckpoints(CRASH_STAGE_BOOT_RECOVERY_IDLE);
        return;
    }

#if PLATFORM_LOCAL_CONTROL_APPLICATION
    // Local automation also runs without saved Wi-Fi credentials, in setup AP.
    applicationInitialize();
    normalMeterInitialized = true;
#endif
    if (!deviceCredentialsReady) {
        configureProvisioningRoutes();
        startCredentialAccessPoint(true);
        finishBootCheckpoints(credentialPortalActive ? CRASH_STAGE_BOOT_AP_IDLE : CRASH_STAGE_BOOT_AP_FAILED);
        return;
    }

    markCrashStage(CRASH_STAGE_SETUP, ESP.getFreeHeap());
    cs();
    doublePrintf(420, 1, 1, "Initialize...");
    doublePrintf(420, 2, 1, "Ver: %s", VERSION);
    doublePrintf(420, 3, 1, "Date: %s", BUILD_DATE);
    // Checkpoints replace the old five-second cosmetic startup delay.

#if !PLATFORM_LOCAL_CONTROL_APPLICATION
    applicationInitialize();
#endif

    bootCheckpoint(CRASH_STAGE_BOOT_WEB_CONFIG);
    configureWebRoutes();
    bootCheckpoint(CRASH_STAGE_BOOT_MQTT_CONFIG);
    configureMqtt();
    normalMeterInitialized = true;

    markCrashStage(CRASH_STAGE_WIFI_CONNECT, ESP.getFreeHeap());
    connectWiFiInitial(false);

    if (WiFi.status() == WL_CONNECTED) {
        bootCheckpoint(CRASH_STAGE_NETWORK_SERVICES);
        startNetworkServices();
        doublePrintf(420, 3, 1, "Wifi connected!");
        const IPAddress ip = WiFi.localIP();
        char ipText[16];
        snprintf(ipText, sizeof(ipText), "%u.%u.%u.%u",
                 ip[0], ip[1], ip[2], ip[3]);
        doublePrintf(420, 4, 1, "%s", ipText);
    } else {
        doublePrintf(420, 3, 1, "Offline / local meter");
        wifiOfflineSinceMs = millis();
    }

    bootCheckpoint(CRASH_STAGE_BOOT_LOOP_ENTER);
}

// Retry indefinitely without reboot while router/Internet is unavailable.

// =====================================================
// NETWORK PRIORITY SERVICE
// Προτεραιότητα: Web -> MQTT -> Web ξανά.
// Καλείται περισσότερες από μία φορές ανά loop και πάντα
// μετά από την τοπική μέτρηση που είναι ήδη due.
// =====================================================
void serviceNetworkPriority(unsigned long now) {
    if (credentialPortalOnly || WiFi.status() != WL_CONNECTED) return;
    // A router may return long after an offline boot. Diagnose that FIRST
    // MQTT attempt too, rather than lose it when local startup already finished.
    if (!bootFirstMqttAttempted && mqttSettingsReady()) bootCheckpointsActive = true;

    if (credentialPortalActive && !credentialPortalOnly) {
        stopCredentialAccessPoint();
    }

    bootCheckpoint(CRASH_STAGE_NETWORK_SERVICES);
    startNetworkServices();

    // 1. Web πρώτα, ώστε η σελίδα να απαντά όσο πιο άμεσα γίνεται.
    bootCheckpoint(CRASH_STAGE_BOOT_FIRST_WEB);
    server.handleClient();
    if (credentialRestartPending) return;

    // 2. MQTT αμέσως μετά: keepalive, incoming requests, reconnect.
    maintainMqtt(now);

    // 3. Δεύτερη ευκαιρία στο Web μετά από πιθανό MQTT reconnect.
    server.handleClient();
    if (credentialRestartPending) return;

}

void loop() {
    unsigned long now = millis();
    if (serviceCredentialRestart(now)) return;

    if (mqttAdminRestartPending &&
        now - mqttAdminRestartRequestedMs >= MQTT_ADMIN_RESTART_DELAY_MS) {
        mqttAdminRestartPending = false;
        if (mqttClient.connected()) mqttClient.disconnect();
        mqttNet.stop(0);
        delay(20);
#if PLATFORM_LOCAL_CONTROL_APPLICATION
        applicationPrepareOta();
#endif
        ESP.restart();
        return;
    }

    if (forceRecoveryRestartPending &&
        now - forceRecoveryRestartRequestedMs >= FORCE_RECOVERY_RESTART_DELAY_MS) {
        delay(20);
        ESP.restart();
        return;
    }

    if (recoveryMode) {
        serviceFlashSetupButton(now);
        if (credentialPortalActive || credentialPortalOnly) serviceCredentialPortal(now);
        serviceRecoveryOnly(now);
        serviceOledDiagnostic(millis());
        return;
    }

    if (credentialPortalOnly && !normalMeterInitialized) {
        serviceCredentialPortal(now);
        serviceOledDiagnostic(millis());
        yield();
        return;
    }

#if PLATFORM_LOCAL_CONTROL_APPLICATION
    applicationServiceLocal(now);
#endif

    bool meterDisplayUpdated = false;
    // LOCAL METER FIRST: neither a missing router nor a slow broker may defer
    // the first read/display behind a synchronous network connection attempt.
    if (!bootFirstMeterDone || now - lastRead >= interval) {
        lastRead = now;
        bootCheckpoint(CRASH_STAGE_BOOT_FIRST_METER);
        lastMeterReadOk = readMeterWithHealth();
        bootCheckpoint(CRASH_STAGE_BOOT_FIRST_DISPLAY);
        applicationRenderMeasurements(lastMeterReadOk);
        bootFirstMeterDone = true;
        meterDisplayUpdated = true;
    }
    serviceFlashSetupButton(millis());
    if (credentialPortalActive || credentialPortalOnly) serviceCredentialPortal(millis());
    if (serviceNetworkStackRecovery(millis())) {
        serviceOledDiagnostic(millis());
        yield();
        return;
    }
    serviceWiFiReconnect(millis());

    // =====================================================
    // Network service after the due local measurement.
    // =====================================================
    serviceNetworkPriority(millis());

    if (!credentialPortalOnly && WiFi.status() == WL_CONNECTED) {

        // MQTT update: φρέσκια μέτρηση μόνο όταν ζητηθεί.
        if (mqttUpdateResponseDue(millis())) {
            mqttUpdateRequested = false;

            // Reuse the just-completed local read rather than repeating a timeout.
            if (!meterReadAttempted || millis() - lastRead >= interval) {
                lastRead = millis();
                readMeterWithHealth();
            }
            // Rate-limit replies, not acceptance of requests. Failed publishes
            // stay pending, but use 5/10/30 s retry backoff so a low-memory or
            // broken transport condition cannot continuously pressure TCP/TLS.
            const bool updatePublished = publishMqttMeasurements();
            if (!updatePublished) {
                mqttUpdateRequested = true;
                scheduleMqttUpdateRetry();
            } else {
                resetMqttUpdateRetry();
            }
            lastMqttUpdatePublishMs = millis();

            // Αμέσως μετά το Modbus read/publish εξυπηρετούμε πάλι δίκτυο.
            serviceNetworkPriority(millis());
        }

        // MQTT admin commands.
        if (mqttAdminCommandPending) {
            processMqttAdminCommand();

            // Τα admin commands μπορεί να κάνουν meter read / EEPROM / FS.
            // Επιστρέφουμε αμέσως σε Web/MQTT πριν συνεχίσουμε.
            serviceNetworkPriority(millis());
        }
    }

    // =====================================================
    // BACKGROUND HEALTH / APPLICATION SERVICE
    // =====================================================
    serviceMeterHealth(millis());
    applicationBackground(millis());

    // Application-specific storage/history work runs in applicationBackground().

    if (bootCheckpointsActive && bootFirstMeterDone &&
        (WiFi.status() != WL_CONNECTED || bootFirstMqttAttempted || !mqttSettingsReady())) {
        bootCheckpoint(CRASH_STAGE_BOOT_FIRST_DISPLAY);
        applicationRenderMeasurements(lastMeterReadOk); // Clear a completed network CP frame.
        finishBootCheckpoints(CRASH_STAGE_BOOT_RUNNING);
        meterDisplayUpdated = true;
    }
    serviceWiFiStatusDisplay(millis(), meterDisplayUpdated);
    serviceOledDiagnostic(millis());

    yield();
}

// ==========================================================
// END MODULE: MAIN SETUP / LOOP
// ==========================================================
