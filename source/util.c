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

Result ncm_open_storage(NcmContentStorage *ncm_storage, NcmStorageId storage_id)
{
    Result rc = ncmOpenContentStorage(ncm_storage, storage_id);
    if (R_FAILED(rc))
        printf("failed to open content storage\n");
    return rc;
}

void ncm_close_storage(NcmContentStorage *ncm_storage)
{
    ncmContentStorageClose(ncm_storage);
    serviceClose(&ncm_storage->s);
}

int64_t ncm_get_storage_free_space(NcmStorageId storage_id)
{
    int64_t size = 0;
    NcmContentStorage ncm_storage;
    if (R_FAILED(ncm_open_storage(&ncm_storage, storage_id)))
        return size;
    if (R_FAILED(ncmContentStorageGetFreeSpaceSize(&ncm_storage, &size)))
        printf("failed to get free storage space\n");
    ncm_close_storage(&ncm_storage);
    return size;
}

int64_t ncm_get_storage_free_space_sd_card(void)
{
    return ncm_get_storage_free_space(NcmStorageId_SdCard);
}