#include "screen.h"

static PSCREEN gVideo = (PSCREEN)(0x000B8000);

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
		gVideo[i].color = 10;
		gVideo[i].c = boot[i];
	}
    CursorPosition(i);
}

void ClearScreen()
{
    int i;

    for (i = 0; i < MAX_OFFSET; i++)
    {
        gVideo[i].color = 10;
        gVideo[i].c = ' ';
    }
    CursorMove(0, 0);
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

void LogSerialAndScreen(char* FormatBuffer, ...)
{
    char logBuffer[LOG_BUF_MAX_SIZE];
    va_list va;

    va_start(va, FormatBuffer);
    cl_vsnprintf(logBuffer, LOG_BUF_MAX_SIZE, FormatBuffer, va);

    // after call logBuffer will contain formatted buffer

    Log(logBuffer); // log through serial
    ScreenDisplay(logBuffer, 0x03); // display on screen - you will need to implement this part in `screen.c`
}