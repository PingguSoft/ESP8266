/*
 This project is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 see <http://www.gnu.org/licenses/>
*/

#include <Arduino.h>
#include <stdarg.h>
#include <string.h>
#include "Commands.h"
#include "Utils.h"
#include "ByteArray.h"

Commands::Commands(char *host, int port)
{
    mStrHost = host;
    mPort    = port;
    mCfgIdx  = 0;
}

Commands::~Commands()
{
    mUDP.stop();
}

void Commands::sendto(u8 *data, int size)
{
    mUDP.beginPacket(mStrHost, mPort);
    mUDP.write(data, size);
    mUDP.endPacket();

    //Utils::dump(data, size);
    //Serial.printf("-------------------------TX END -----------------------\n\n");
}

void Commands::move(u8 enRollPitch, s8 roll, s8 pitch, s8 yaw, s8 gaz)
{
    mEnRollPitch = enRollPitch;
    mRoll  = roll;
    mPitch = pitch;
    mYaw   = yaw;
    mGaz   = gaz;
}

void Commands::enableVideoAutoRecording(u8 enable, u8 storage)
{
    PRINT_FUNC;
    int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 10, "BBHBB", ARDRONE3, CLASS_SETPICTURE, 5, enable, storage);
    sendto(mBuf, size);
}

void Commands::takePicture(u8 storage)
{
    PRINT_FUNC;
    int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 10, "BBHB", ARDRONE3, CLASS_MEDIARECORD, 0, storage);
    sendto(mBuf, size);
}

void Commands::recordVideo(u8 enable, u8 storage)
{
    PRINT_FUNC;
    int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 10, "BBHBB", ARDRONE3, CLASS_MEDIARECORD, 1, enable, storage);
    sendto(mBuf, size);
}

void Commands::setDate(void)
{
    PRINT_FUNC;
    int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 10, "BBHS", ARDRONE3, CLASS_COMMON, 1, "2016-04-15");
    sendto(mBuf, size);
}

void Commands::setTime(void)
{
    PRINT_FUNC;
    int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 10, "BBHS", ARDRONE3, CLASS_COMMON, 2, "T185603+0000");
    sendto(mBuf, size);
}

void Commands::enableVideoStreaming(u8 enable)
{
    PRINT_FUNC;
    int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 11, "BBHB", ARDRONE3, CLASS_STREAMING, 0, enable);
    sendto(mBuf, size);
}

void Commands::moveCamera(s8 tilt, s8 pan)
{
    PRINT_FUNC;
    int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 10, "BBHBB", ARDRONE3, CLASS_CAMERA, 0, tilt, pan);
    sendto(mBuf, size);
}

void Commands::takeOff(void)
{
    PRINT_FUNC;
    int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 10, "BBH", ARDRONE3, CLASS_PILOTING, 1);
    sendto(mBuf, size);
}

void Commands::land(void)
{
    PRINT_FUNC;
    int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 10, "BBH", ARDRONE3, CLASS_PILOTING, 3);
    sendto(mBuf, size);
}
void Commands::emergency(void)
{
    PRINT_FUNC;
    int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 10, "BBH", ARDRONE3, CLASS_PILOTING, 4);
    sendto(mBuf, size);
}
void Commands::trim(void)
{
    PRINT_FUNC;
    int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 10, "BBH", ARDRONE3, CLASS_PILOTING, 0);
    sendto(mBuf, size);
}
void Commands::requestSettings(void)
{
    PRINT_FUNC;
    int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 10, "BBH", ARDRONE3, CLASS_SETTING,  0);
    sendto(mBuf, size);
}
void Commands::requestStates(void)
{
    PRINT_FUNC;
    int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 10, "BBH", ARDRONE3, CLASS_COMMON,   0);
    sendto(mBuf, size);
}
void Commands::resetHome(void)
{
    PRINT_FUNC;
    int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 10, "BBH", ARDRONE3, CLASS_GPS,      1);
    sendto(mBuf, size);
}

bool Commands::config(void)
{
    bool done = false;

    switch (mCfgIdx) {
        case 0: setDate();          break;
        case 1: setTime();          break;
        case 2: requestStates();    break;
        case 3: requestSettings();  break;
        case 4: moveCamera(0, 0);   break;
        case 5: enableVideoAutoRecording(1); break;
        case 6: enableVideoStreaming(1);
            done = true;
            break;
    }
    mCfgIdx++;

    return done;
}

void Commands::process(u8 *dataAck, int size)
{
    long ts = millis();
    int  diff = ts - mLastTS;

    if (size > 0) {
        sendto(dataAck, size);
    }

    // send PCMD every 25ms
    if (diff >= 25) {
        u8  flag = 0;

        if (mEnRollPitch)
            flag = 1;

        //Serial.println("PCMD");
        int size = Bebop::buildCmd(mBuf, FRAME_TYPE_DATA, 10, "BBHBbbbbI", ARDRONE3, CLASS_PILOTING, 2, flag, mRoll, mPitch, mYaw, mGaz, 0);
        sendto(mBuf, size);

        mLastTS = ts;
    }
}

