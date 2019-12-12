#include <stdio.h>
#include <switch.h>

#include "nand.h"
#include "util.h"


#define APP_PATH    "/switch/nand-dumper"
#define APP_VERSION "0.0.1"

// global because i intend to have the menu in a seperate c file soon (tm).
static uint8_t g_cursor = 0;
static int64_t g_sd_free_space = 0;


void app_init(void)
{
    consoleInit(NULL);
    ncmInitialize();
    timeInitialize();
    g_sd_free_space = ncm_get_storage_free_space_sd_card();
}

void app_exit(void)
{
    ncmExit();
    timeExit();
    consoleExit(NULL);
}

void menu_print()
{
    consoleClear();
    printf("Welcome to nand dumper %s\n\n\n", APP_VERSION);
    printf("Total free sd card space = %ldGB\n\n\n", g_sd_free_space / 0x100000);

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

int main(int argc, char *argv[])
{
    app_init();
    create_dir(APP_PATH);
    change_dir(APP_PATH);

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
                nand_dump_start(g_sd_free_space);
            else
                break;
        }

        if (k.down & KEY_B || k.down & KEY_PLUS)
            break;
    }
    
    app_exit();
    return 0;
}