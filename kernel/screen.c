#include "screen.h"

static PSCREEN gVideo = (PSCREEN)(0x000B8000);

WORD GetCursorPosition(void)
{
    WORD pos = 0;
    __outbyte(0x3D4, 0x0F);
    pos |= __inbyte(0x3D5);
    __outbyte(0x3D4, 0x0E);
    pos |= ((WORD)__inbyte(0x3D5)) << 8;
    return pos;
}

void CursorMove(int row, int col)
{
    unsigned short location = (row * MAX_COLUMNS) + col;       /* Short is a 16bit type , the formula is used here*/

    //Cursor Low port
    __outbyte(0x3D4, 0x0F);                                    //Sending the cursor low byte to the VGA Controller
    __outbyte(0x3D5, (unsigned char)(location & 0xFF));

    //Cursor High port
    __outbyte(0x3D4, 0x0E);                                    //Sending the cursor high byte to the VGA Controller
    __outbyte(0x3D5, (unsigned char)((location >> 8) & 0xFF)); //Char is a 8bit type
}

void CursorPosition(int pos)
{
    int row, col;

    if (pos > MAX_OFFSET)
    {
        pos = pos % MAX_OFFSET;
    }

    row = pos / MAX_COLUMNS;
    col = pos % MAX_COLUMNS;

    CursorMove(row, col);
}

void HelloBoot()
{
    int i, len;
	char boot[] = "Hello Boot! Greetings from C...";

	len = 0;
	while (boot[len] != 0)
	{
		len++;
	}

	for (i = 0; (i < len) && (i < MAX_OFFSET); i++)
	{
		gVideo[i].color = GREEN_BRIGHT_8BIT;
		gVideo[i].c = boot[i];
	}

    CursorMove(1, 0);
}

void ClearScreen()
{
    int i;

    for (i = 0; i < MAX_OFFSET; i++)
    {
        gVideo[i].color = GREEN_BRIGHT_8BIT;
        gVideo[i].c = ' ';
    }
    CursorMove(0, 0);
}

void ClearCharacter() {
    WORD cursor = GetCursorPosition();
    if(cursor%MAX_COLUMNS != 0) cursor--;
    gVideo[cursor].color = BLACK_8BIT;
    gVideo[cursor].c = ' ';
    CursorPosition(cursor);
}

/**
 * Called if 'Enter' Keyboard Interrupt occurs
 */
void EnterNewLine() {
    WORD cursor = GetCursorPosition();
    if(cursor < MAX_LINES * MAX_COLUMNS - MAX_COLUMNS)
    {
        CursorPosition(cursor + MAX_COLUMNS - cursor%80);
        ScreenDisplay("$ ", CYAN_8BIT);
    }
    else ClearScreen();
}

/**
 * Based on provided arrow (up, down, left or right) move the cursor accordingly.
 */
void MoveTextCursor(WORD Key) {
    WORD CurrentCursorPosition = GetCursorPosition();
    if (KEY_UP == Key) {
        INT16 Offset = CurrentCursorPosition - MAX_COLUMNS;
        if ( Offset >= 0) {
            CursorPosition(CurrentCursorPosition - MAX_COLUMNS);
        }
    } else if (KEY_DOWN == Key) {
        INT16 Offset = CurrentCursorPosition + MAX_COLUMNS;
        if ( Offset <= MAX_LINES * MAX_COLUMNS) {
            CursorPosition(CurrentCursorPosition + MAX_COLUMNS);
        }
    } else if (KEY_LEFT == Key) {
        if ( CurrentCursorPosition - 1 >= 0) {
            CursorPosition(CurrentCursorPosition - 1);
        }
    } else if (KEY_RIGHT == Key) {
        if ( CurrentCursorPosition + 1 <= MAX_LINES * MAX_COLUMNS) {
            CursorPosition(CurrentCursorPosition + 1);
        }
    } 
}

void ScreenDisplay(const char* str, BYTE color)
{
    // Get current cursor position
    unsigned short pos_low, pos_high;
    __outbyte(0x3D4, 0x0F);
    pos_low = __inbyte(0x3D5);
    __outbyte(0x3D4, 0x0E);
    pos_high = __inbyte(0x3D5);
    int cursorPos = (pos_high << 8) | pos_low;

    for (int i = 0; str[i] != '\0'; i++) {
        char c = str[i];

        if (c == '\n') {
            cursorPos += MAX_COLUMNS - (cursorPos % MAX_COLUMNS);
        } else {
            if (cursorPos >= MAX_OFFSET)
                cursorPos = 0; // wrap

            gVideo[cursorPos].c = c;
            gVideo[cursorPos].color = color;
            cursorPos++;
        }
    }

    CursorPosition(cursorPos);
}

/**
 * Format specifiers used are the ones from HAL9000:
** %c character
** %s null-terminated string
** %d signed 32-bit decimal number
** %u unsigned 32-bit decimal number
** %x unsigned 32-bit hex number

To display 64-bit values (QWORDs) use the UPPERCASE letter, i.e. %D signed 64-bit integer, %X unsigned 64-bit hex, etc.
 */
void LogSerialAndScreen(char* FormatBuffer, ...)
{
    char logBuffer[LOG_BUF_MAX_SIZE];
    va_list va;

    va_start(va, FormatBuffer);
    cl_vsnprintf(logBuffer, LOG_BUF_MAX_SIZE, FormatBuffer, va);

    // after call logBuffer will contain formatted buffer

    Log(logBuffer); // log through serial
    ScreenDisplay(logBuffer, PINK_MAGENTA_8BIT); // display on screen - you will need to implement this part in `screen.c`
}

/**
 * Format specifiers used are the ones from HAL9000:
** %c character
** %s null-terminated string
** %d signed 32-bit decimal number
** %u unsigned 32-bit decimal number
** %x unsigned 32-bit hex number

To display 64-bit values (QWORDs) use the UPPERCASE letter, i.e. %D signed 64-bit integer, %X unsigned 64-bit hex, etc.
 */
void FormattedLog(char* FormatBuffer, ...)
{
    char logBuffer[LOG_BUF_MAX_SIZE];
    va_list va;

    va_start(va, FormatBuffer);
    cl_vsnprintf(logBuffer, LOG_BUF_MAX_SIZE, FormatBuffer, va);

    // after call logBuffer will contain formatted buffer

    Log(logBuffer); // log through serial
}

BYTE BufferCLI[MAX_COLUMNS] = {0};
/** 
 Initializes command buffer and edit-mode buffer.
*/
void CLI_init() {
    for(int i = 0; i < MAX_COLUMNS; i++) BufferCLI[i] = (BYTE) 0;
}