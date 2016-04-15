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


Commands::Commands()
{
    memset(mSeqID, 0, sizeof(mSeqID));
}

Commands::~Commands()
{
}


//    0          1       2      3 4 5 6       7              8        9 10     11
// frametype, frameid, seqid, payloadlen+7    payload...
//                                            prj,           cls,     cmd,     args

int Commands::buildCmd(u8 *buf, u8 prj, u8 cls, u16 cmd, char *fmt, ...)
{
    va_list valist;
    u8      frameid = 10;
    u8      seqid;
    u8      vu8;
    u16     vu16;
    u32     vu32;
    float   vf;
    int     idx;
    u32     size;

    if (prj == ARDRONE3 && cls == CLASS_STREAMING && cmd == STREAM_VIDEO) {
        frameid = 11;   // ack req
        seqid   = mSeqID[1]++;
    } else {
        frameid = 10;
        seqid   = mSeqID[0]++;
    }

    buf[0]  = 2;
    buf[1]  = frameid;
    buf[2]  = seqid;
    buf[7]  = prj;
    buf[8]  = cls;
    buf[9]  = (cmd & 0xff);
    buf[10] = (cmd & 0xff00) >> 8;
    idx     = 11;
    size    = 0;

    if (fmt) {
        va_start(valist, fmt);

        for (int i = 0; i < strlen(fmt); i++) {
            switch (fmt[i]) {
                case 'b':
                case 'B':
                    buf[idx++] = va_arg(valist, u8);
                    size = 1;
                    break;

               case 'h':
               case 'H':
                    vu16 = va_arg(valist, u16);
                    size = sizeof(s16);
                    buf[idx]     = (vu16 & 0xff);
                    buf[idx + 1] = (vu16 & 0xff00) >> 8;
                    break;

               case 'i':
               case 'I':
                    vu32 = va_arg(valist, u32);
                    size = sizeof(s32);
                    memcpy(&buf[idx], &vu32, size);
                    break;

               case 'f':
                    vf   = va_arg(valist, float);
                    size = sizeof(float);
                    memcpy(&buf[idx], &vf, size);
                    break;

               default:
                    size = 0;
                    Serial.printf("ABNORMAL FORMAT !!!  : %c\n", fmt[i]);
                    break;
            }
            idx += size;
        }
        va_end(valist);
    }

    size = idx + size;
    memcpy(&buf[3], &size, sizeof(u32));

    return size;
}


int Commands::buildCmd(u8 *buf, u8 prj, u8 cls, u16 cmd)
{
    return buildCmd(buf, prj, cls, cmd, NULL);
}

void Commands::move(u8 enRollPitch, s8 roll, s8 pitch, s8 yaw, s8 gaz)
{
    buildCmd(mBuf, ARDRONE3, CLASS_PILOTING, CMD_PCMD, "bbbbI", roll, pitch, yaw, gaz, 0);
}

void Commands::enableVideoAutoRecording(u8 enable, u8 storage)
{   
    buildCmd(mBuf, ARDRONE3, CLASS_SETPICTURE, PIC_VIDEO_AUTORECORD, "BB", enable, storage);
}

void Commands::takePicture(u8 storage)
{   
    buildCmd(mBuf, ARDRONE3, CLASS_MEDIARECORD, MEDIA_PICTURE, "B", storage);
}

void Commands::recordVideo(u8 enable, u8 storage)
{   
    buildCmd(mBuf, ARDRONE3, CLASS_MEDIARECORD, MEDIA_VIDEO, "BB", enable, storage);
}
