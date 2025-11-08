#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "main.h"
#include "string.h"
#include "logging.h"

#define MAX_LINES       25
#define MAX_COLUMNS     80
#define MAX_OFFSET      2000 //25 lines * 80 chars
#define LOG_BUF_MAX_SIZE 512

#pragma pack(push)
#pragma pack(1)
typedef struct _SCREEN
{
    char c;
    BYTE color;
}SCREEN, *PSCREEN;
#pragma pack(pop)

void HelloBoot();

void SetColor(BYTE Color);
void ClearScreen();
void PutChar(char C, int Pos);
void PutString(char* String, int Pos);
void PutStringLine(char* String, int Line);
void LogSerialAndScreen(char* FormatBuffer, ...);

#endif // _SCREEN_H_