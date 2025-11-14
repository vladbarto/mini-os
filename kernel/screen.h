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
extern BYTE MainScreenVideoMemoryBuffer[MAX_LINES * MAX_COLUMNS * 2];
extern BYTE EditScreenVideoMemoryBuffer[MAX_LINES * MAX_COLUMNS * 2];
extern QWORD MainScreenCursorPosition;
extern QWORD EditScreenCursorPosition;
extern BOOL EditScreenInitialized;
extern BOOL CapsLockOn;

void HelloBoot();

void SetColor(BYTE Color);
void ScreenDisplay(const char* str, BYTE color);
void ClearScreen();
void ClearCharacter();
void EnterNewLine(BYTE prompt);
void MoveTextCursor(WORD Key);
void PutChar(char C, int Pos);
void PutString(char* String, int Pos);
void PutStringLine(char* String, int Line);
void LogSerialAndScreen(char* FormatBuffer, ...);
void FormattedLog(char* FormatBuffer, ...);
void CLI_init();
/**
 * Saves the screen of main mode storing directly video memory
 * @Param MainScreenVideoMemoryBuffer - if NULL don't store the previous content
 * @Param BufferSize - 20x80
 * @Param CursorPosition - Last known cursor position when state was saved
 */
void 
SaveScreen(
    void*     VideoMemoryBuffer,
    DWORD     BufferSize,
    QWORD*    CursorPosition
);

/**
 * Restores the screen of main mode writing directly the previously saved video memory
 * @Param VideoMemoryBuffer - if NULL empty screen to be restored
 * @Param BufferSize - 20x80
 * @Param cursorPosition - restore last known cursor position when state was saved
 */
void RestoreScreen(
    void*   VideoMemoryBuffer,
    DWORD   BufferSize,
    QWORD*  cursorPosition
);
#endif // _SCREEN_H_