#include <Arduino.h>
#include <stdarg.h>
#include <ESP8266WiFi.h>
#include "Commands.h"
#include "Receiver.h"

enum {
    STATE_INIT = 0,
    STATE_AP_CONNECT,
    STATE_DISCOVERY,
    STATE_DISCOVERY_ACK,
    STATE_CONFIG,
    STATE_WORK,
};

u8  Bebop::mSeqID[256];

static char *HOST = "192.168.42.1";

static WiFiClient mClient;
static int mNextState = STATE_INIT;
static Commands mCmd(HOST, 54321);
static Receiver mRcv(43210);

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


static u8 dataAck[1024];


static u8 recVideo = 0;
static s8 speed = 0;
static s8 roll = 0;
static s8 pitch = 0;
static s8 yaw = 0;

void loop() {
    int size;

    switch (mNextState) {
        case STATE_INIT:
        {
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
        }
        break;

        case STATE_DISCOVERY:
        {
            Serial.printf("Connect to discovery socket !!!\n");

            if (!mClient.connect(HOST, 44444)) {
                Serial.printf("Connection Failed !!!\n");
            } else {
                char *req = "{\"controller_type\":\"computer\", \"controller_name\":\"katarina\", \"d2c_port\":\"43210\"}";
                mClient.print(req);
                mClient.flush();
                mNextState = 3;
            }
        }
        break;

        case STATE_DISCOVERY_ACK:
        {
            Serial.printf("Waiting response !!!\n");

            while (mClient.available()) {
                byte buf[256];
                int len = mClient.read(buf, 256);
                if (len > 0) {
                    Serial.printf("%d %s\n", len, (char*)buf);
                    mRcv.begin();
                    mNextState = STATE_CONFIG;
                }
                //{ "status": 0, "c2d_port": 54321, "arstream_fragment_size": 65000, "arstream_fragment_maximum_number": 4, "arstream_max_ack_interval": -1, "c2d_update_port": 51, "c2d_user_port": 21 }
            }
        }
        break;

        case STATE_CONFIG:
        {
            if (mCmd.config())
                mNextState = STATE_WORK;
            size = mRcv.process(dataAck);
            if (size > 0)
                mCmd.process(dataAck, size);
        }
        break;

        case STATE_WORK:
        {
            size = mRcv.process(dataAck);
            mCmd.process(dataAck, size);
        }
        break;
    }

    size = Serial.available();
    while(size--) {
        u8 ch = Serial.read();
        Serial.printf("***** : %c\n", ch);

        switch (ch) {
            case 'p' : mCmd.takePicture();                                  break;
            case 'v' : recVideo = !recVideo;    mCmd.recordVideo(recVideo); break;

            case 'a' : mCmd.takeOff();                                      break;
            case 'z' : mCmd.land();                                         break;

            case 's' :
                speed += (speed < 90) ? 10 : 0;
                Serial.printf("thr : %d\n", speed);
                break;

            case 'x' :
                speed += (speed > -90) ? -10 : 0;
                Serial.printf("thr : %d\n", speed);
                break;

            case 'i' :
                pitch += (pitch < 90) ? 10 : 0;
                Serial.printf("pitch : %d\n", pitch);
                break;

            case 'k' :
                pitch += (pitch > -90) ? -10 : 0;
                Serial.printf("pitch : %d\n", pitch);
                break;

            case 'j' :
                roll += (roll > -90) ? -10 : 0;
                Serial.printf("roll : %d\n", roll);
                break;

            case 'l' :
                roll += (roll < 90) ? 10 : 0;
                Serial.printf("roll : %d\n", roll);
                break;

            case 'u' :
                yaw += (yaw > -90) ? -10 : 0;
                Serial.printf("yaw : %d\n", yaw);
                break;

            case 'o' :
                yaw += (yaw < 90) ? 10 : 0;
                Serial.printf("yaw : %d\n", yaw);
                break;

            case ' ' :
                yaw = 0;
                roll = 0;
                pitch = 0;
                speed = 0;
                Serial.printf("reset pos\n");
                break;
        }
    }
    u8 flag = 0;

    if (roll != 0 || pitch != 0)
        flag = 1;
    mCmd.move(flag, roll, pitch, yaw, speed);
}

