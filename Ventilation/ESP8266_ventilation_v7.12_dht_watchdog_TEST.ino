// Ventilation v7.12 DHT11 watchdog diagnostic candidate from verified 7.11 source; unverified hardware.
// Original fan safety logic, GPIO mapping and offline-first scheduling preserved.
// ESP8266_common_platform - central editable shared firmware.
// Application generation inserts only the selected application's declarations,
// implementation and profile at the marked integration points below.
// Baseline: EnergyMeter 5.58 behavior, cleaned source commit 0c874d3 (5.59).
// OLED hardware map for these installations (128x64 I2C):
//   OLED_TYPE 3 = CH1116 1.54-inch; uses U8g2 SSD1309 initialization.
//   OLED_TYPE 2 = SSD1309 1.3-inch; OLED_TYPE 0 = no physical OLED.
// Existing releases keep their original OLED_TYPE, version and OTA identity.
// BEGIN PLATFORM PROFILE
#define PLATFORM_CUSTOM_APPLICATION 1
#define PLATFORM_NAME "ESP8266_common_platform"
#define PLATFORM_VERSION "1.06"
#define VERSION "7.12"
#define BUILD_DATE "09/11/2026"
#define DEVICE_LABEL "Ventilation"
#define APPLICATION_ID "ventilation"
#define DEVICE_ID "vent"
#define DEVICE_KIND "VENTILATION"
#define DRIVER_ID "DS18B20-DHT11"
#define OTA_PROFILE_PREFIX "ventilation-3ds18b20-2dht11"
#define MQTT_TOPIC_PREFIX "home/energy/"
#define OLED_TYPE 0
#define APP_SAMPLE_INTERVAL_MS 2500UL
#define SITE_IP_D 81
#define PLATFORM_LOCAL_CONTROL_APPLICATION 1
#define PLATFORM_FLASH_BUTTON_ENABLED 0
#define PLATFORM_SENSOR_ERROR "sensor_invalid"
#define PLATFORM_WEB_MIN_HEAP_BYTES 2048UL
#define PLATFORM_WEB_MIN_BLOCK_BYTES 512U
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
#elif OLED_TYPE == 3
#define OLED_ID "CH1116"
#define OTA_PROFILE_ID OTA_PROFILE_PREFIX "-ch1116"
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
struct LegacyVentCredentialRecord; // Required by Arduino CLI auto-prototype generation.
struct RtcCrashTrace;
struct RecoveryRecord;
struct OledDiagnostic;
enum class DisplayFont : uint8_t;
enum MqttAdminCommand : uint8_t;

// BEGIN APPLICATION DECLARATIONS
// Ventilation application 2.00: hardware/domain state only.
// Extracted from the user-supplied Ventilation 1.07 source.
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <DHTesp.h>
#include <time.h>
#include <math.h>
#include <cmath>
#include <ctype.h>

static const uint8_t GPIO_1WIRE=4;    // DS18B20 bus
static const uint8_t GPIO_DHT_OUT=0;  // DHT11 outside, boot-strap pin GPIO0
static const uint8_t GPIO_DHT_IN=14;  // DHT11 inside
static const uint8_t GPIO_FAN=12;     // D6: observed installed relay ON = HIGH
static const uint8_t GPIO_SPEED=13;   // D7: observed installed HIGH SPEED = HIGH
static const uint8_t GPIO_MAINS=15;   // D8: mains-sense input, boot-strap GPIO15
static const uint8_t GPIO_RF=1;      // UART TX pin GPIO1 reused as RF switch input
static const uint8_t GPIO_LED=LED_BUILTIN;
static const uint8_t FAN_ON_LEVEL=HIGH, FAN_OFF_LEVEL=LOW; // Calibrated to installed hardware feedback
static const uint8_t SPEED_HIGH_LEVEL=HIGH, SPEED_LOW_LEVEL=LOW; // Calibrated to installed hardware feedback

static uint8_t addrOutside[8]={0x28,0xfc,0xe9,0xa7,0x04,0x00,0x00,0x89};
static uint8_t addrMiddle[8] ={0x28,0xff,0x93,0x0c,0xa4,0x16,0x05,0xb2};
static uint8_t addrInside[8] ={0x28,0xa3,0xb4,0xa7,0x04,0x00,0x00,0xfa};

OneWire oneWire(GPIO_1WIRE);
DallasTemperature ds(&oneWire);
DHT dhtOutside(GPIO_DHT_OUT,DHT11),dhtInside(GPIO_DHT_IN,DHT11);
DHTesp dhtCalculations;

// Explicit speed commands override RF until the next reboot.
bool speedOverrideActive=false;
unsigned long lastLedAt=0;
unsigned long lastSensorAt=0,lastClockTick=0,lastSnapshotAt=0;
bool sensorsOk=false;
float temperatureOutside=NAN,temperatureInside=NAN,temperatureMiddle=NAN;
// Failure counting is per normal 2.5s sensor cycle, not per Wi-Fi/MQTT request.
// Only the Outside and Inside DS18B20 are critical for fan comparison.
static const uint8_t VENT_FAIL_LIMIT=10;
static const uint8_t VENT_GOOD_FOR_REARM=3;
enum VentProbe : uint8_t {
  DS_OUTSIDE=0,DS_MIDDLE=1,DS_INSIDE=2,
  DHT_OUT_TEMP=3,DHT_OUT_HUM=4,DHT_IN_TEMP=5,DHT_IN_HUM=6,
  VENT_PROBE_COUNT=7
};
uint8_t ventProbeFails[VENT_PROBE_COUNT]={};
uint8_t ventGoodOutside=0,ventGoodInside=0;
bool ventSafeMode=false,ventSafeOutside=false,ventSafeInside=false;
void ventTrackProbe(uint8_t probe,bool valid) {
  if(valid)ventProbeFails[probe]=0;
  else if(ventProbeFails[probe]<VENT_FAIL_LIMIT)++ventProbeFails[probe];
}
bool ventTryRearmSafeMode() {
  if(!ventSafeMode)return true;
  if(!(std::isfinite(temperatureOutside)&&temperatureOutside>-55&&temperatureOutside<125)||
     !(std::isfinite(temperatureInside)&&temperatureInside>-55&&temperatureInside<125)||
     ventGoodOutside<VENT_GOOD_FOR_REARM||ventGoodInside<VENT_GOOD_FOR_REARM)
    return false;
  ventSafeMode=false;
  ventSafeOutside=false;
  ventSafeInside=false;
  return true;
}
float humidityOutside=NAN,humidityInside=NAN,dhtTemperatureOutside=NAN,dhtTemperatureInside=NAN;

bool manualMode=false,summerMode=false,manualFan=false,manualHighSpeed=false;
bool fanRunning=false,fanHighSpeed=false;
int pauseFrom=6*60+15,pauseUntil=8*60+30;
int currentHour=0,currentMinute=0,currentSecond=0,previousDayKey=-1;
uint32_t uptimeDayCount=0;
long manualTimeOffset=0;
bool timeSynchronized=false;
// One minute sample uses 1/80 C, exactly covering DS18B20 1/16 C readings
// and the 1/10 C values restored from flash, with the existing empty sentinel.
// Keep the flash records unchanged; this saves 5760 bytes of runtime RAM.
int16_t todayInside[1440],todayOutside[1440];
float yesterdayInside[96],yesterdayOutside[96];
// Compact, fixed-size, A/B flash snapshot: 2 days x 96 x 2 x int16_t
// (0.1 C resolution). No minute-by-minute flash writes.
static const uint32_t HISTORY_MAGIC=0x56313548UL;
static const uint16_t HISTORY_FORMAT=1;
static const int16_t HISTORY_EMPTY=-32768;
static const char HISTORY_A[]="/vent-history-a.bin";
static const char HISTORY_B[]="/vent-history-b.bin";
// Separate fixed-size A/B daily archive. Compatible with existing v1.06
// /vent-history-a.bin and /vent-history-b.bin (do not change their layout).
static const char DAILY_A[]="/vent-days-a.bin", DAILY_B[]="/vent-days-b.bin";
static const uint32_t DAILY_MAGIC=0x56313037UL;
static const uint16_t DAILY_FORMAT=1;
static const uint8_t DAILY_COUNT=30;
struct DailyRecord {
  int32_t dayKey;
  int16_t insideMin,insideMax,outsideMin,outsideMax; // 0.1 C
  int32_t insideSum,outsideSum;                       // sum of 0.1 C samples
  uint16_t insideCount,outsideCount;                  // 1-min valid samples
};
struct DailyArchive {
  uint32_t magic,sequence;
  uint16_t format,count;
  DailyRecord days[30];
  uint32_t checksum;
};
static DailyArchive dailyArchive; // ~736 bytes, not allocated on stack
bool dailyLoaded=false,dailyActiveA=false;
uint32_t dailyWrites=0,dailyWriteErrors=0;
struct HistorySnapshot {
  uint32_t magic,sequence;
  uint16_t format,reserved;
  int32_t dayKey,slotKey;
  uint32_t savedEpoch;
  int16_t currentInside[96],currentOutside[96],previousInside[96],previousOutside[96];
  uint32_t checksum;
};
static HistorySnapshot historyDisk; // one shared buffer, never stack-allocate
uint32_t historySequence=0,historySavedAt=0,historyWrites=0,historyWriteFailures=0;
int lastLoggedMinute=-1;
int32_t historyLastSlotKey=-1;
bool historyActiveSlotA=false,historyLoaded=false;
int historyLastAttemptMinute=-1;
int32_t historyPendingRetrySlot=-1;
bool manualClockProvided=false;

bool commandReady=false,historyStreaming=false;
bool ventilationCommandInvalid=false;
char commandBuf[480]={};
char historyRange[9]={};
uint8_t historyOffset=0;
char VENT_HISTORY_TOPIC[sizeof(MQTT_TOPIC_PREFIX) + sizeof(DEVICE_ID) + 12 + sizeof("/vent/history") - 1] = {};
uint32_t ventilationStatePublishFailures=0;
bool ventilationInitialized=false,ventilationOtaPaused=false;

// Credential-only compatibility adapter; networking/storage stay in the platform.
struct LegacyVentCredentialRecord {
  uint32_t magic;
  uint16_t version;
  char ssid[33];
  char wifiPassword[65];
  char mqttUser[65];
  char mqttPassword[65];
  uint32_t checksum;
};
// END APPLICATION DECLARATIONS

// Display backend for the generated, single-translation-unit firmware sketch.
// OLED_TYPE: 2=SSD1309 1.3-inch, 3=CH1116 1.54-inch (U8g2 SSD1309 init), 0=headless.
// No U8g2 code or framebuffer is included in a headless build.
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#ifndef OLED_TYPE
  #error "Set OLED_TYPE to 2 (SSD1309), 3 (CH1116) or 0 (headless) in the device profile"
#endif

enum class DisplayFont : uint8_t {
    Tiny4x6,
    Small5x7,
    Medium6x12,
    Normal10
};

#if OLED_TYPE == 2 || OLED_TYPE == 3
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
    // For CH1116 (OLED_TYPE 3), the SSD1309 U8g2 init is the tested-compatible backend.
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
  #error "Invalid OLED_TYPE: use 2 (SSD1309), 3 (CH1116) or 0 (headless)"
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

// MQTT transport failure must not force STA Wi-Fi down or stop the HTTP server.
// Retry cooldown is isolated from local sensors and fan control.
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
    15000UL, 30000UL, 60000UL
};
// Protect local control from rapid TLS/heap retry storms.
const unsigned long MQTT_TRANSPORT_COOLDOWN_MS = 60000UL;
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
    CRASH_STAGE_MQTT_TRANSPORT_RESET = 58,
    CRASH_STAGE_BOOT_MQTT_DNS = 59,
    CRASH_STAGE_MFLN_PROBE = 60,
    CRASH_STAGE_MQTT_CONNECT = 61,
    CRASH_STAGE_MQTT_TLS_CALL = 62,
    CRASH_STAGE_MQTT_TLS_RETURN = 63,
    CRASH_STAGE_MQTT_POST_CONNECT = 64,
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
    CRASH_STAGE_BOOT_OLED_WRITE = 100,
    CRASH_STAGE_VENT_DS18B20 = 101,
    CRASH_STAGE_VENT_DHT11 = 102,
    CRASH_STAGE_VENT_HISTORY_STREAM = 103,
    CRASH_STAGE_VENT_HISTORY_SAVE = 104,
    CRASH_STAGE_VENT_SAMPLE_PUBLISH = 105,
    CRASH_STAGE_VENT_DHT_OUTSIDE = 106,
    CRASH_STAGE_VENT_DHT_INSIDE = 107,
    CRASH_STAGE_TRACE_COMPLETED = 110
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

// RTC records stay in RTC memory (no LittleFS writes in hot paths).
// If the watchdog fires before destructor, the active stage survives.
struct CrashTraceScope {
    uint32_t lastStage;
    CrashTraceScope(uint32_t stage, uint32_t detail) : lastStage(stage) {
        markCrashStage(stage, detail);
    }
    void enter(uint32_t stage, uint32_t detail) {
        lastStage = stage;
        markCrashStage(stage, detail);
    }
    ~CrashTraceScope() {
        markCrashStage(CRASH_STAGE_TRACE_COMPLETED, lastStage);
    }
    CrashTraceScope(const CrashTraceScope&) = delete;
    CrashTraceScope& operator=(const CrashTraceScope&) = delete;
};

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
        case CRASH_STAGE_MQTT_TRANSPORT_RESET: return "MQTT_TRANSPORT_RESET";
        case CRASH_STAGE_BOOT_MQTT_DNS: return "MQTT_DNS";
        case CRASH_STAGE_MFLN_PROBE: return "MFLN_PROBE";
        case CRASH_STAGE_MQTT_CONNECT: return "MQTT_CONNECT";
        case CRASH_STAGE_MQTT_TLS_CALL: return "MQTT_TLS_CALL";
        case CRASH_STAGE_MQTT_TLS_RETURN: return "MQTT_TLS_RETURN";
        case CRASH_STAGE_MQTT_POST_CONNECT: return "MQTT_POST_CONNECT";
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
        case CRASH_STAGE_VENT_DS18B20: return "VENT_DS18B20";
        case CRASH_STAGE_VENT_DHT11: return "VENT_DHT11"; // v7.11 legacy
        case CRASH_STAGE_VENT_DHT_OUTSIDE: return "VENT_DHT_OUTSIDE";
        case CRASH_STAGE_VENT_DHT_INSIDE: return "VENT_DHT_INSIDE";
        case CRASH_STAGE_VENT_HISTORY_STREAM: return "VENT_HISTORY_STREAM";
        case CRASH_STAGE_VENT_HISTORY_SAVE: return "VENT_HISTORY_SAVE";
        case CRASH_STAGE_VENT_SAMPLE_PUBLISH: return "VENT_SAMPLE_PUBLISH";
        case CRASH_STAGE_TRACE_COMPLETED: return "TRACE_COMPLETED";
        default: return "UNKNOWN/NONE";
    }
}

const char* crashDetailMeaning(uint32_t stage) {
    switch (stage) {
        case CRASH_STAGE_MQTT_TLS_RETURN: return "connect_elapsed_ms";
        case CRASH_STAGE_TRACE_COMPLETED: return "last_completed_stage";
        case CRASH_STAGE_MQTT_TRANSPORT_RESET:
        case CRASH_STAGE_BOOT_MQTT_DNS:
        case CRASH_STAGE_MFLN_PROBE:
        case CRASH_STAGE_MQTT_TLS_CALL:
        case CRASH_STAGE_MQTT_POST_CONNECT:
        case CRASH_STAGE_VENT_DS18B20:
        case CRASH_STAGE_VENT_DHT11:
        case CRASH_STAGE_VENT_DHT_OUTSIDE:
        case CRASH_STAGE_VENT_DHT_INSIDE:
        case CRASH_STAGE_VENT_HISTORY_STREAM:
        case CRASH_STAGE_VENT_HISTORY_SAVE:
        case CRASH_STAGE_VENT_SAMPLE_PUBLISH: return "stack_free_bytes_before";
        default: return "legacy_detail_or_heap_bytes";
    }
}


// ===== MQTT ADMIN COMMAND STATE =====
enum MqttAdminCommand : uint8_t {
    MQTT_ADMIN_NONE = 0,
    MQTT_ADMIN_REMOTE_OTA,
    MQTT_ADMIN_OTA_REJECTED,
    MQTT_ADMIN_RESTART,
    MQTT_ADMIN_VENTILATION,
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
const unsigned long WIFI_RETRY_STEPS_MS[] = {12000UL, 20000UL, 30000UL, 45000UL};
const uint8_t WIFI_RETRY_STEP_COUNT = sizeof(WIFI_RETRY_STEPS_MS) / sizeof(WIFI_RETRY_STEPS_MS[0]);
uint8_t wifiRetryStep = 0;
unsigned long lastWifiRetry = 0;
uint32_t wifiConnectAttempts = 0;
bool wifiWasConnected = false;
WiFiEventHandler wifiDisconnectHandler;
uint16_t wifiLastDisconnectReason = 0;
uint32_t wifiDisconnectEvents = 0;
uint32_t wifiLastDisconnectAtMs = 0;
// Low-water marks sampled at sensor completion and MQTT transitions.
uint32_t localMinimumHeapBytes = 0xFFFFFFFFUL;
uint16_t localMinimumBlockBytes = 0xFFFFU;
uint32_t localMinimumStackBytes = 0xFFFFFFFFUL;
void noteLocalMemoryLowWater() {
    const uint32_t heap = ESP.getFreeHeap();
    const uint16_t block = ESP.getMaxFreeBlockSize();
    const uint32_t stack = ESP.getFreeContStack();
    if (heap < localMinimumHeapBytes) localMinimumHeapBytes = heap;
    if (block < localMinimumBlockBytes) localMinimumBlockBytes = block;
    if (stack < localMinimumStackBytes) localMinimumStackBytes = stack;
}
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

bool safeFlashResponse(int statusCode,const char* type,PGM_P body);
bool safeRamResponse(int statusCode,const char* type,const char* body);

void sendApPinPage(int statusCode) {
    safeFlashResponse(statusCode, "text/html; charset=utf-8", PSTR(
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
        safeFlashResponse(404, "text/plain", PSTR("Not found"));
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

    CrashTraceScope mqttTrace(CRASH_STAGE_MQTT_TRANSPORT_RESET, ESP.getFreeContStack());

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
    mqttTrace.enter(CRASH_STAGE_BOOT_MQTT_DNS, ESP.getFreeContStack());
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
        mqttTrace.enter(CRASH_STAGE_MFLN_PROBE, ESP.getFreeContStack());
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
    mqttTrace.enter(CRASH_STAGE_MQTT_CONNECT, ESP.getFreeHeap());

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
    const uint32_t mqttConnectCallStartedMs = millis();
    mqttTrace.enter(CRASH_STAGE_MQTT_TLS_CALL, ESP.getFreeContStack());
    bool ok = mqttClient.connect(
        clientId,
        mqttUser,
        mqttPassword,
        MQTT_STATUS_TOPIC,
        0,
        true,
        "offline"
    );
    mqttTrace.enter(CRASH_STAGE_MQTT_TLS_RETURN, millis() - mqttConnectCallStartedMs);
    mqttLastConnectSucceeded = ok;
    mqttLastAttemptState = mqttClient.state();
    noteLocalMemoryLowWater();
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
        mqttTrace.enter(CRASH_STAGE_MQTT_POST_CONNECT, ESP.getFreeContStack());
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
/*
 * Compact HTTP replies for ESP8266 Core 2.7.2.
 * - No ESP8266WebServer::send()/send_P()/sendContent_P() for HTML bodies.
 * - HTTP header uses a fixed stack buffer; static content stays in flash.
 * - Body uses the existing bounded PROGMEM->RAM writer, never whole-page String.
 * - WebResponseScope remains responsible for the final TCP close.
 */
bool safeHttpHeader(WiFiClient& client,int code,const char* type,size_t len) {
    const char* reason = code==200?"OK":code==400?"Bad Request":
                         code==403?"Forbidden":code==404?"Not Found":
                         code==409?"Conflict":code==503?"Service Unavailable":
                         code==500?"Internal Server Error":"OK";
    char h[224];
    const int n=snprintf(h,sizeof(h),
        "HTTP/1.1 %d %s\r\nContent-Type: %s\r\n"
        "Content-Length: %lu\r\nCache-Control: no-store\r\n"
        "%sConnection: close\r\n\r\n",
        code,reason,type,(unsigned long)len,code==503?"Retry-After: 2\r\n":"");
    return n>0 && n<(int)sizeof(h) &&
        writeRootBytes(client,reinterpret_cast<const uint8_t*>(h),(size_t)n);
}
bool safeFlashResponse(int statusCode,const char* type,PGM_P body) {
    if(!body)return false;
    WiFiClient client=server.client();
    const size_t len=strlen_P(body);
    return safeHttpHeader(client,statusCode,type,len) &&
        writeRootProgmemViaRam(client,body,len);
}
bool safeRamResponse(int statusCode,const char* type,const char* body) {
    if(!body)return false;
    WiFiClient client=server.client();
    const size_t len=strlen(body);
    return safeHttpHeader(client,statusCode,type,len) &&
        writeRootBytes(client,reinterpret_cast<const uint8_t*>(body),len);
}

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

/* Stream each diagnostic field separately. Never allocate a second 2-4 KB
   String to assemble /debug or /debug.txt in OTA-ONLY Recovery. */
bool streamReportBytes(WiFiClient& client,const char* text,size_t length,bool escapeHtml){
    if(!text||!length)return true;
    if(!escapeHtml)
        return writeRootBytes(client,reinterpret_cast<const uint8_t*>(text),length);
    char chunk[128];
    size_t used=0;
    for(size_t i=0;i<length;++i){
        const char c=text[i];
        const char* entity=c=='&'?"&amp;":c=='<'?"&lt;":c=='>'?"&gt;":nullptr;
        const size_t n=entity?strlen(entity):1;
        if(used+n>sizeof(chunk)){
            if(!writeRootBytes(client,reinterpret_cast<const uint8_t*>(chunk),used))return false;
            used=0;
        }
        if(entity){memcpy(chunk+used,entity,n);used+=n;}
        else chunk[used++]=c;
    }
    return !used||writeRootBytes(client,reinterpret_cast<const uint8_t*>(chunk),used);
}
bool streamReportPart(WiFiClient& client,const char* part,bool escapeHtml){
    return part&&streamReportBytes(client,part,strlen(part),escapeHtml);
}
bool streamReportPart(WiFiClient& client,const String& part,bool escapeHtml){
    return streamReportBytes(client,part.c_str(),part.length(),escapeHtml);
}
bool streamRecoveryReport(WiFiClient& client, bool htmlEscape) {
    if (!streamReportPart(client, ("" DEVICE_LABEL " OTA-ONLY RECOVERY / Ver " + String(VERSION) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Measurements, MQTT, admin commands and statistics: DISABLED\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Firmware=" + String(VERSION) + " build=" + String(BUILD_DATE) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Meter=" + String(DEVICE_KIND) + " OLED=" + String(OLED_TYPE) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Incident reason=" + String(resetReasonLabel(recoveryIncident.reason)) +
         " (" + String(recoveryIncident.reason) + ")\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Incident stored in LittleFS=" +
         String(recoveryIncidentPersistent ? "YES" : "NO (current boot only)") + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("RTC crash breadcrumb="), htmlEscape)) return false;
    if (rtcTraceBeforeResetValid) {
        if (!streamReportPart(client, (String(crashStageLabel(rtcTraceBeforeReset.stage)) +
             " (" + String(rtcTraceBeforeReset.stage) + ")" +
             " detail=" + String(rtcTraceBeforeReset.detail) + "\n"), htmlEscape)) return false;
        if (!streamReportPart(client, ("RTC trace detail meaning=" +
            String(crashDetailMeaning(rtcTraceBeforeReset.stage)) + "\n"), htmlEscape)) return false;
        if (!streamReportPart(client, ("RTC trace heap free bytes=" + String(rtcTraceBeforeReset.heapFree) + "\n"), htmlEscape)) return false;
        if (!streamReportPart(client, ("RTC trace max free block bytes=" + String(rtcTraceBeforeReset.maxFreeBlock) + "\n"), htmlEscape)) return false;
        if (!streamReportPart(client, ("RTC trace heap fragmentation percent=" +
             String(rtcTraceBeforeReset.heapFragmentation) + "\n"), htmlEscape)) return false;
    } else {
        if (!streamReportPart(client, ("UNAVAILABLE\n"), htmlEscape)) return false;
    }
    if (!streamReportPart(client, ("External RST clear failed=" +
         String(recoveryResetClearFailed ? "YES" : "NO") + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Recovery exit: successful OTA, external RST pin, or /restart-normal\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Exception cause=" + String(recoveryIncident.exccause) + "\n"), htmlEscape)) return false;
    char address[20];
    snprintf(address, sizeof(address), "0x%08lX", (unsigned long)recoveryIncident.epc1);
    if (!streamReportPart(client, ("EPC1=" + String(address) + "\n"), htmlEscape)) return false;
    snprintf(address, sizeof(address), "0x%08lX", (unsigned long)recoveryIncident.epc2);
    if (!streamReportPart(client, ("EPC2=" + String(address) + "\n"), htmlEscape)) return false;
    snprintf(address, sizeof(address), "0x%08lX", (unsigned long)recoveryIncident.epc3);
    if (!streamReportPart(client, ("EPC3=" + String(address) + "\n"), htmlEscape)) return false;
    snprintf(address, sizeof(address), "0x%08lX", (unsigned long)recoveryIncident.excvaddr);
    if (!streamReportPart(client, ("EXCVADDR=" + String(address) + "\n"), htmlEscape)) return false;
    snprintf(address, sizeof(address), "0x%08lX", (unsigned long)recoveryIncident.depc);
    if (!streamReportPart(client, ("DEPC=" + String(address) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Current boot reason=" + String(resetReasonLabel(currentBootReset.reason)) +
         " (" + String(currentBootReset.reason) + ")\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("ESP reset information=" + ESP.getResetInfo() + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Chip ID=" + String(ESP.getChipId(), HEX) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("CPU MHz=" + String(ESP.getCpuFreqMHz()) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Core version=" + ESP.getCoreVersion() + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("SDK version=" + String(ESP.getSdkVersion()) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Uptime seconds=" + String(millis()/1000UL) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Free heap bytes=" + String(ESP.getFreeHeap()) + "\n"), htmlEscape)) return false;
#if defined(UMM_INFO)
    if (!streamReportPart(client, ("Maximum free block bytes=" + String(ESP.getMaxFreeBlockSize()) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Heap fragmentation percent=" + String(ESP.getHeapFragmentation()) + "\n"), htmlEscape)) return false;
#endif
    if (!streamReportPart(client, ("Free contiguous stack bytes=" + String(ESP.getFreeContStack()) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Free sketch space bytes=" + String(ESP.getFreeSketchSpace()) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Flash configured bytes=" + String(ESP.getFlashChipSize()) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Flash actual bytes=" + String(ESP.getFlashChipRealSize()) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Wi-Fi status=" + String((int)WiFi.status()) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Static local IP=" + WiFi.localIP().toString() + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Gateway=" + WiFi.gatewayIP().toString() + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Subnet=" + WiFi.subnetMask().toString() + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("DNS=" + WiFi.dnsIP().toString() + "\n"), htmlEscape)) return false;
    if (WiFi.status() == WL_CONNECTED) {
        if (!streamReportPart(client, ("Wi-Fi RSSI dBm=" + String(WiFi.RSSI()) + "\n"), htmlEscape)) return false;
        if (!streamReportPart(client, ("Web root requests=" + String(webRootRequestCount) + "\n"), htmlEscape)) return false;
        if (!streamReportPart(client, ("Web rapid reloads retried=" + String(webRootRateLimitedCount) + "\n"), htmlEscape)) return false;
        if (!streamReportPart(client, ("Web memory busy=" + String(webRootBusyCount) + "\n"), htmlEscape)) return false;
        if (!streamReportPart(client, ("Web write aborts=" + String(webRootWriteAbortCount) + "\n"), htmlEscape)) return false;
        if (!streamReportPart(client, ("Web sockets explicitly closed=" + String(webRootForcedCloseCount) + "\n"), htmlEscape)) return false;
        if (!streamReportPart(client, ("Wi-Fi channel=" + String(WiFi.channel()) + "\n"), htmlEscape)) return false;
    }
    if (!streamReportPart(client, ("Recovery OTA active=" + String(recoveryOtaInProgress ? "YES" : "WAITING") + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("OTA recovery clear failed=" + String(recoveryOtaClearFailed ? "YES" : "NO") + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("AP attempts=" + String(credentialApAttempts) + " last failure=" +
         String(credentialApLastFailure) + " (1=radio,2=IP,3=start), heap=" +
         String(credentialApHeapAtAttempt) + "\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("OLED failure hold ms=" + String(OLED_DIAGNOSTIC_HOLD_MS) +
         " pending=" + String(oledDiagnosticCount) + "\n"), htmlEscape)) return false;
    if (oledDiagnosticCount) {
        if (!streamReportPart(client, ("OLED diagnostic=" + String(oledDiagnostics[0].title) + "\n"), htmlEscape)) return false;
        if (!streamReportPart(client, ("OLED cause=" + String(oledDiagnostics[0].detail) + "\n"), htmlEscape)) return false;
    }
    if (!streamReportPart(client, ("Only Recovery HTTP endpoints are available: /, /debug, /debug.txt, /restart-normal, POST /esp-restart.\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("Successful OTA or Restart normally clears the recovery incident before reboot.\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("If clearing the incident fails, Recovery remains latched. External RST also clears Recovery.\n"), htmlEscape)) return false;
    if (!streamReportPart(client, ("No passwords are shown in this diagnostic report.\n"), htmlEscape)) return false;
    return true;
}

void handleRecoveryText() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    WiFiClient client=server.client();
    static const char header[]=
        "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\n"
        "Cache-Control: no-store\r\nConnection: close\r\n\r\n";
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(header),sizeof(header)-1))return;
    streamRecoveryReport(client,false);
}

void handleRecoveryPage() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    WiFiClient client=server.client();
    static const char header[]=
        "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: no-store\r\nConnection: close\r\n\r\n";
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(header),sizeof(header)-1))return;
    PGM_P head=PSTR(
        "<!doctype html><html><head><meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<meta http-equiv='refresh' content='10'>"
        "<title>" DEVICE_LABEL " RECOVERY</title>"
        "<style>body{font:22px/1.35 Arial,sans-serif;max-width:760px;margin:auto;"
        "padding:10px;color:#142431}pre{white-space:pre-wrap;overflow-wrap:anywhere;"
        "border:1px solid #bbb;padding:12px;background:#f5f5f5}</style></head><body>"
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
    if(!writeRootProgmemViaRam(client,head,strlen_P(head)))return;
    if(!streamRecoveryReport(client,true))return;
    PGM_P tail=PSTR("</pre></body></html>");
    writeRootProgmemViaRam(client,tail,strlen_P(tail));
}

// POST only; intentionally keeps persistent recovery latch intact.
void handleRecoveryEspRestart() {
    WebResponseScope responseScope;
    if (!recoveryMode || !requireApWebPin()) return;
    if (recoveryOtaInProgress || recoveryNormalRestartPending) {
        safeFlashResponse(409, "text/plain; charset=utf-8", PSTR("Restart refused: OTA or normal restart in progress."));
        return;
    }
    if (!recoveryEspRestartPending) {
        recoveryEspRestartPending = true;
        recoveryEspRestartRequestedMs = millis();
    }
    safeFlashResponse(200, "text/plain; charset=utf-8", PSTR("ESP restarting, OTA-only Recovery incident preserved."));
}

void handleRecoveryRestartNormal() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    // Fail closed: normal mode is allowed only after the persistent recovery
    // latch has definitely been removed from LittleFS.
    if (!littleFsReady) {
        queueOledDiagnostic(OLED_DIAG_RECOVERY_CLEAR, "RECOVERY CLEAR FAILED", "LittleFS unavailable; Recovery remains latched");
        safeFlashResponse(503, "text/plain; charset=utf-8", PSTR("Restart refused: LittleFS unavailable; Recovery remains latched."));
        return;
    }

    bool clearSucceeded = true;
    if (LittleFS.exists(RECOVERY_FILE)) {
        clearSucceeded = LittleFS.remove(RECOVERY_FILE);
    }

    if (!clearSucceeded) {
        recoveryResetClearFailed = true;
        queueOledDiagnostic(OLED_DIAG_RECOVERY_CLEAR, "RECOVERY CLEAR FAILED", "Recovery file removal failed; restart refused");
        safeFlashResponse(500, "text/plain; charset=utf-8", PSTR("Restart refused: recovery incident could not be cleared."));
        return;
    }

    recoveryIncident = RecoveryRecord();
    recoveryIncidentPersistent = false;

    safeFlashResponse(200, "text/html; charset=utf-8", PSTR("<!doctype html><html><head><meta charset='utf-8'>"
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
        safeFlashResponse(503, "text/plain; charset=utf-8", PSTR("LittleFS unavailable"));
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
        safeFlashResponse(400, "text/plain; charset=utf-8", PSTR("Invalid credential fields"));
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
        safeFlashResponse(400, "text/plain; charset=utf-8", PSTR("Credential field too long"));
        return;
    }
    rec.checksum = deviceCredentialChecksum(rec);

    if (!writeDeviceCredentialRecord(rec)) {
        safeFlashResponse(500, "text/plain; charset=utf-8", PSTR("Could not save credentials"));
        return;
    }
    safeFlashResponse(200, "text/html; charset=utf-8", PSTR("<!doctype html><html><body><h2>Saved</h2>"
                "<p>Saved and read-back verified. Restarting with the stored Wi-Fi settings.</p>"
                "<p>MQTT rejection will not erase settings or disable router Wi-Fi.</p></body></html>"));
    credentialRestartPending = true;
    credentialRestartRequestedMs = millis();
}

void handleForgetCredentialsConfirm() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    safeFlashResponse(200, "text/html; charset=utf-8", PSTR("<!doctype html><html><head><meta charset='utf-8'>"
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
                "</body></html>"));
}

void handleForgetCredentials() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    if (server.arg("confirm") != "ERASE") {
        safeFlashResponse(400, "text/html; charset=utf-8", PSTR("<!doctype html><html><body><h2>Confirmation required</h2>"
                    "<p>Credentials were NOT erased.</p>"
                    "<p><a href='/forget-credentials-confirm'>Open confirmation page</a></p>"
                    "</body></html>"));
        return;
    }

    if (!littleFsReady) {
        safeFlashResponse(503, "text/plain; charset=utf-8", PSTR("LittleFS unavailable"));
        return;
    }
    bool ok = true;
#if PLATFORM_LOCAL_CONTROL_APPLICATION
    if (!applicationClearLegacyCredentials()) {
        safeFlashResponse(500, "text/plain; charset=utf-8", PSTR("Could not remove legacy credentials"));
        return;
    }
#endif
    if (LittleFS.exists(DEVICE_CREDENTIAL_FILE)) ok &= LittleFS.remove(DEVICE_CREDENTIAL_FILE);
    if (LittleFS.exists(DEVICE_CREDENTIAL_TEMP_FILE)) ok &= LittleFS.remove(DEVICE_CREDENTIAL_TEMP_FILE);
    if (!ok) {
        safeFlashResponse(500, "text/plain; charset=utf-8", PSTR("Could not remove credentials"));
        return;
    }
    safeFlashResponse(200, "text/html; charset=utf-8", PSTR("<!doctype html><html><body><h2>Credentials cleared</h2>"
                "<p>Restarting into dedicated Setup AP mode...</p>"
                "<p>The OLED will show the open AP name, web PIN and setup address.</p>"
                "</body></html>"));
    credentialRestartPending = true;
    credentialRestartRequestedMs = millis();
}

void handleForceOtaRecoveryConfirm() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    safeFlashResponse(200, "text/html; charset=utf-8", PSTR(
                "<!doctype html><html><head><meta charset='utf-8'>"
                "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<title>Force OTA Recovery</title><style>body{font:22px/1.35 Arial,sans-serif;max-width:760px;margin:auto;padding:10px;color:#142431}h1,h2,small{font-size:inherit}pre{font:inherit;white-space:pre-wrap;overflow-wrap:anywhere}input,button{font:inherit}</style></head><body>"
                "<h2>Force OTA-ONLY Recovery</h2>"
                "<p>This restarts the ESP8266 into recovery mode. Measurements and MQTT will be disabled until OTA succeeds or Recovery is exited.</p>"
                "<form method='post' action='/force-ota-recovery'>"
                "<label><input type='checkbox' name='confirm' value='ENTER' required> "
                "Enter OTA-ONLY Recovery after restart.</label><br>"
                "<a href='/restart-esp'>Restart ESP (normal reboot)</a><br><br>"
                "<button type='submit'>ENTER OTA-ONLY RECOVERY</button></form>"
                "<p><a href='/'>Cancel</a></p></body></html>"));
}

void handleForceOtaRecovery() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    if (server.arg("confirm") != "ENTER") {
        safeFlashResponse(400, "text/plain; charset=utf-8", PSTR("Confirmation required"));
        return;
    }
    if (!littleFsReady) {
        safeFlashResponse(503, "text/plain; charset=utf-8", PSTR("LittleFS unavailable"));
        return;
    }

    recoveryIncident = RecoveryRecord();
    recoveryIncident.reason = RECOVERY_REASON_FORCED_OTA;
    if (!saveRecoveryIncident() || !loadRecoveryIncident()) {
        safeFlashResponse(500, "text/plain; charset=utf-8", PSTR("Could not persist Recovery request"));
        return;
    }

    clearRtcCrashTrace();
    safeFlashResponse(200, "text/html; charset=utf-8", PSTR(
                "<!doctype html><html><body><h2>OTA-ONLY Recovery requested</h2>"
                "<p>Restarting. IDE_OTA and mDNS will be enabled in Recovery.</p>"
                "</body></html>"));
    forceRecoveryRestartPending = true;
#if PLATFORM_LOCAL_CONTROL_APPLICATION
    applicationPrepareOta();
#endif
    forceRecoveryRestartRequestedMs = millis();
}

// Same GET-confirmation-then-POST flow as ESP32-C3 CommonPlatform.
void handleRestartEspConfirm() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    if (recoveryMode) {
        safeFlashResponse(409, "text/plain; charset=utf-8", PSTR("ESP is in Recovery; use /restart-normal instead."));
        return;
    }
    safeFlashResponse(200, "text/html; charset=utf-8", PSTR(
        "<!doctype html><html><head><meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Restart ESP</title></head><body>"
        "<h2>Restart ESP8266</h2>"
        "<p>Normal restart only. Wi-Fi/MQTT settings are preserved. "
        "This does NOT enter OTA-ONLY Recovery.</p>"
        "<form method='post' action='/restart-esp'>"
        "<button type='submit'>CONFIRM ESP RESTART</button></form>"
        "<p><a href='/force-ota-recovery-confirm'>Cancel</a></p>"
        "</body></html>"));
}
void handleRestartEsp() {
    WebResponseScope responseScope;
    if (!requireApWebPin()) return;
    if (recoveryMode || credentialRestartPending || forceRecoveryRestartPending ||
        mqttAdminRestartPending || mqttAdminCommandPending || recoveryOtaInProgress) {
        safeFlashResponse(409, "text/plain; charset=utf-8",
            PSTR("Restart unavailable during Recovery, OTA or a pending restart."));
        return;
    }
    mqttAdminRestartPending = true;
    mqttAdminRestartRequestedMs = millis();
    safeFlashResponse(200, "text/plain; charset=utf-8", PSTR("ESP8266 restarting normally."));
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

    n = snprintf_P(line, sizeof(line), PSTR(
        "Wi-Fi real disconnections: %lu\n"
        "Last real Wi-Fi disconnect reason: %u\n"
        "Last disconnect age seconds: %lu\n"
        "MQTT-only cooldown duration ms: %lu\n"),
        (unsigned long)wifiDisconnectEvents,
        (unsigned)wifiLastDisconnectReason,
        (unsigned long)(wifiLastDisconnectAtMs ? (millis() - wifiLastDisconnectAtMs) / 1000UL : 0UL),
        (unsigned long)MQTT_TRANSPORT_COOLDOWN_MS);
    if (n <= 0 || n >= (int)sizeof(line)) return;
    if (!writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n)) return;

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
        "MQTT-only cooldowns (Wi-Fi kept ON): %lu\n"
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
    if (n <= 0 || n >= (int)sizeof(body)) {
        safeFlashResponse(503, "application/json", PSTR("{\"error\":\"diagnostics_overflow\"}"));
        return;
    }
    safeRamResponse(200, "application/json; charset=utf-8", body);
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
        safeFlashResponse(503, "text/plain; charset=utf-8", PSTR("OTA-ONLY RECOVERY: open /debug or /restart-normal; POST /esp-restart for safe restart."));
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
    server.on("/restart-esp", HTTP_GET, handleRestartEspConfirm);
    server.on("/restart-esp", HTTP_POST, handleRestartEsp);
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
        safeFlashResponse(404, "text/plain; charset=utf-8", PSTR("Not found"));
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
    ++wifiDisconnectEvents;
    wifiLastDisconnectAtMs = millis();
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
    // Mains-powered unit: avoid Wi-Fi modem sleep during weak-signal reception.
    // This does not improve RF sensitivity or guarantee a connection.
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
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
    // A broker/TLS failure is not proof that Wi-Fi or local HTTP is broken.
    // Keep Wi-Fi, the web listener and physical sensing/fan control running.
    // Recycle only the failed MQTT transport and enforce a cooldown.
    mqttNet.stop(0);
    lastMqttReconnectMs = millis();
    mqttRetryStep = MQTT_RUNTIME_RETRY_STEP_COUNT - 1;
    mqttRetryDelayMs = MQTT_TRANSPORT_COOLDOWN_MS;
    resetMqttRuntimeFailureStreak();
    noteLocalMemoryLowWater();
    queueOledDiagnostic(OLED_DIAG_MQTT, "MQTT COOLDOWN",
                        "Broker-only cleanup; local operation continues");
    return false; // Do not short-circuit local meter / network service.
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
        // Preserve last disconnection cause for network diagnostics.
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
// Ventilation domain logic extracted from the user-provided 1.07 source.

bool heapSuitable(uint32_t minimum,uint16_t contiguous){
  uint32_t free=0;uint16_t block=0;uint8_t frag=0;
  ESP.getHeapStats(&free,&block,&frag);
  return free>=minimum && block>=contiguous;
}

void dailyClearRecord(uint8_t idx){
  DailyRecord& r=dailyArchive.days[idx];
  memset(&r,0,sizeof(r));r.insideMin=r.outsideMin=32767;
  r.insideMax=r.outsideMax=-32768;
}

uint32_t dailyChecksum() {
  const uint8_t* b=(const uint8_t*)&dailyArchive;
  uint32_t sum=2166136261UL;
  for(size_t i=0;i<offsetof(DailyArchive,checksum);i++){
    sum^=b[i];sum*=16777619UL;
  }
  return sum;
}

bool dailyValid(){
  return dailyArchive.magic==DAILY_MAGIC &&
    dailyArchive.format==DAILY_FORMAT && dailyArchive.count==DAILY_COUNT &&
    dailyArchive.checksum==dailyChecksum();
}

bool dailyReadFile(const char* name){
  File f=LittleFS.open(name,"r");
  if(!f)return false;
  size_t n=f.size()==sizeof(dailyArchive)?
    f.read((uint8_t*)&dailyArchive,sizeof(dailyArchive)):0;
  f.close();return n==sizeof(dailyArchive)&&dailyValid();
}

void dailyInit(){
  memset(&dailyArchive,0,sizeof(dailyArchive));
  dailyArchive.magic=DAILY_MAGIC;
  dailyArchive.format=DAILY_FORMAT;
  dailyArchive.count=DAILY_COUNT;
  for(uint8_t i=0;i<DAILY_COUNT;i++)dailyClearRecord(i);
}

void dailyLoad(){
  if(!littleFsReady)return;
  bool a=dailyReadFile(DAILY_A);
  uint32_t aSeq=a?dailyArchive.sequence:0;
  bool b=dailyReadFile(DAILY_B);
  bool useA=a&&(!b||aSeq>=dailyArchive.sequence);
  if(useA&&(!dailyReadFile(DAILY_A)))a=false;
  if(!a&&!b)dailyInit();
  dailyLoaded=a||b;
  dailyActiveA=useA;
}

bool dailySave(){
  if(!littleFsReady||!timeSynchronized||!heapSuitable(3300UL,1250U))return false;
  dailyArchive.magic=DAILY_MAGIC;
  dailyArchive.format=DAILY_FORMAT;dailyArchive.count=DAILY_COUNT;
  dailyArchive.sequence++;
  dailyArchive.checksum=dailyChecksum();
  const char* target=dailyActiveA?DAILY_B:DAILY_A;
  File f=LittleFS.open(target,"w");
  if(!f){dailyArchive.sequence--;dailyWriteErrors++;return false;}
  bool good=f.write((const uint8_t*)&dailyArchive,sizeof(dailyArchive))==sizeof(dailyArchive);
  f.flush();f.close();
  if(good){
    // Verify checksum directly from flash in small pieces, do not allocate
    // another ~750B archive or overwrite the active in-RAM records.
    File check=LittleFS.open(target,"r");
    if(check&&check.size()==sizeof(dailyArchive)){
      uint32_t crc=2166136261UL;
      const size_t payload=offsetof(DailyArchive,checksum);
      for(size_t i=0;i<payload;i++){
        int c=check.read();
        if(c<0){good=false;break;}
        crc^=(uint8_t)c;crc*=16777619UL;
        if((i&127)==0)yield();
      }
      good=good&&crc==dailyArchive.checksum;
    }else good=false;
    if(check)check.close();
  }
  if(!good){dailyArchive.sequence--;dailyWriteErrors++;return false;}
  dailyActiveA=!dailyActiveA;dailyLoaded=true;dailyWrites++;return true;
}

uint32_t historyChecksum() {
  const HistorySnapshot& h=historyDisk;
  const uint8_t* b=(const uint8_t*)&h;
  uint32_t v=2166136261UL;
  for(size_t i=0;i<offsetof(HistorySnapshot,checksum);++i){v^=b[i];v*=16777619UL;}
  return v;
}

bool historySnapshotValid() {
  const HistorySnapshot& h=historyDisk;
  return h.magic==HISTORY_MAGIC&&h.format==HISTORY_FORMAT&&
    h.dayKey>40000&&h.dayKey<80000&&
    h.savedEpoch>=1700000000UL&&h.slotKey>=0&&
    h.checksum==historyChecksum();
}

int16_t historyQuantize(float value) {
  if(!std::isfinite(value)||value<-55.0f||value>125.0f)return HISTORY_EMPTY;
  return (int16_t)lroundf(value*10.0f);
}

float historyExpand(int16_t value) {
  return value==HISTORY_EMPTY?NAN:(float)value*0.1f;
}

int16_t minuteTemperatureEncode(float value) {
  if(!std::isfinite(value)||value<-55.0f||value>125.0f)return HISTORY_EMPTY;
  return (int16_t)lroundf(value*80.0f);
}

float minuteTemperatureDecode(int16_t value) {
  if(value==HISTORY_EMPTY)return NAN;
  // Preserve the exact float produced by historyExpand for restored tenths.
  // Codes from DS18B20 are multiples of 5; their overlap with tenths is an
  // exact half-degree, so both paths produce the same original reading.
  return value%8==0?(float)(value/8)*0.1f:(float)value/80.0f;
}

int32_t historyDayOrdinal(int key) {
  int year=key/400+1900,day=key%400;
  int32_t total=0;
  for(int y=2000;y<year;++y)
    total+=365+((y%4==0 && (y%100!=0||y%400==0))?1:0);
  return total+day;
}

int16_t historyQuarterAverage(const int16_t* minutes,int q) {
  float total=0.0f;int samples=0;
  for(int i=q*15;i<(q+1)*15;i++){
    float t=minuteTemperatureDecode(minutes[i]);
    if(std::isfinite(t)&&t>=-55.0f&&t<=125.0f){total+=t;samples++;}
  }
  return samples?historyQuantize(total/samples):HISTORY_EMPTY;
}

void historyApplySnapshot() {
  const HistorySnapshot& h=historyDisk;
  for(int q=0;q<96;q++){
    float inside=historyExpand(h.currentInside[q]);
    float outside=historyExpand(h.currentOutside[q]);
    for(int j=0;j<15;j++){
      todayInside[q*15+j]=minuteTemperatureEncode(inside);
      todayOutside[q*15+j]=minuteTemperatureEncode(outside);
    }
    yesterdayInside[q]=historyExpand(h.previousInside[q]);
    yesterdayOutside[q]=historyExpand(h.previousOutside[q]);
  }
  previousDayKey=h.dayKey;
  historySavedAt=h.savedEpoch;
  historyLastSlotKey=h.slotKey;
  historySequence=h.sequence;
  historyLoaded=true;
  lastLoggedMinute=-1;
}

void historyLoadLittleFS() {
  if(!littleFsReady)return;
  const char* paths[]={HISTORY_A,HISTORY_B};
  for(int slot=0;slot<2;slot++){
    File file=LittleFS.open(paths[slot],"r");
    if(!file)continue;
    bool sizeOk=file.size()==sizeof(HistorySnapshot);
    size_t read=sizeOk?file.read((uint8_t*)&historyDisk,sizeof(historyDisk)):0;
    file.close();
    if(read!=sizeof(historyDisk)||!historySnapshotValid())continue;
    if(!historyLoaded||historyDisk.sequence>historySequence){
      historyApplySnapshot();
      historyActiveSlotA=(slot==0);
    }
  }
}

void dailyRotateTo(int32_t dayKey){
  if(dayKey<40000||dayKey>80000)return;
  if(dailyArchive.days[0].dayKey==dayKey)return;
  int32_t latest=dailyArchive.days[0].dayKey;
  if(latest!=0){
    int32_t delta=historyDayOrdinal(dayKey)-historyDayOrdinal(latest);
    if(delta<=0)return;
    if(delta>=DAILY_COUNT){
      for(uint8_t i=0;i<DAILY_COUNT;i++)dailyClearRecord(i);
    }else{
      memmove(&dailyArchive.days[delta],&dailyArchive.days[0],
        (DAILY_COUNT-delta)*sizeof(DailyRecord));
      for(uint8_t i=0;i<delta;i++)dailyClearRecord(i);
    }
  }else dailyClearRecord(0);
  dailyArchive.days[0].dayKey=dayKey;
}

void dailyRecordSample(float inside,float outside){
  if(dailyArchive.days[0].dayKey!=previousDayKey)return;
  DailyRecord& r=dailyArchive.days[0];
  if(std::isfinite(inside)&&inside>=-55.0f&&inside<=125.0f&&r.insideCount<1440){
    int16_t v=historyQuantize(inside);
    if(v<r.insideMin)r.insideMin=v;
    if(v>r.insideMax)r.insideMax=v;
    r.insideSum+=(int32_t)v;r.insideCount++;
  }
  if(std::isfinite(outside)&&outside>=-55.0f&&outside<=125.0f&&r.outsideCount<1440){
    int16_t v=historyQuantize(outside);
    if(v<r.outsideMin)r.outsideMin=v;
    if(v>r.outsideMax)r.outsideMax=v;
    r.outsideSum+=(int32_t)v;r.outsideCount++;
  }
}

bool historySaveLittleFS() {
  if(!littleFsReady||!timeSynchronized||previousDayKey<40000)return false;
  // An 800-byte LittleFS write must not starve MQTT/OTA on a fragmented ESP.
  if(!heapSuitable(3400UL,1300U)){++historyWriteFailures;return false;}
  CrashTraceScope historySaveTrace(CRASH_STAGE_VENT_HISTORY_SAVE, ESP.getFreeContStack());
  memset(&historyDisk,0,sizeof(historyDisk));
  historyDisk.magic=HISTORY_MAGIC;
  historyDisk.sequence=historySequence+1;
  historyDisk.format=HISTORY_FORMAT;
  historyDisk.dayKey=previousDayKey;
  historyDisk.slotKey=previousDayKey*100+(currentHour*60+currentMinute)/15;
  historyDisk.savedEpoch=(uint32_t)time(nullptr);
  for(int q=0;q<96;q++){
    historyDisk.currentInside[q]=historyQuarterAverage(todayInside,q);
    historyDisk.currentOutside[q]=historyQuarterAverage(todayOutside,q);
    historyDisk.previousInside[q]=historyQuantize(yesterdayInside[q]);
    historyDisk.previousOutside[q]=historyQuantize(yesterdayOutside[q]);
  }
  historyDisk.checksum=historyChecksum();
  // Keep the old complete slot intact until the other slot is verified.
  const char* target=historyActiveSlotA?HISTORY_B:HISTORY_A;
  File file=LittleFS.open(target,"w");
  if(!file){++historyWriteFailures;return false;}
  size_t bytes=file.write((const uint8_t*)&historyDisk,sizeof(historyDisk));
  file.flush();file.close();
  if(bytes!=sizeof(historyDisk)){++historyWriteFailures;return false;}
  file=LittleFS.open(target,"r");
  if(!file){++historyWriteFailures;return false;}
  bool valid=file.size()==sizeof(historyDisk) &&
    file.read((uint8_t*)&historyDisk,sizeof(historyDisk))==sizeof(historyDisk) &&
    historySnapshotValid();
  file.close();
  if(!valid){++historyWriteFailures;return false;}
  historyActiveSlotA=!historyActiveSlotA;
  historySequence=historyDisk.sequence;
  historyLastSlotKey=historyDisk.slotKey;
  historySavedAt=historyDisk.savedEpoch;
  ++historyWrites;
  // Extra daily archive is updated on the same 15-minute cadence, not
  // at the 15-second live refresh interval or per-minute RAM sample.
  dailySave();
  return true;
}

void historyMaybePersist() {
  if(!littleFsReady||!timeSynchronized)return;
  int minute=currentHour*60+currentMinute;
  int32_t slot=previousDayKey*100+minute/15;
  if(slot==historyLastSlotKey)return;
  if(minute%15==0)historyPendingRetrySlot=slot;
  if(historyPendingRetrySlot!=slot || minute%15>3 ||
     minute==historyLastAttemptMinute)return;
  historyLastAttemptMinute=minute;
  if(historySaveLittleFS())historyPendingRetrySlot=-1;
}

String hhmm(int minute){char s[6];snprintf(s,sizeof(s),"%02d:%02d",minute/60,minute%60);return String(s);}

String clockString(){char s[9];snprintf(s,sizeof(s),"%02d:%02d:%02d",currentHour,currentMinute,currentSecond);return String(s);}

bool parseClock(const String &s,bool withSeconds,int &seconds) {
  if(s.length()!=(withSeconds?8U:5U)||s.charAt(2)!=':'||(withSeconds&&s.charAt(5)!=':'))return false;
  for(unsigned int i=0;i<s.length();i++){
    if(i==2||(withSeconds&&i==5))continue;
    if(s.charAt(i)<'0'||s.charAt(i)>'9')return false;
  }
  int h=s.substring(0,2).toInt(),m=s.substring(3,5).toInt();
  int sec=withSeconds?s.substring(6,8).toInt():0;
  if(h>23||m>59||sec>59)return false;
  seconds=h*3600+m*60+sec;return true;
}

void quarterAverages(float (&dest)[96],const int16_t (&src)[1440]) {
  for(int k=0;k<96;++k){
    double sum=0;int count=0;
    for(int i=k*15;i<k*15+15;++i){
      float t=minuteTemperatureDecode(src[i]);
      if(std::isfinite(t)&&t>=-20&&t<=80){sum+=t;++count;}
    }
    dest[k]=count?sum/count:NAN;
  }
}

void rolloverHistory() {
  quarterAverages(yesterdayInside,todayInside);
  quarterAverages(yesterdayOutside,todayOutside);
  for(int i=0;i<1440;++i)todayInside[i]=todayOutside[i]=HISTORY_EMPTY;
  lastLoggedMinute=-1;
}

void updateClock() {
  time_t now=time(nullptr);
  if(now>1700000000){
    now+=manualTimeOffset;
    struct tm local={};localtime_r(&now,&local);
    int key=local.tm_year*400+local.tm_yday;
    if(previousDayKey!=-1&&key!=previousDayKey){
      int32_t gap=historyDayOrdinal(key)-historyDayOrdinal(previousDayKey);
      if(gap==1){rolloverHistory();}
      else {
        // Long power outage: never mislabel stale records as "yesterday".
        for(int i=0;i<1440;i++)todayInside[i]=todayOutside[i]=HISTORY_EMPTY;
        for(int i=0;i<96;i++)yesterdayInside[i]=yesterdayOutside[i]=NAN;
        lastLoggedMinute=-1;
      }
      if(gap>0)uptimeDayCount+=(uint32_t)gap;
      historyLastAttemptMinute=-1;
      historyPendingRetrySlot=-1;
    }
    previousDayKey=key;
    dailyRotateTo(key);
    currentHour=local.tm_hour;currentMinute=local.tm_min;currentSecond=local.tm_sec;
    timeSynchronized=true;
    return;
  }
  // Monotonic clock until the first NTP response, with an explicit unsynchronized marker.
  unsigned long ms=millis();unsigned long elapsed=(ms-lastClockTick)/1000UL;
  if(!elapsed)return;
  lastClockTick+=elapsed*1000UL;
  unsigned long sec=(unsigned long)currentHour*3600UL+currentMinute*60UL+currentSecond+elapsed;
  if(sec>=86400UL){rolloverHistory();uptimeDayCount+=sec/86400UL;}
  sec%=86400UL;currentHour=sec/3600UL;currentMinute=(sec/60UL)%60UL;currentSecond=sec%60UL;
}

void setManualClock(int totalSeconds) {
  currentHour=totalSeconds/3600;currentMinute=(totalSeconds/60)%60;currentSecond=totalSeconds%60;
  time_t now=time(nullptr);
  if(now>1700000000){struct tm local={};localtime_r(&now,&local);
    manualTimeOffset=totalSeconds-(local.tm_hour*3600+local.tm_min*60+local.tm_sec);
  }
  manualClockProvided=true;
  lastClockTick=millis();
}

void logTemperatureMinute() {
  if(!timeSynchronized&&!manualClockProvided)return;
  int minute=currentHour*60+currentMinute;
  if(minute==lastLoggedMinute)return;
  lastLoggedMinute=minute;
  todayInside[minute]=minuteTemperatureEncode(temperatureInside);
  todayOutside[minute]=minuteTemperatureEncode(temperatureOutside);
  if(timeSynchronized)dailyRecordSample(temperatureInside,temperatureOutside);
  historyMaybePersist(); // At most one verified A/B write per 15-minute bucket.
}

bool validTemperature(float t){return std::isfinite(t)&&t>-55&&t<125;}

// Asynchronous conversion avoids blocking the application on DS18B20 timing.
// Existing fan SafeMode and history calculation are unchanged.
bool ventDsConversionPending = false;
unsigned long ventDsConversionStartedMs = 0;
unsigned long ventLastSampleCompletedMs = 0;
unsigned long ventMaximumSampleGapMs = 0;
uint32_t ventCompletedSampleCount = 0;
const unsigned long VENT_DS_CONVERSION_WAIT_MS = 750UL; // 12-bit worst case
// v7.11 WDT: only 784 stack bytes, 5016 heap bytes, 4408 max block.
const uint32_t VENT_DHT_MIN_STACK_BYTES = 1536UL;
const uint32_t VENT_DHT_MIN_HEAP_BYTES = 8192UL;
const uint32_t VENT_DHT_MIN_BLOCK_BYTES = 6144UL;
bool ventDhtOutsideNext = true;
uint32_t ventDhtOutsideReads = 0, ventDhtInsideReads = 0;
uint32_t ventDhtLowMemorySkips = 0, ventDhtReadFailures = 0;
uint32_t ventDhtLastDurationMs = 0;
uint32_t ventDhtLastStackBytes = 0, ventDhtLastHeapBytes = 0;
uint32_t ventDhtLastMaxBlockBytes = 0;

void sampleHardware() {
  const unsigned long ms=millis();
  if (!ventDsConversionPending) {
    if(lastSensorAt && ms-lastSensorAt<2500UL)return;
    lastSensorAt=ms;
    {
      CrashTraceScope sensorStartTrace(CRASH_STAGE_VENT_DS18B20, ESP.getFreeContStack());
      ds.requestTemperatures(); // returns immediately: setWaitForConversion(false)
    }
    ventDsConversionStartedMs=millis();
    ventDsConversionPending=true;
    return;
  }
  if((unsigned long)(ms-ventDsConversionStartedMs)<VENT_DS_CONVERSION_WAIT_MS)return;
  ventDsConversionPending=false;
  CrashTraceScope sensorTrace(CRASH_STAGE_VENT_DS18B20, ESP.getFreeContStack());
  float out=ds.getTempC(addrOutside),in=ds.getTempC(addrInside),mid=ds.getTempC(addrMiddle);
  temperatureOutside=validTemperature(out)?out:NAN;
  temperatureInside=validTemperature(in)?in:NAN;
  temperatureMiddle=validTemperature(mid)?mid:NAN;
  // Only one noncritical DHT11 each completed DS cycle, alternating probes.
  // Critical DS18B20 sampling, SafeMode and fan control are unchanged.
  const bool readOutside = ventDhtOutsideNext;
  ventDhtOutsideNext = !ventDhtOutsideNext;
  ventDhtLastStackBytes = ESP.getFreeContStack();
  ventDhtLastHeapBytes = ESP.getFreeHeap();
  ventDhtLastMaxBlockBytes = ESP.getMaxFreeBlockSize();
  const bool dhtMemoryReady =
    ventDhtLastStackBytes >= VENT_DHT_MIN_STACK_BYTES &&
    ventDhtLastHeapBytes >= VENT_DHT_MIN_HEAP_BYTES &&
    ventDhtLastMaxBlockBytes >= VENT_DHT_MIN_BLOCK_BYTES;
  if (!dhtMemoryReady) {
    ++ventDhtLowMemorySkips;
    // Optional data is unavailable rather than a misleading stale value.
    if (readOutside) { humidityOutside=NAN; dhtTemperatureOutside=NAN; }
    else { humidityInside=NAN; dhtTemperatureInside=NAN; }
  } else {
    sensorTrace.enter(readOutside ?
      CRASH_STAGE_VENT_DHT_OUTSIDE : CRASH_STAGE_VENT_DHT_INSIDE,
      ventDhtLastStackBytes);
    const unsigned long beginDhtMs = millis();
    if (readOutside) {
      humidityOutside = dhtOutside.readHumidity();
      dhtTemperatureOutside = dhtOutside.readTemperature(); // cached
      ++ventDhtOutsideReads;
      if (!std::isfinite(humidityOutside) || !validTemperature(dhtTemperatureOutside))
        ++ventDhtReadFailures;
    } else {
      humidityInside = dhtInside.readHumidity();
      dhtTemperatureInside = dhtInside.readTemperature(); // cached
      ++ventDhtInsideReads;
      if (!std::isfinite(humidityInside) || !validTemperature(dhtTemperatureInside))
        ++ventDhtReadFailures;
    }
    ventDhtLastDurationMs = millis() - beginDhtMs;
  }
  // Service the Wi-Fi SDK following this timing-sensitive optional read.
  yield();
  // No DHT error or low-memory skip affects the critical DS fan decision.
  const bool outOk=validTemperature(temperatureOutside);
  const bool inOk=validTemperature(temperatureInside);
  ventTrackProbe(DS_OUTSIDE,outOk);
  ventTrackProbe(DS_MIDDLE,validTemperature(temperatureMiddle));
  ventTrackProbe(DS_INSIDE,inOk);
  ventTrackProbe(DHT_OUT_TEMP,validTemperature(dhtTemperatureOutside));
  ventTrackProbe(DHT_OUT_HUM,std::isfinite(humidityOutside)&&humidityOutside>=0&&humidityOutside<=100);
  ventTrackProbe(DHT_IN_TEMP,validTemperature(dhtTemperatureInside));
  ventTrackProbe(DHT_IN_HUM,std::isfinite(humidityInside)&&humidityInside>=0&&humidityInside<=100);
  if(outOk){if(ventGoodOutside<VENT_GOOD_FOR_REARM)++ventGoodOutside;}
  else ventGoodOutside=0;
  if(inOk){if(ventGoodInside<VENT_GOOD_FOR_REARM)++ventGoodInside;}
  else ventGoodInside=0;
  if(ventProbeFails[DS_OUTSIDE]>=VENT_FAIL_LIMIT){
    ventSafeMode=true;ventSafeOutside=true;
  }
  if(ventProbeFails[DS_INSIDE]>=VENT_FAIL_LIMIT){
    ventSafeMode=true;ventSafeInside=true;
  }
  sensorsOk=outOk&&inOk;
  const unsigned long completedAt=millis();
  if (ventLastSampleCompletedMs) {
    const unsigned long gap=completedAt-ventLastSampleCompletedMs;
    if(gap>ventMaximumSampleGapMs)ventMaximumSampleGapMs=gap;
  }
  ventLastSampleCompletedMs=completedAt;
  ++ventCompletedSampleCount;
  noteLocalMemoryLowWater();
}

bool isPausePeriod() {
  if(pauseFrom==pauseUntil)return false; // equal bounds mean no pause
  int now=currentHour*60+currentMinute;
  return pauseFrom<pauseUntil ? (now>=pauseFrom&&now<pauseUntil):(now>=pauseFrom||now<pauseUntil);
}

void applyFanControl() {
  if(ventilationOtaPaused || ventSafeMode){
    fanRunning=false;
    manualFan=false; // Never restore a previously ON manual command after rearm.
    digitalWrite(GPIO_FAN,FAN_OFF_LEVEL);
    return;
  }
  if(manualMode)fanRunning=manualFan;
  else if(!sensorsOk)fanRunning=false; // fail safe on invalid critical thermometers
  else if(isPausePeriod())fanRunning=false;
  else if(summerMode){
    if(fanRunning&&temperatureInside<=temperatureOutside)fanRunning=false;
    else if(!fanRunning&&temperatureInside>=temperatureOutside+0.5f)fanRunning=true;
  } else {
    if(fanRunning&&temperatureOutside<temperatureInside)fanRunning=false;
    else if(!fanRunning&&temperatureOutside>=temperatureInside+1.0f)fanRunning=true;
  }
  fanHighSpeed=speedOverrideActive?manualHighSpeed:(digitalRead(GPIO_RF)==HIGH);
  digitalWrite(GPIO_FAN,fanRunning?FAN_ON_LEVEL:FAN_OFF_LEVEL);
  digitalWrite(GPIO_SPEED,fanHighSpeed?SPEED_HIGH_LEVEL:SPEED_LOW_LEVEL);
}

String jsonNumber(float v){return std::isfinite(v)?String(v,1):String("null");}

void jsonFloat(float value,char *dest,size_t size){
  if(!dest||size<8)return;
  if(!std::isfinite(value)){strncpy(dest,"null",size);dest[size-1]=0;return;}
  // ESP8266 newlib printf float support is not guaranteed: dtostrf is stable.
  dtostrf(value,0,1,dest);
  if(strlen(dest)>=size){strncpy(dest,"null",size);dest[size-1]=0;}
}

bool publishMqttSample(){
  if(!mqttClient.connected())return false;
  if(!heapSuitable(2400UL,1600U)){
    ++ventilationStatePublishFailures;return false;
  }
  CrashTraceScope publishTrace(CRASH_STAGE_VENT_SAMPLE_PUBLISH, ESP.getFreeContStack());
  // EnergyMeter v5.58: use a fixed bounded buffer for hot APP update path;
  // do NOT repeatedly allocate/reallocate a large Arduino String under TLS.
  char ti[18],to[18],tm[18],hi[18],ho[18],di[18],dout[18];
  char ahIn[18],ahOut[18],feelIn[18],feelOut[18];
  jsonFloat(temperatureInside,ti,sizeof(ti));
  jsonFloat(temperatureOutside,to,sizeof(to));
  jsonFloat(temperatureMiddle,tm,sizeof(tm));
  jsonFloat(humidityInside,hi,sizeof(hi));
  jsonFloat(humidityOutside,ho,sizeof(ho));
  jsonFloat(dhtTemperatureInside,di,sizeof(di));
  jsonFloat(dhtTemperatureOutside,dout,sizeof(dout));
  const bool insideDhtOk=validTemperature(dhtTemperatureInside)&&
    std::isfinite(humidityInside)&&humidityInside>=0&&humidityInside<=100;
  const bool outsideDhtOk=validTemperature(dhtTemperatureOutside)&&
    std::isfinite(humidityOutside)&&humidityOutside>=0&&humidityOutside<=100;
  jsonFloat(insideDhtOk?dhtCalculations.computeAbsoluteHumidity(dhtTemperatureInside,humidityInside):NAN,ahIn,sizeof(ahIn));
  jsonFloat(outsideDhtOk?dhtCalculations.computeAbsoluteHumidity(dhtTemperatureOutside,humidityOutside):NAN,ahOut,sizeof(ahOut));
  jsonFloat(insideDhtOk?dhtCalculations.computeHeatIndex(dhtTemperatureInside,humidityInside):NAN,feelIn,sizeof(feelIn));
  jsonFloat(outsideDhtOk?dhtCalculations.computeHeatIndex(dhtTemperatureOutside,humidityOutside):NAN,feelOut,sizeof(feelOut));
  char body[940];
  int n=snprintf(body,sizeof(body),
    "{\"meter\":\"VENTILATION\",\"device_type\":\"VENTILATION\","
    "\"device\":\"%s\",\"firmware\":\"%s\","
    "\"temp_inside\":%s,\"temp_outside\":%s,\"temp_middle\":%s,"
    "\"humidity_inside\":%s,\"humidity_outside\":%s,"
    "\"dht_temp_inside\":%s,\"dht_temp_outside\":%s,"
    "\"absolute_humidity_inside\":%s,\"absolute_humidity_outside\":%s,"
    "\"feels_like_inside\":%s,\"feels_like_outside\":%s,"
    "\"safe_mode\":%s,\"safe_outside\":%s,\"safe_inside\":%s,"
    "\"fan_on\":%s,\"speed\":\"%s\",\"mode\":\"%s\",\"season\":\"%s\","
    "\"time\":\"%02d:%02d:%02d\","
    "\"stop_from\":\"%02d:%02d\",\"stop_to\":\"%02d:%02d\","
    "\"sensors_ok\":%s,\"time_synced\":%s,"
    "\"rf_high\":%s,\"speed_source\":\"%s\", "
    "\"uptime_days\":%lu,\"rssi\":%d,\"ota_supported\":true}",
    mqttDeviceId,VERSION,ti,to,tm,hi,ho,di,dout,
    ahIn,ahOut,feelIn,feelOut,
    ventSafeMode?"true":"false",ventSafeOutside?"true":"false",ventSafeInside?"true":"false",
    fanRunning?"true":"false",fanHighSpeed?"high":"low",
    manualMode?"manual":"auto",summerMode?"summer":"winter",
    currentHour,currentMinute,currentSecond,
    pauseFrom/60,pauseFrom%60,pauseUntil/60,pauseUntil%60,
    sensorsOk?"true":"false",timeSynchronized?"true":"false",
    digitalRead(GPIO_RF)==HIGH?"true":"false",
    speedOverrideActive?"operator":"rf",
    (unsigned long)uptimeDayCount,WiFi.RSSI());
  if(n>0 && n<(int)sizeof(body) && n<950 &&
     mqttClient.publish(MQTT_STATE_TOPIC,body,false)){
    return true;
  }else{
    ++ventilationStatePublishFailures;return false;
  }
}

void publishAck(bool ok,const char* command,const char* error="") {
  if(!mqttClient.connected())return;
  char body[180];snprintf(body,sizeof(body),
    "{\"ok\":%s,\"cmd\":\"%s\",\"error\":\"%s\"}",ok?"true":"false",command,error);
  mqttClient.publish(MQTT_ADMIN_RESPONSE_TOPIC,body,false);
}

void processCommand() {
  commandReady=false;String raw=String(commandBuf);commandBuf[0]=0;
  int split=raw.indexOf('|');String cmd=split<0?raw:raw.substring(0,split);
  String value=split<0?"":raw.substring(split+1);
  bool ok=true;const char* error="";
  if(cmd=="vent_mode"){
    if(value=="manual"){
      manualMode=true;manualFan=fanRunning;manualHighSpeed=fanHighSpeed;
    }else if(value=="auto")manualMode=false;
    else{ok=false;error="invalid_value";}
  }else if(cmd=="vent_fan"){
    if(!manualMode){ok=false;error="manual_required";}
    else if(value=="on")manualFan=true;
    else if(value=="off")manualFan=false;
    else{ok=false;error="invalid_value";}
  }else if(cmd=="vent_speed"){
    if(value=="high"){manualHighSpeed=true;speedOverrideActive=true;}
    else if(value=="low"){manualHighSpeed=false;speedOverrideActive=true;}
    else{ok=false;error="invalid_value";}
  }else if(cmd=="vent_season"){
    if(value=="summer")summerMode=true;
    else if(value=="winter")summerMode=false;
    else{ok=false;error="invalid_value";}
  }else if(cmd=="vent_schedule"){
    int pipe=value.indexOf('|');int t1,t2;
    if(pipe<0||!parseClock(value.substring(0,pipe),false,t1)||
      !parseClock(value.substring(pipe+1),false,t2)){ok=false;error="invalid_time";}
    else{pauseFrom=t1/60;pauseUntil=t2/60;}
  }else if(cmd=="vent_safe_reset"){
    if(!ventTryRearmSafeMode()){ok=false;error="sensors_not_stably_recovered";}
  }else if(cmd=="vent_time"){
    int s;
    if(timeSynchronized){ok=false;error="ntp_active";}
    else if(!parseClock(value,true,s)){ok=false;error="invalid_time";}
    else setManualClock(s);
  }else if(cmd=="vent_history"){
    if(value!="current"&&value!="previous"&&value!="days30"){ok=false;error="invalid_range";}
    else{value.toCharArray(historyRange,sizeof(historyRange));historyOffset=0;historyStreaming=true;}
  }else{ok=false;error="invalid_command";}
  if(ok){
    applyFanControl();
    publishAck(true,cmd.c_str());
    if(!publishMqttSample())mqttUpdateRequested=true;
  }
  else publishAck(false,cmd.c_str(),error);
}

String dailyValue(int16_t deci,uint16_t count){
  return count?String((float)deci/10.0f,1):String("null");
}

String dailyMean(int32_t sum,uint16_t count){
  return count?String((float)sum/(10.0f*count),1):String("null");
}

String dailyRow(uint8_t idx){
  const DailyRecord& r=dailyArchive.days[idx];
  String msg=String(r.dayKey);
  msg+=',';msg+=dailyValue(r.insideMin,r.insideCount);
  msg+=',';msg+=dailyValue(r.insideMax,r.insideCount);
  msg+=',';msg+=dailyMean(r.insideSum,r.insideCount);
  msg+=',';msg+=dailyValue(r.outsideMin,r.outsideCount);
  msg+=',';msg+=dailyValue(r.outsideMax,r.outsideCount);
  msg+=',';msg+=dailyMean(r.outsideSum,r.outsideCount);
  return msg;
}

void streamHistoryMQTT() {
  if(!historyStreaming||!mqttClient.connected())return;
  CrashTraceScope historyStreamTrace(CRASH_STAGE_VENT_HISTORY_STREAM, ESP.getFreeContStack());
  if(strcmp(historyRange,"days30")==0){
    if(historyOffset>=DAILY_COUNT){
      if(mqttClient.publish(VENT_HISTORY_TOPIC,"E|days30",false))historyStreaming=false;
      return;
    }
    String msg="D|";msg+=historyOffset;
    for(uint8_t i=0;i<3;i++){
      uint8_t index=historyOffset+i;
      msg+='|';msg+=dailyRow(index);
    }
    if(msg.length()<800 && mqttClient.publish(VENT_HISTORY_TOPIC,msg.c_str(),false))historyOffset+=3;
    return;
  }
  if(historyOffset>=96){String msg="E|";msg+=historyRange;
    mqttClient.publish(VENT_HISTORY_TOPIC,msg.c_str(),false);historyStreaming=false;return;}
  float in[96],out[96];
  if(strcmp(historyRange,"previous")==0){memcpy(in,yesterdayInside,sizeof(in));memcpy(out,yesterdayOutside,sizeof(out));}
  else{quarterAverages(in,todayInside);quarterAverages(out,todayOutside);}
  String msg="R|";msg+=historyRange;msg+='|';msg+=historyOffset;
  for(int i=0;i<8;++i){int at=historyOffset+i;msg+='|';
    msg+=std::isfinite(in[at])?String(in[at],1):String("null");
    msg+=',';msg+=std::isfinite(out[at])?String(out[at],1):String("null");}
  if(mqttClient.publish(VENT_HISTORY_TOPIC,msg.c_str(),false))historyOffset+=8;
}

// Ventilation-specific pages and controls. Shared platform owns HTTP lifetime,
// backpressure, credentials, diagnostics and OTA recovery routes.
// HTTP body bytes use the platform RAM-copy writer (Core 2.7.2 write_P is unsafe
// under fragmented heap); no full HTML page is assembled in RAM.
bool ventilationWebSend(const String& content) {
  WiFiClient client=server.client();
  return writeRootBytes(client,reinterpret_cast<const uint8_t*>(content.c_str()),content.length());
}
bool ventilationWebSend(const char* content) {
  WiFiClient client=server.client();
  return writeRootBytes(client,reinterpret_cast<const uint8_t*>(content),strlen(content));
}
bool ventilationWebSendProgmem(PGM_P content) {
  WiFiClient client=server.client();
  return writeRootProgmemViaRam(client,content,strlen_P(content));
}
bool ventilationWebSend(const __FlashStringHelper* content) {
  return ventilationWebSendProgmem(reinterpret_cast<PGM_P>(content));
}
void redirectHome(){server.sendHeader("Location","/");server.send(303);}
bool localControlAuthorized(){return requireApWebPin();}

// Human-readable web values are separate from JSON/MQTT serialization.
String ventilationWebNumber(float value) {
  return std::isfinite(value)?String(value,1):String(F("—"));
}

// Control forms must keep their in-progress values while the operator edits.
bool applicationWebLiveUpdates() { return false; }

bool applicationRenderWebContent(WiFiClient& client) {
  // Only application content belongs here. The common root handler owns the
  // page, styles, access checks, HTTP lifetime, network status and footer.
  String s;
  if(!s.reserve(640))return false;
  s=F("<section>");
  if(ventSafeMode){
    s+=F("<p role='alert' style='color:#b00020'><strong>SAFE MODE - FAN OFF.</strong> Critical DS18B20: ");
    if(ventSafeOutside)s+=F("OUTSIDE ");
    if(ventSafeInside)s+=F("INSIDE ");
    s+=F(". Repair the probe, wait for 3 good samples, then reset SafeMode.</p>");
  }
  s+=F("<p>Outside: <b>");
  s+=ventSafeOutside?String(F("SafeMode")):ventilationWebNumber(temperatureOutside);s+=F(" °C</b> — humidity <b>");
  s+=ventilationWebNumber(humidityOutside);s+=F("%</b> — DHT <b>");s+=ventilationWebNumber(dhtTemperatureOutside);s+=F(" °C</b></p>");
  s+=F("<p>Middle: <b>");s+=ventilationWebNumber(temperatureMiddle);s+=F(" °C</b></p>");
  s+=F("<p>Inside: <b>");
  s+=ventSafeInside?String(F("SafeMode")):ventilationWebNumber(temperatureInside);s+=F(" °C</b> — humidity <b>");
  s+=ventilationWebNumber(humidityInside);s+=F("%</b> — DHT <b>");s+=ventilationWebNumber(dhtTemperatureInside);s+=F(" °C</b></p>");
  if(!writeRootBytes(client,reinterpret_cast<const uint8_t*>(s.c_str()),s.length()))return false;
  s.remove(0);yield();
  if(std::isfinite(humidityOutside)&&validTemperature(dhtTemperatureOutside)){
    s+=F("<p>Outside feels like: <b>");s+=ventilationWebNumber(dhtCalculations.computeHeatIndex(dhtTemperatureOutside,humidityOutside));
    s+=F(" °C</b> · Absolute humidity <b>");s+=ventilationWebNumber(dhtCalculations.computeAbsoluteHumidity(dhtTemperatureOutside,humidityOutside));
    static const char* perception[]={"Dry","Very comfortable","Comfortable","Ok","Uncomfortable",
      "Quite uncomfortable","Very uncomfortable","Severe uncomfortable"};
    uint8_t category=dhtCalculations.computePerception(temperatureOutside,humidityOutside);
    s+=F(" g/m³</b> · Perception: <b>");s+=category<8?perception[category]:"unknown";s+=F("</b></p>");
  }
  if(std::isfinite(humidityInside)&&validTemperature(dhtTemperatureInside)){
    s+=F("<p>Inside feels like: <b>");s+=ventilationWebNumber(dhtCalculations.computeHeatIndex(dhtTemperatureInside,humidityInside));
    s+=F(" °C</b> · Absolute humidity <b>");s+=ventilationWebNumber(dhtCalculations.computeAbsoluteHumidity(dhtTemperatureInside,humidityInside));s+=F(" g/m³</b></p>");
  }
  s+=F("</section><section><p>Fan: <b>");s+=(fanRunning?"ON":"OFF");s+=F("</b> · Speed: <b>");
  s+=(fanHighSpeed?"570":"440");s+=F(" m³/h</b></p>");
  s+=F("<p>Control: <b>");s+=(manualMode?"MANUAL":"AUTO");s+=F("</b> · Season: <b>");
  s+=(summerMode?"SUMMER / Cooling":"WINTER / Heating");s+=F("</b></p>");
  if(!writeRootBytes(client,reinterpret_cast<const uint8_t*>(s.c_str()),s.length()))return false;
  s.remove(0);yield();
  s+=F("<p>RF switch: <b>");s+=(digitalRead(GPIO_RF)==HIGH?"ON":"OFF");
  s+=F("</b> · Speed source: <b>");s+=(speedOverrideActive?"WEB/MQTT override":"RF switch");s+=F("</b></p>");
  s+=F("<p>Control clock: <b>");s+=clockString();s+=F("</b> ");
  s+=(timeSynchronized?"(NTP)":"(not NTP synchronized)");s+=F("</p>");
  if(!writeRootBytes(client,reinterpret_cast<const uint8_t*>(s.c_str()),s.length()))return false;
  s.remove(0);yield();
  if(ventSafeMode)
    s+=F("<form method='post' action='/safe-reset'><button>Reset SafeMode (after repair)</button></form>");
  s+=F("<form method='post' action='/control'><button>Switch AUTO / MANUAL</button></form>");
  s+=F("<form method='post' action='/season'><button>Switch SUMMER / WINTER</button></form>");
  s+=F("<form method='post' action='/fan'><button>Fan ON / OFF (MANUAL)</button></form>");
  s+=F("<form method='post' action='/speed'><button>Speed 570 / 440 m³/h (AUTO &amp; MANUAL)</button></form>");
  if(!writeRootBytes(client,reinterpret_cast<const uint8_t*>(s.c_str()),s.length()))return false;
  s.remove(0);yield();
  s+=F("<fieldset><form method='post' action='/schedule'>Pause from <input name='from' size='5' value='");
  s+=hhmm(pauseFrom);s+=F("'> to <input name='to' size='5' value='");s+=hhmm(pauseUntil);
  s+=F("'><button>Save schedule</button></form></fieldset>");
  if(!timeSynchronized){
    s+=F("<fieldset><form method='post' action='/time'>Set time <input name='clock' size='8' value='");
    s+=clockString();s+=F("'><button>Set</button></form></fieldset>");
  }
  s+=F("<p><a href='/charts?view=24'>Γραφήματα θερμοκρασιών · 24 ώρες / 48 ώρες / 30 ημέρες</a></p></section>");
  return writeRootBytes(client,reinterpret_cast<const uint8_t*>(s.c_str()),s.length());
}
bool ventilationWebFinish() {
  WiFiClient client=server.client();
  if(!platformWebWriteStatus(client))return false;
  char stamp[32];
  formatCurrentDateTime(stamp,sizeof(stamp));
  return platformWebWriteFooter(client,stamp,false);
}

// Keep the history arrays off the shared footer's call stack.
bool ventilationRenderHistoryContent() {
  bool previous=server.arg("day")=="previous";
  float insideValues[96],outsideValues[96];
  if(previous){memcpy(insideValues,yesterdayInside,sizeof(insideValues));
    memcpy(outsideValues,yesterdayOutside,sizeof(outsideValues));}
  else{quarterAverages(insideValues,todayInside);quarterAverages(outsideValues,todayOutside);}
  if(!ventilationWebSend(F("<p><a href='/'>Back</a></p>"
    "<canvas id='graph' width='760' height='310' style='max-width:100%;background:white'></canvas>"
    "<div class='table-wrap'><table><tr><th>Time</th><th>Inside</th><th>Outside</th><th>Difference</th></tr>")))return false;
  for(int i=0;i<96;i++){
    String row="<tr><td>"+hhmm(i*15)+"</td><td>"+ventilationWebNumber(insideValues[i])+"</td><td>"+
       ventilationWebNumber(outsideValues[i])+"</td><td>";
    if(std::isfinite(insideValues[i])&&std::isfinite(outsideValues[i]))row+=String(insideValues[i]-outsideValues[i],1);
    else row+=F("—");
    row+="</td></tr>";
    if(!ventilationWebSend(row))return false;
    if(i%12==11)yield();
  }
  if(!ventilationWebSend(F("</table></div><script>const a=[")))return false;
  for(int i=0;i<96;i++){
    String pair="["+jsonNumber(insideValues[i])+","+jsonNumber(outsideValues[i])+"],";
    if(!ventilationWebSend(pair))return false;
  }
  return ventilationWebSend(F("];const c=document.getElementById('graph'),x=c.getContext('2d');"
    "const vals=a.flat().filter(Number.isFinite),lo=Math.min(...vals,0)-1,hi=Math.max(...vals,1)+1;"
    "function py(v){return 290-260*(v-lo)/(hi-lo)}"
    "function draw(k,color){x.strokeStyle=color;x.lineWidth=2;x.beginPath();let started=false;"
    "for(let i=0;i<a.length;i++){let y=a[i][k];if(y===null||!Number.isFinite(y)){started=false;continue;}"
    "let xx=30+i*7;started?x.lineTo(xx,py(y)):x.moveTo(xx,py(y));started=true;}x.stroke()}"
    "x.fillStyle='#555';x.fillText(hi.toFixed(1)+'C',2,15);x.fillText(lo.toFixed(1)+'C',2,305);"
    "draw(0,'#dc2626');draw(1,'#2563eb');x.fillStyle='#dc2626';x.fillText('Inside',45,15);"
    "x.fillStyle='#2563eb';x.fillText('Outside',120,15)</script>"));
}
void routeHistory() {
  WebResponseScope responseScope;
  if(!requireApWebPin())return;
  // EnergyMeter-like reject heavy web work before heap fragmentation can
  // exhaust TCP/TLS resources and interfere with active ArduinoOTA.
  if(!heapSuitable(4000UL,1024U)){
    ++webRootBusyCount;
    safeFlashResponse(503, "text/plain", PSTR("Not enough contiguous heap for history; retry later."));
    return;
  }
  markCrashStage(CRASH_STAGE_WEB_ROOT,ESP.getFreeHeap());
  WiFiClient client=server.client();
  if(!platformWebWriteHttpHeader(client) ||
     !platformWebWritePageStart(client,PSTR("Temperature history · 15-minute averages")))return;
  if(!ventilationRenderHistoryContent())return;
  ventilationWebFinish();
}

float quarterAt(const int16_t* samples,uint8_t q){
  float sum=0.0f;int n=0;
  for(int i=q*15;i<q*15+15;i++){
    float v=minuteTemperatureDecode(samples[i]);
    if(std::isfinite(v)&&v>=-55.0f&&v<=125.0f){sum+=v;n++;}
  }
  return n?sum/n:NAN;
}
void routeCharts(){
  WebResponseScope responseScope;
  if(!requireApWebPin())return;
  if(!heapSuitable(2850UL,850U)){
    safeFlashResponse(503, "text/plain", PSTR("Low heap: retry charts later"));return;
  }
  String view=server.arg("view");
  if(view!="24"&&view!="48"&&view!="30")view="24";
  WiFiClient client=server.client();
  if(!platformWebWriteHttpHeader(client) ||
     !platformWebWritePageStart(client,PSTR("Γραφήματα θερμοκρασίας")))return;
  if(!ventilationWebSendProgmem(PSTR(
    "  <nav><a href=\"/\">Αρχική</a><a href=\"/charts?view=24\">24 ώρες</a>\n"
    "  <a href=\"/charts?view=48\">48 ώρες</a><a href=\"/charts?view=30\">30 ημέρες</a></nav>\n"
    "  <div class=\"scroll\"><canvas id=\"graph\" width=\"1060\" height=\"525\"></canvas></div>\n"
    "  <p><b style=\"color:#dc2626\">Μέσα</b> · <b style=\"color:#2563eb\">Έξω</b>\n"
    "  · Σήμερα συνεχής γραμμή · Χθες διακεκομμένη</p>\n"
    "  <div class=\"scroll\" id=\"readings\"></div>\n"
    "  <p><small>°C · 24ωρα ανά 15 λεπτά · ημερήσιο ελάχιστο, μέγιστο, μέση τιμή</small></p>\n"
    "  <script>const view=\""
  )))return;
  if(!ventilationWebSend(view))return;
  if(!ventilationWebSendProgmem(PSTR(
    "\",today=["
  )))return;
  if(view!="30"){
    for(uint8_t i=0;i<96;i++){
      if(i)if(!ventilationWebSend(","))return;
      String r="[";
      r+=jsonNumber(quarterAt(todayInside,i));r+=",";
      r+=jsonNumber(quarterAt(todayOutside,i));r+="]";
      if(!ventilationWebSend(r))return;if(i%8==7)yield();
    }
  }
  if(!ventilationWebSendProgmem(PSTR(
    "],previous=["
  )))return;
  if(view=="48"){
    for(uint8_t i=0;i<96;i++){
      if(i)if(!ventilationWebSend(","))return;
      String r="[";r+=jsonNumber(yesterdayInside[i]);r+=",";
      r+=jsonNumber(yesterdayOutside[i]);r+="]";
      if(!ventilationWebSend(r))return;if(i%8==7)yield();
    }
  }
  if(!ventilationWebSendProgmem(PSTR(
    "],days=["
  )))return;
  if(view=="30"){
    for(uint8_t i=0;i<DAILY_COUNT;i++){
      if(i)if(!ventilationWebSend(","))return;
      String r="[";
      r+=dailyRow(i);r+="]";
      if(!ventilationWebSend(r))return;if(i%5==4)yield();
    }
  }
  if(!ventilationWebSendProgmem(PSTR(
    "];\n"
    "  const g=document.getElementById(\"graph\").getContext(\"2d\");\n"
    "  const fmt=v=>v==null||!Number.isFinite(v)?\"—\":Number(v).toLocaleString(\"el-GR\",{minimumFractionDigits:1,maximumFractionDigits:1});\n"
    "  const dt=k=>{if(!k)return \"—\";const y=Math.floor(k/400)+1900,d=k%400;\n"
    "    return new Date(Date.UTC(y,0,1)+d*86400000).toISOString().slice(0,10)};\n"
    "  const series=view===\"30\"?[\n"
    "    [days.map(r=>r[1]),\"#dc2626\",[4,4]],[days.map(r=>r[2]),\"#dc2626\",[4,4]],\n"
    "    [days.map(r=>r[3]),\"#dc2626\",[]],[days.map(r=>r[4]),\"#2563eb\",[4,4]],\n"
    "    [days.map(r=>r[5]),\"#2563eb\",[4,4]],[days.map(r=>r[6]),\"#2563eb\",[]]\n"
    "  ]:[\n"
    "    [today.map(r=>r[0]),\"#dc2626\",[]],[today.map(r=>r[1]),\"#2563eb\",[]],\n"
    "    ...(view===\"48\"?[\n"
    "      [previous.map(r=>r[0]),\"#dc2626\",[7,5]],\n"
    "      [previous.map(r=>r[1]),\"#2563eb\",[7,5]]\n"
    "    ]:[])\n"
    "  ];\n"
    "  const values=series.flatMap(a=>a[0]).filter(Number.isFinite);\n"
    "  const cv=g.canvas, L=65,T=30,W=960,H=440;\n"
    "  g.clearRect(0,0,cv.width,cv.height);\n"
    "  if(values.length){\n"
    "    let lo=Math.floor(Math.min(...values)*2)/2,hi=Math.ceil(Math.max(...values)*2)/2;\n"
    "    if(lo===hi){lo-=.5;hi+=.5}\n"
    "    const y=v=>T+H*(hi-v)/(hi-lo);\n"
    "    g.font=\"12px Arial\";g.lineWidth=1;g.textAlign=\"right\";\n"
    "    for(let half=Math.round(lo*2);half<=Math.round(hi*2);half++){\n"
    "      const v=half/2,yy=y(v);\n"
    "      g.strokeStyle=\"#dde3e8\";g.setLineDash([]);\n"
    "      g.beginPath();g.moveTo(L,yy);g.lineTo(L+W,yy);g.stroke();\n"
    "      g.fillStyle=\"#374151\";g.fillText(fmt(v),L-7,yy+3);\n"
    "    }\n"
    "    const N=view===\"30\"?30:96;\n"
    "    const x=i=>L+W*i/(view===\"30\"?29:96);\n"
    "    g.textAlign=\"center\";\n"
    "    if(view!==\"30\"){\n"
    "      for(let hour=0;hour<=24;hour++){\n"
    "        let xx=x(hour*4);\n"
    "        g.strokeStyle=\"#d6dde4\";g.beginPath();g.moveTo(xx,T);g.lineTo(xx,T+H);g.stroke();\n"
    "        if(hour<24){g.fillStyle=\"#374151\";g.fillText(String(hour).padStart(2,\"0\"),xx,T+H+22)}\n"
    "      }\n"
    "    }else{\n"
    "      for(let index=0;index<30;index+=3){\n"
    "        const xx=x(index);\n"
    "        g.strokeStyle=\"#e3e7ec\";g.beginPath();g.moveTo(xx,T);g.lineTo(xx,T+H);g.stroke();\n"
    "        g.fillStyle=\"#374151\";g.fillText(dt(days[index]?.[0]).slice(5),xx,T+H+22);\n"
    "      }\n"
    "    }\n"
    "    for(const [points,color,dash] of series){\n"
    "      g.strokeStyle=color;g.lineWidth=2.3;g.setLineDash(dash);\n"
    "      g.beginPath();let begun=false;\n"
    "      for(let i=0;i<points.length;i++){\n"
    "        const v=points[i];if(!Number.isFinite(v)){begun=false;continue}\n"
    "        let xx=x(i);begun?g.lineTo(xx,y(v)):g.moveTo(xx,y(v));begun=true;\n"
    "      }\n"
    "      g.stroke();\n"
    "    }\n"
    "    g.setLineDash([]);\n"
    "  }else{g.fillStyle=\"#374151\";g.fillText(\"Δεν υπάρχουν διαθέσιμες καταγραφές\",L+20,T+50)}\n"
    "  "
  )))return;
  if(!ventilationWebSendProgmem(PSTR(
    "\n"
    "  const holder=document.getElementById(\"readings\");\n"
    "  const head=view===\"30\"?\n"
    "    [\"Ημερομηνία\",\"Μέσα min °C\",\"Μέσα max °C\",\"Μέσα μέση °C\",\"Έξω min °C\",\"Έξω max °C\",\"Έξω μέση °C\"]:\n"
    "    view===\"48\"?[\"Ώρα\",\"Μέσα σήμερα °C\",\"Έξω σήμερα °C\",\"Μέσα χθες °C\",\"Έξω χθες °C\"]:\n"
    "    [\"Ώρα\",\"Μέσα °C\",\"Έξω °C\"];\n"
    "  const rows=view===\"30\"?days.map(r=>[dt(r[0]),...r.slice(1).map(fmt)]):\n"
    "    Array.from({length:96},(_,i)=>{\n"
    "      const time=String(Math.floor(i/4)).padStart(2,\"0\")+\":\"+String((i%4)*15).padStart(2,\"0\");\n"
    "      return view===\"48\"?[time,fmt(today[i]?.[0]),fmt(today[i]?.[1]),\n"
    "        fmt(previous[i]?.[0]),fmt(previous[i]?.[1])]:\n"
    "        [time,fmt(today[i]?.[0]),fmt(today[i]?.[1])];\n"
    "    });\n"
    "  holder.innerHTML=\"<table><thead><tr>\"+head.map(v=>\"<th>\"+v+\"</th>\").join(\"\")+\n"
    "    \"</tr></thead><tbody>\"+rows.map(r=>\"<tr>\"+r.map(v=>\"<td>\"+v+\"</td>\").join(\"\")+\n"
    "    \"</tr>\").join(\"\")+\"</tbody></table>\";\n"
    "  </script>"
  )))return;
  ventilationWebFinish();
}

void applicationConfigureWebRoutes() {
  server.on("/history",HTTP_GET,routeHistory);
  server.on("/charts",HTTP_GET,routeCharts);
  server.on("/fan",HTTP_POST,[](){
    WebResponseScope responseScope;
    if(!localControlAuthorized())return;
    if(manualMode){manualFan=!manualFan;applyFanControl();}
    redirectHome();
  });
  server.on("/speed",HTTP_POST,[](){
    WebResponseScope responseScope;
    if(!localControlAuthorized())return;
    manualHighSpeed=!fanHighSpeed;speedOverrideActive=true;
    applyFanControl();redirectHome();
  });
  server.on("/control",HTTP_POST,[](){
    WebResponseScope responseScope;
    if(!localControlAuthorized())return;
    manualMode=!manualMode;
    if(manualMode){manualFan=fanRunning;manualHighSpeed=fanHighSpeed;}
    applyFanControl();redirectHome();
  });
  server.on("/safe-reset",HTTP_POST,[](){
    WebResponseScope responseScope;
    if(!localControlAuthorized())return;
    if(!ventTryRearmSafeMode()){
      safeFlashResponse(409, "text/plain", PSTR("SafeMode remains active: both critical DS18B20 need 3 consecutive valid samples."));
      return;
    }
    applyFanControl();
    redirectHome();
  });
  server.on("/season",HTTP_POST,[](){
    WebResponseScope responseScope;
    if(!localControlAuthorized())return;
    summerMode=!summerMode;applyFanControl();redirectHome();
  });
  server.on("/schedule",HTTP_POST,[](){
    WebResponseScope responseScope;
    if(!localControlAuthorized())return;
    int a,b;
    if(parseClock(server.arg("from"),false,a)&&parseClock(server.arg("to"),false,b)){
      pauseFrom=a/60;pauseUntil=b/60;applyFanControl();
    }
    redirectHome();
  });
  server.on("/time",HTTP_POST,[](){
    WebResponseScope responseScope;
    if(!localControlAuthorized())return;
    if(timeSynchronized){safeFlashResponse(409, "text/plain", PSTR("NTP already synchronized"));return;}
    int seconds;
    if(parseClock(server.arg("clock"),true,seconds))setManualClock(seconds);
    redirectHome();
  });
}


void applicationEarlyInitialize() {
  // GPIO1 is the wired RF input. Never initialize the UART.
  digitalWrite(GPIO_FAN,FAN_OFF_LEVEL);
  pinMode(GPIO_FAN,OUTPUT);
  digitalWrite(GPIO_SPEED,SPEED_HIGH_LEVEL);
  pinMode(GPIO_SPEED,OUTPUT);
  pinMode(GPIO_RF,INPUT);
  pinMode(GPIO_MAINS,INPUT);
  pinMode(GPIO_LED,OUTPUT);
  fanRunning=false;
  fanHighSpeed=true;
}

void applicationInitialize() {
  if(ventilationInitialized)return;
  for(int i=0;i<1440;++i)todayInside[i]=todayOutside[i]=HISTORY_EMPTY;
  for(int i=0;i<96;++i)yesterdayInside[i]=yesterdayOutside[i]=NAN;
  ds.begin();
  ds.setWaitForConversion(false);
  dhtOutside.begin();dhtInside.begin();
  historyLoadLittleFS();
  dailyInit();dailyLoad();
  lastClockTick=millis();
  ventilationInitialized=true;
}

void applicationInitializeTopics() {
  snprintf(VENT_HISTORY_TOPIC,sizeof(VENT_HISTORY_TOPIC),MQTT_TOPIC_PREFIX "%s/vent/history",mqttDeviceId);
}

void applicationServiceLocal(unsigned long now) {
  if(!ventilationInitialized || ventilationOtaPaused)return;
  updateClock();
  sampleHardware();
  applyFanControl();
  logTemperatureMinute();
  if(now-lastLedAt>=500UL){lastLedAt=now;digitalWrite(GPIO_LED,!digitalRead(GPIO_LED));}
}

bool applicationReadMeasurements() {
  applicationServiceLocal(millis());
  return sensorsOk;
}

void applicationRenderMeasurements(bool) {
  // Supplied ventilation wiring reserves GPIO4 for OneWire: this profile has no OLED.
}

bool applicationDualZoneEnabled() { return false; }

bool applicationStageCommand(const String& message) {
  if(!message.startsWith("vent_"))return false;
  if(message.length()==0 || message.length()>=sizeof(commandBuf)){
    ventilationCommandInvalid=true;
    mqttAdminCommand=MQTT_ADMIN_VENTILATION;
    mqttAdminCommandPending=true;
    return true;
  }
  ventilationCommandInvalid=false;
  message.toCharArray(commandBuf,sizeof(commandBuf));
  commandReady=true;
  mqttAdminCommand=MQTT_ADMIN_VENTILATION;
  mqttAdminCommandPending=true;
  return true;
}

bool applicationProcessCommand(MqttAdminCommand command) {
  if(command!=MQTT_ADMIN_VENTILATION)return false;
  if(ventilationCommandInvalid){
    publishAck(false,"vent_command","invalid_value");
    return true;
  }
  processCommand();
  return true;
}

void applicationResetCommand() {
  commandReady=false;
  ventilationCommandInvalid=false;
  commandBuf[0]='\0';
}

void applicationBackground(unsigned long) {
  if(!ventilationInitialized || ventilationOtaPaused)return;
  if(historyStreaming)streamHistoryMQTT();
}

void applicationPrepareOta() {
  // Latch local processing paused so a sensor refresh cannot turn the fan on.
  ventilationOtaPaused=true;
  fanRunning=false;
  digitalWrite(GPIO_FAN,FAN_OFF_LEVEL);
  if(ventilationInitialized)historySaveLittleFS();
}

void applicationFormatSummary(char* out,size_t capacity) {
  snprintf(out,capacity,"VENT fan=%s %s/%s",fanRunning?"ON":"OFF",
    manualMode?"MANUAL":"AUTO",summerMode?"SUMMER":"WINTER");
}

bool applicationAppendDebug(WiFiClient& client) {
  char line[360];
  int n=snprintf(line,sizeof(line),
    "\nVentilation: GPIO fan=%u speed=%u RF=%u mains=%u\n"
    "Control=%s season=%s fan=%s speed=%s sensors=%s clock=%s\n"
    "SafeMode=%s failed: Outside=%u Inside=%u Middle=%u DHT-out-temp=%u DHT-out-hum=%u DHT-in-temp=%u DHT-in-hum=%u\n"
    "History loaded=%s sequence=%lu writes=%lu failures=%lu dailywrites=%lu dailyerrors=%lu\n",
    (unsigned)GPIO_FAN,(unsigned)GPIO_SPEED,(unsigned)GPIO_RF,(unsigned)GPIO_MAINS,
    manualMode?"MANUAL":"AUTO",summerMode?"SUMMER":"WINTER",fanRunning?"ON":"OFF",
    fanHighSpeed?"HIGH":"LOW",sensorsOk?"OK":"INVALID",timeSynchronized?"NTP":"LOCAL",
    ventSafeMode?"ON":"OFF",
    (unsigned)ventProbeFails[DS_OUTSIDE],(unsigned)ventProbeFails[DS_INSIDE],
    (unsigned)ventProbeFails[DS_MIDDLE],(unsigned)ventProbeFails[DHT_OUT_TEMP],
    (unsigned)ventProbeFails[DHT_OUT_HUM],(unsigned)ventProbeFails[DHT_IN_TEMP],
    (unsigned)ventProbeFails[DHT_IN_HUM],
    historyLoaded?"YES":"NO",(unsigned long)historySequence,(unsigned long)historyWrites,
    (unsigned long)historyWriteFailures,(unsigned long)dailyWrites,(unsigned long)dailyWriteErrors);
  if(n<=0||n>=(int)sizeof(line)||
     !writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n))return false;
  n=snprintf(line,sizeof(line),
    "Local completed samples=%lu max gap ms=%lu last sample age ms=%lu DS conversion pending=%s\n"
    "Min observed heap=%lu largest block=%u stack=%lu\n",
    (unsigned long)ventCompletedSampleCount,
    (unsigned long)ventMaximumSampleGapMs,
    (unsigned long)(ventLastSampleCompletedMs?millis()-ventLastSampleCompletedMs:0UL),
    ventDsConversionPending?"YES":"NO",
    (unsigned long)(localMinimumHeapBytes==0xFFFFFFFFUL?0:localMinimumHeapBytes),
    (unsigned)(localMinimumBlockBytes==0xFFFFU?0:localMinimumBlockBytes),
    (unsigned long)(localMinimumStackBytes==0xFFFFFFFFUL?0:localMinimumStackBytes));
  if(n<=0||n>=(int)sizeof(line)||
     !writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n))return false;
  n=snprintf(line,sizeof(line),
    "DHT out reads=%lu in reads=%lu low-memory skips=%lu failures=%lu last ms=%lu\\n"
    "DHT pre-read stack=%lu heap=%lu max block=%lu next=%s\\n",
    (unsigned long)ventDhtOutsideReads,(unsigned long)ventDhtInsideReads,
    (unsigned long)ventDhtLowMemorySkips,(unsigned long)ventDhtReadFailures,
    (unsigned long)ventDhtLastDurationMs,
    (unsigned long)ventDhtLastStackBytes,(unsigned long)ventDhtLastHeapBytes,
    (unsigned long)ventDhtLastMaxBlockBytes,
    ventDhtOutsideNext?"outside":"inside");
  return n>0 && n<(int)sizeof(line) &&
    writeRootBytes(client,reinterpret_cast<const uint8_t*>(line),(size_t)n);
}

uint32_t legacyVentCredentialChecksum(const LegacyVentCredentialRecord& record) {
  const uint8_t* bytes=reinterpret_cast<const uint8_t*>(&record);
  uint32_t value=2166136261UL;
  for(size_t i=0;i<offsetof(LegacyVentCredentialRecord,checksum);++i){value^=bytes[i];value*=16777619UL;}
  return value;
}

bool applicationImportLegacyCredentials() {
  if(!littleFsReady || LittleFS.exists(DEVICE_CREDENTIAL_FILE))return false;
  File file=LittleFS.open("/vent-credentials.bin","r");
  if(!file)return false;
  LegacyVentCredentialRecord old={};
  bool ok=file.size()==sizeof(old) &&
    file.readBytes(reinterpret_cast<char*>(&old),sizeof(old))==sizeof(old);
  file.close();
  if(!ok || old.magic!=0x56454E54UL || old.version!=1 ||
     old.checksum!=legacyVentCredentialChecksum(old) ||
     !credentialFieldIsValid(old.ssid,sizeof(old.ssid)) ||
     !credentialFieldIsValid(old.wifiPassword,sizeof(old.wifiPassword)) ||
     !credentialFieldIsValid(old.mqttUser,sizeof(old.mqttUser)) ||
     !credentialFieldIsValid(old.mqttPassword,sizeof(old.mqttPassword)))return false;
  DeviceCredentialRecord converted={};
  converted.magic=DEVICE_CREDENTIAL_MAGIC;
  converted.version=DEVICE_CREDENTIAL_VERSION;
  converted.mqttPort=MQTT_BROKER_PORT;
  if(!copyCredentialField(converted.ssid,sizeof(converted.ssid),old.ssid) ||
     !copyCredentialField(converted.wifiPassword,sizeof(converted.wifiPassword),old.wifiPassword) ||
     !copyCredentialField(converted.mqttHost,sizeof(converted.mqttHost),MQTT_BROKER_HOST) ||
     !copyCredentialField(converted.mqttUser,sizeof(converted.mqttUser),old.mqttUser) ||
     !copyCredentialField(converted.mqttPassword,sizeof(converted.mqttPassword),old.mqttPassword))return false;
  converted.checksum=deviceCredentialChecksum(converted);
  // Keep the old record as a recovery source until the shared staged write verifies.
  return writeDeviceCredentialRecord(converted) && applyDeviceCredentials(converted);
}

bool applicationClearLegacyCredentials() {
  if(!littleFsReady)return false;
  const char* paths[]={"/vent-credentials.bin","/vent-credentials.tmp"};
  for(const char* path:paths)
    if(LittleFS.exists(path) && !LittleFS.remove(path))return false;
  return true;
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
#if OLED_TYPE == 2 || OLED_TYPE == 3
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
