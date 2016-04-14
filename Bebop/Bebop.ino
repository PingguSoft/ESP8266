#include <Arduino.h>
#include <stdarg.h>
#include <ESP8266WiFi.h>

enum {
    STATE_INIT = 0,
    STATE_AP_CONNECT,
    STATE_DISCOVERY,
    STATE_DISCOVERY_ACK,
};

static WiFiClient mClient;
static int mNextState = STATE_INIT;


void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.printf("Ready !!!\n");
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);

    switch(event) {
        case WIFI_EVENT_STAMODE_GOT_IP:
            Serial.println("WiFi connected");
            Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
            mNextState = STATE_DISCOVERY;
            break;


        case WIFI_EVENT_STAMODE_DISCONNECTED:
            Serial.println("WiFi lost connection");
            break;
    }
}

void loop() {
    switch (mNextState) {
        case STATE_INIT:
            int n = WiFi.scanNetworks();

            if (n == 0) {
                Serial.printf("no networks found\n");
            } else {
                Serial.printf("%d networks found\n", n);
                for (int i = 0; i < n; i++) {
                    Serial.printf("%d : %s (%d) %d\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.encryptionType(i));
                    if (!strncmp(WiFi.SSID(i).c_str(), "BebopDrone", 10)) {
                        WiFi.onEvent(WiFiEvent);
                        WiFi.begin(WiFi.SSID(i).c_str(), "");
                        mNextState = STATE_AP_CONNECT;
                        Serial.printf("Connect to BebopDrone !!!\n");
                        break;
                    }
                }
            }
            break;

        case STATE_DISCOVERY:
            Serial.printf("Connect to discovery socket !!!\n");

            if (!mClient.connect("192.168.42.1", 44444)) {
                Serial.printf("Connection Failed !!!\n");
            } else {
                char *req = "{\"controller_type\":\"computer\", \"controller_name\":\"katarina\", \"d2c_port\":\"43210\"}";
                mClient.print(req);
                mClient.flush();
                mNextState = 3;
            }
            break;

        case STATE_DISCOVERY_ACK:
            Serial.printf("Waiting response !!!\n");

            while (mClient.available()) {
                byte buf[256];
                int len = mClient.read(buf, 256);
                if (len > 0) {
                    Serial.printf("%d %s\n", len, (char*)buf);
                    mNextState = 4;
                }
                //{ "status": 0, "c2d_port": 54321, "arstream_fragment_size": 65000, "arstream_fragment_maximum_number": 4, "arstream_max_ack_interval": -1, "c2d_update_port": 51, "c2d_user_port": 21 }
            }
            break;
    }

    delay(100);
}

