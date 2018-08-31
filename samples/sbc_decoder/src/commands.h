#ifndef __COMMANDS_H_
#define __COMMANDS_H_

#define CMD_AREA             0xF8
#define DATA_AREA            0x07

#define CMD_SEND_CHUNK       0x08
#define CMD_RECEIVE_CHUNK    0x10
#define CMD_READY            0x18
#define CMD_BREAK_OFF        0xF8

#define CLEAR_DATA_AREA(pkg)    ((pkg) & CMD_AREA)
#define CLEAR_CMD_AREA(pkg)     ((pkg) & DATA_AREA)
#define DETECT_CMD(pkg, cmd)    !(CLEAR_DATA_AREA(pkg) ^ cmd)

#endif