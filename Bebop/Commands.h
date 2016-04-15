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

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "Common.h"


// http://robotika.cz/robots/katarina/en#150202
// https://github.com/robotika/katarina
// https://github.com/Parrot-Developers/libARCommands/blob/master/Xml/ARDrone3_commands.xml


//    0          1       2      3 4 5 6       7
// frametype, frameid, seqid, payloadlen+7    payload

class Commands
{
public:
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

    enum {
        CMD_FLATTRIM = 0,       // struct.pack("BBH", 1, 0, 0)
        CMD_TAKEOFF,            // struct.pack("BBH", 1, 0, 1)
        CMD_PCMD,               // struct.pack("<BBHBbbbbf", 1, 0, 2, flag, roll, pitch, yaw, gaz, psi )
        CMD_LANDING,            // struct.pack("BBH", 1, 0, 3)
        CMD_EMERGENCY,          // struct.pack("BBH", 1, 0, 4)
        CMD_NAVIGATE_HOME,      // 
        CMD_AUTO_TAKEOFF_MODE,
        CMD_MOVEBY
    };

    enum {
        ANIM_FLIP = 0,
            
    };

    enum {
        CAM_ORIENTATION = 0,
    };

    enum {
        MEDIA_PICTURE = 0,
        MEDIA_VIDEO,
        MEDIA_VIDEO2,
        
    };

    enum {
        PIC_VIDEO_AUTORECORD = 5,
            
    };

    enum {
        COM_ALLSTATES = 0,
        COM_DATE = 1,
        COM_TIME = 2,
    };

    enum {
        SPEED_MAXVERTICAL = 0,
    };

    enum {
        STREAM_VIDEO = 0,
    };

    enum {
        SETTING_ALL = 0,
    };

    enum {
        GPS_SETHIME   = 0,
        GPS_RESETHOME = 1,
    };

    Commands();
    ~Commands();


    void takeOff(void)      {   buildCmd(mBuf, ARDRONE3, CLASS_PILOTING, CMD_TAKEOFF);    }
    void land(void)         {   buildCmd(mBuf, ARDRONE3, CLASS_PILOTING, CMD_LANDING);    }
    void emergency(void)    {   buildCmd(mBuf, ARDRONE3, CLASS_PILOTING, CMD_EMERGENCY);  }
    void trim(void)         {   buildCmd(mBuf, ARDRONE3, CLASS_PILOTING, CMD_FLATTRIM);   }
    void move(u8 enRollPitch, s8 roll, s8 pitch, s8 yaw, s8 gaz);
    void enableVideoAutoRecording(u8 enable, u8 storage = 0);
    void takePicture(u8 storage = 0);
    void recordVideo(u8 enable, u8 storage = 0);
//    void setDate(

// datetime.datetime.now().date().isoformat()                ==> '2016-04-15'              V
// datetime.datetime.now().time().isoformat()                ==> '18:55:34.756000'
// datetime.datetime.now().time().strftime("T%H%M%S+0000")   ==> 'T185603+0000'            V

private:
    int buildCmd(u8 *buf, u8 prj, u8 cls, u16 cmd, char *fmt, ...);
    int buildCmd(u8 *buf, u8 prj, u8 cls, u16 cmd);
    
        
    u8  mSeqID[2];
    u8  mBuf[100];
};

#endif
