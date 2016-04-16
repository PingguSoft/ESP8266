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

#ifndef _BEBOP_H_
#define _BEBOP_H_

#include <Arduino.h>
#include <stdarg.h>
#include <string.h>
#include "Common.h"
#include "Utils.h"

// http://robotika.cz/robots/katarina/en#150202
// https://github.com/robotika/katarina
// https://github.com/Parrot-Developers/libARCommands/blob/master/Xml/ARDrone3_commands.xml

enum {
    FRAME_TYPE_ACK = 0x1,
    FRAME_TYPE_DATA = 0x2,
    FRAME_TYPE_DATA_LOW_LATENCY = 0x3,
    FRAME_TYPE_DATA_WITH_ACK = 0x4,
};

enum {
    BUFFER_ID_PING = 0,
    BUFFER_ID_PONG = 1
};

enum {
    ARDRONE3 = 1,
};

enum frameid {
    FRAME_NO_ACK  = 10,
    FRAME_ACK_REQ = 11,
};

enum {
    CLASS_PILOTING  = 0,
    CLASS_CAMERA    = 1,
    CLASS_SETTING   = 2,
    CLASS_COMMON    = 4,
    CLASS_ANIMATION = 5,
    CLASS_MEDIARECORD = 7,
    CLASS_MEDIASTORAGE = 8,
    CLASS_SPEED        = 11,
    CLASS_SETPICTURE = 19,
    CLASS_STREAMING  = 21,
    CLASS_GPS        = 23,
};

#define PACK_CMD(prj, cls, cmd) ((prj << 24) | (cls << 16) | (cmd))
#define PACK_PRJ_CLS(prj, cls)  ((prj << 24) | (cls << 16))

#define GET_PRJ_CLS(id)         ((id) & 0xffff0000)
#define GET_PRJ(id)             ((id >> 24) & 0xff)
#define GET_CLS(id)             ((id >> 16) & 0xff)
#define GET_CMD(id)             ((id) & 0xffff)

class Bebop {

public:
//    0          1       2      3 4 5 6       7              8        9 10     11
// frametype, frameid, seqid, payloadlen+7    payload...
//                                            prj,           cls,     cmd,     args

    static int buildCmd(u8 *buf, u8 ft, u8 fi, const char *fmt, ...)
    {
        u8      vu8;
        u16     vu16;
        u32     vu32;
        float   vf;
        int     idx;
        u32     size;
        char    *str;
        u64     vu64;

        buf[0]  = ft;
        buf[1]  = fi;
        buf[2]  = mSeqID[fi]++;

        idx     = 7;
        size    = 0;

        if (fmt) {
            va_list ap;

            va_start(ap, fmt);
            for (int i = 0; i < strlen(fmt); i++) {
                switch (fmt[i]) {
                    case 'b':
                    case 'B':
                        buf[idx] = va_arg(ap, u32);
                        size = 1;
                        break;

                   case 'h':
                   case 'H':
                        vu16 = va_arg(ap, u32);
                        size = Utils::put16(&buf[idx], vu16);
                        break;

                   case 'i':
                   case 'I':
                   case 'l':
                   case 'L':
                        vu32 = va_arg(ap, u32);
                        size = Utils::put32(&buf[idx], vu32);
                        break;

                   case 'q':
                   case 'Q':
                        vu64 = va_arg(ap, u64);
                        size = Utils::putlonglong(&buf[idx], vu64);
                        break;

                   case 'f':
                        vf   = va_arg(ap, float);
                        size = Utils::putfloat(&buf[idx], vf);
                        break;

                   case 's':
                   case 'S':
                        str  = va_arg(ap, char*);
                        size = Utils::putstr(&buf[idx], str);
                        break;

                   case 'p':
                   case 'P':
                        size = va_arg(ap, u32);
                        str  = va_arg(ap, char*);
                        memcpy(&buf[idx], str, size);
                        break;

                   default:
                        size = 0;
                        Serial.printf("ABNORMAL FORMAT !!!  : %c\n", fmt[i]);
                        break;
                }
                idx += size;
            }
            va_end(ap);
        }
        Utils::put32(&buf[3], idx);

        return idx;
    }

private:
    static u8  mSeqID[256];
};

#endif
