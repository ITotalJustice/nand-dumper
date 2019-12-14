#include <stdio.h>
#include <switch.h>

#include "nand.h"
#include "util.h"


#define APP_PATH    "/switch/nand-dumper"
#define APP_VERSION "0.0.1"

// global because i intend to have the menu in a seperate c file soon (tm).
#define CURSOR_MAX  14
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


const char *partition_names[] =
{
    "BOOT0",
    "BOOT1",
    "NAND",
    "BCPKG2_1",
    "BCPKG2_2",
    "BCPKG2_3",
    "BCPKG2_4",
    "BCPKG2_5",
    "BCPKG2_6",
    "prodinfo",
    "prodinfof",
    "safe",
    "system",
    "user",
};

uint8_t partition_id[] =
{
    FsBisPartitionId_BootPartition1Root,
    FsBisPartitionId_BootPartition2Root,
    FsBisPartitionId_UserDataRoot,
    FsBisPartitionId_BootConfigAndPackage2Part1,
    FsBisPartitionId_BootConfigAndPackage2Part2,
    FsBisPartitionId_BootConfigAndPackage2Part3,
    FsBisPartitionId_BootConfigAndPackage2Part4,
    FsBisPartitionId_BootConfigAndPackage2Part5,
    FsBisPartitionId_BootConfigAndPackage2Part6,
    FsBisPartitionId_CalibrationBinary,
    FsBisPartitionId_CalibrationFile,
    FsBisPartitionId_SafeMode,
    FsBisPartitionId_User,
    FsBisPartitionId_System
};


void menu_print()
{
    consoleClear();
    printf("Welcome to nand dumper %s", APP_VERSION);
    printf("\t\t\tTotal free sd card space = %ldGB\n\n\n\n\n", g_sd_free_space / 0x100000);

    for (uint8_t i = 0; i < CURSOR_MAX; i++)
    {
        if (g_cursor == i)
            printf("> dump %s\n\n", partition_names[i]);
        else
            printf("dump %s\n\n", partition_names[i]);
    }

    consoleUpdate(NULL);
}

int main(int argc, char *argv[])
{
    app_init();
    create_dir(APP_PATH);
    change_dir(APP_PATH);
    set_up_output_dir();

    menu_print();

    while (appletMainLoop())
    {
        poll_input_t k;
        poll_input(&k);

        if (k.down & KEY_DOWN)
        {
            g_cursor = move_cursor_down(g_cursor, CURSOR_MAX);
            menu_print();
        }

        if (k.down & KEY_UP)
        {
            g_cursor = move_cursor_up(g_cursor, CURSOR_MAX);
            menu_print();
        }

        if (k.down & KEY_A)
        {
            nand_dump_start(partition_names[g_cursor], partition_id[g_cursor], g_sd_free_space);  
        }

        if (k.down & KEY_B || k.down & KEY_PLUS)
            break;
    }
    
    app_exit();
    return 0;
}