#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <switch.h>

#include "util.h"


uint32_t move_cursor_up(uint32_t cursor, uint32_t cursor_max)
{
    if (cursor == 0)
        cursor = cursor_max - 1;
    else
        cursor--;
    return cursor;
}

uint32_t move_cursor_down(uint32_t cursor, uint32_t cursor_max)
{
    if (cursor == cursor_max - 1)
        cursor = 0;
    else
        cursor++;
    return cursor;
}

void print_message_display(const char* message, ...)
{
    char new_message[FS_MAX_PATH];
    va_list arg;
    va_start(arg, message);
    vsprintf(new_message, message, arg);
    va_end(arg);

    printf("%s", new_message);
    consoleUpdate(NULL);
}

void poll_input(poll_input_t *k)
{
    hidScanInput();
    k->down = hidKeysDown(CONTROLLER_P1_AUTO);
    k->held = hidKeyboardHeld(CONTROLLER_P1_AUTO);
}