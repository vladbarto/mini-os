#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <stdlib.h>
#include "main.h"
#include "string.h"
#include "logging.h"
#include "scancode.h"

#define MAX_LINES       25
#define MAX_COLUMNS     80
#define MAX_OFFSET      2000 //25 lines * 80 chars
#define LOG_BUF_MAX_SIZE 512

// ------------ DEFINE COLOURS ------------
#define BLACK_8BIT 0x00
#define WHITE_8BIT 0x0F
#define GREEN_BRIGHT_8BIT 0x0A
#define PINK_MAGENTA_8BIT 0x0D
#define CYAN_8BIT 0x0B
// ----------------------------------------

#pragma pack(push)
#pragma pack(1)
typedef struct _SCREEN
{
    char c;
    BYTE color;
}SCREEN, *PSCREEN;
#pragma pack(pop)

extern BYTE BufferCLI[MAX_COLUMNS];

void HelloBoot();

void SetColor(BYTE Color);
void ScreenDisplay(const char* str, BYTE color);
void ClearScreen();
void ClearCharacter();
void EnterNewLine();
void MoveTextCursor(WORD Key);
void PutChar(char C, int Pos);
void PutString(char* String, int Pos);
void PutStringLine(char* String, int Line);
void LogSerialAndScreen(char* FormatBuffer, ...);
void FormattedLog(char* FormatBuffer, ...);
void CLI_init();
#endif // _SCREEN_H_