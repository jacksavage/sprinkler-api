#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <vector>
#include "config.h"

AsyncWebServer server(80);

struct ScheduleItem {
    int zone_id;
    float duration_seconds;
};

TaskHandle_t scheduleTaskHandle = nullptr;
volatile bool stopFlag = false;

void allZonesOff() {
    for (int i = 1; i <= NUM_ZONES; i++) {
        digitalWrite(ZONE_PINS[i], LOW);
    }
}

void runScheduleTask(void* param) {
    auto* items = (std::vector<ScheduleItem>*)param;

    for (const auto& item : *items) {
        if (stopFlag) break;
        if (item.zone_id < 1 || item.zone_id > NUM_ZONES) continue;

        allZonesOff();
        digitalWrite(ZONE_PINS[item.zone_id], HIGH);

        uint32_t duration_ms = (uint32_t)(item.duration_seconds * 1000.0f);
        uint32_t elapsed = 0;
        while (elapsed < duration_ms && !stopFlag) {
            vTaskDelay(pdMS_TO_TICKS(100));
            elapsed += 100;
        }
    }

    allZonesOff();
    delete items;
    scheduleTaskHandle = nullptr;
    vTaskDelete(nullptr);
}

// Stop any running schedule and turn all zones off.
// Safe to call from the async web server task context.
void stopCurrentSchedule() {
    if (scheduleTaskHandle == nullptr) return;

    stopFlag = true;
    // Wait up to 1 s for the task to exit cleanly
    for (int i = 0; i < 10 && scheduleTaskHandle != nullptr; i++) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    // Force-kill if it hasn't exited
    if (scheduleTaskHandle != nullptr) {
        vTaskDelete(scheduleTaskHandle);
        scheduleTaskHandle = nullptr;
    }

    allZonesOff();
    stopFlag = false;
}

void setup() {
    Serial.begin(115200);

    // Initialise zone pins, all off
    for (int i = 1; i <= NUM_ZONES; i++) {
        pinMode(ZONE_PINS[i], OUTPUT);
        digitalWrite(ZONE_PINS[i], LOW);
    }

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());

    // Add CORS headers to every response
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    // CORS preflight
    server.on("/api/schedule", HTTP_OPTIONS, [](AsyncWebServerRequest* request) {
        request->send(204);
    });

    // POST /api/schedule
    // Body chunks are accumulated into a String stored on request->_tempObject,
    // then parsed once the full body has arrived.
    server.on(
        "/api/schedule",
        HTTP_POST,
        // Called after the full body has been received
        [](AsyncWebServerRequest* request) {
            if (!request->_tempObject) {
                request->send(400, "application/json", "{\"error\":\"Empty body\"}");
                return;
            }

            auto* bodyStr = (String*)request->_tempObject;
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, *bodyStr);
            delete bodyStr;
            request->_tempObject = nullptr;

            if (err) {
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }

            stopCurrentSchedule();

            JsonArray arr = doc.as<JsonArray>();
            if (arr.size() == 0) {
                request->send(200, "application/json", "{\"message\":\"All zones stopped\"}");
                return;
            }

            auto* items = new std::vector<ScheduleItem>();
            for (JsonObject obj : arr) {
                ScheduleItem item;
                item.zone_id = obj["zone_id"].as<int>();
                item.duration_seconds = obj["duration_seconds"].as<float>();
                items->push_back(item);
            }

            xTaskCreate(runScheduleTask, "schedule", 4096, items, 1, &scheduleTaskHandle);
            request->send(200, "application/json", "{\"message\":\"Schedule started\"}");
        },
        nullptr, // upload handler (unused)
        // Body chunk handler
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            if (index == 0) {
                request->_tempObject = new String();
                ((String*)request->_tempObject)->reserve(total);
            }
            ((String*)request->_tempObject)->concat((char*)data, len);
        }
    );

    server.onNotFound([](AsyncWebServerRequest* request) {
        request->send(404, "application/json", "{\"error\":\"Not found\"}");
    });

    server.begin();
    Serial.println("Server started");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi lost, reconnecting...");
        WiFi.reconnect();
    }
    delay(10000);
}
