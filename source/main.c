#include <stdio.h>
#include <switch.h>

#include "nand.h"


#define APP_VERSION "0.0.1"

typedef struct
{
    uint64_t down;
    uint64_t held;
} poll_input_t;


// global because i intend to have the menu in a seperate c file soon (tm).
static uint8_t g_cursor = 0;
static int64_t g_sd_free_space = 0;


void app_init(void)
{
    consoleInit(NULL);
}

void app_exit(void)
{
    consoleExit(NULL);
}

void menu_print()
{
    consoleClear();
    printf("Welcome to nand dumper %s...\n\n\n", APP_VERSION);
    printf("Total free sd card space = %ld\n\n\n", g_sd_free_space / 0x100000);

    const char *options[] =
    {
        "dump nand",
        "exit"
    };

    for (uint8_t i = 0; i < 2; i++)
    {
        if (g_cursor == i)
            printf("> %s\n\n", options[i]);
        else
            printf("%s\n\n", options[i]);
    }

    consoleUpdate(NULL);
}

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

void poll_input(poll_input_t *k)
{
    hidScanInput();
    k->down = hidKeysDown(CONTROLLER_P1_AUTO);
    k->held = hidKeyboardHeld(CONTROLLER_P1_AUTO);
}

int main(int argc, char *argv[])
{
    app_init();
    menu_print();

    while (appletMainLoop())
    {
        poll_input_t k;
        poll_input(&k);

        if (k.down & KEY_DOWN)
        {
            g_cursor = move_cursor_down(g_cursor, 2);
            menu_print();
        }

        if (k.down & KEY_UP)
        {
            g_cursor = move_cursor_up(g_cursor, 2);
            menu_print();
        }

        if (k.down & KEY_A)
        {
            if (g_cursor == 0)
                nand_dump_start();
            
            else
                break;
        }

        if (k.down & KEY_B || k.down & KEY_PLUS)
            break;
    }
    
    app_exit();
    return 0;
}