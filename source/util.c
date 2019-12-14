#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <malloc.h>
#include <dirent.h>
#include <unistd.h>
#include <switch.h>

#include "util.h"


char output_dir[0x30];

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

bool check_if_dir_exists(const char *directory)
{
    DIR *dir = opendir(directory);
    if (!dir)
        return false;
    closedir(dir);
    return true;
}

bool create_dir(const char *dir, ...)
{
    char new_dir[FS_MAX_PATH];
    va_list arg;
    va_start(arg, dir);
    vsprintf(new_dir, dir, arg);
    va_end(arg);

    if (check_if_dir_exists(new_dir))
        return true;
    int res = mkdir(new_dir, 0777);
    if (res == 0)
        return true;
    return false;
}

bool change_dir(const char *path, ...)
{
    char new_path[FS_MAX_PATH];
    va_list arg;
    va_start(arg, path);
    vsprintf(new_path, path, arg);
    va_end(arg);

    if (!check_if_dir_exists(new_path))
        create_dir(new_path);
    int res = chdir(new_path);
    if (res == 0)
        return true;
    return false;
}

void print_message_display(const char *message, ...)
{
    char new_message[FS_MAX_PATH];
    va_list arg;
    va_start(arg, message);
    vsprintf(new_message, message, arg);
    va_end(arg);

    printf("%s", new_message);
    consoleUpdate(NULL);
}

void print_message_loop_lock(const char *message, ...)
{
    char new_message[FS_MAX_PATH];
    va_list arg;
    va_start(arg, message);
    vsprintf(new_message, message, arg);
    va_end(arg);

    printf("%s", new_message);
    consoleUpdate(NULL);

    while (appletMainLoop())
    {
        poll_input_t k;
        poll_input(&k);
        if (k.down & KEY_B)
            break;
    }
}

void poll_input(poll_input_t *k)
{
    hidScanInput();
    k->down = hidKeysDown(CONTROLLER_P1_AUTO);
    k->held = hidKeyboardHeld(CONTROLLER_P1_AUTO);
}

bool get_sys_version(SetSysFirmwareVersion *ver)
{
	if (R_SUCCEEDED(setsysGetFirmwareVersion(ver)))
		return true;
    print_message_loop_lock("failed to get sys fw version.\n\n Press B to exit\n\n");
    return false;
}

bool get_date(TimeCalendarTime *out)
{
    uint64_t time_stamp;
    TimeLocationName name;
    TimeZoneRule rule;
    TimeCalendarAdditionalInfo info;

    if (R_FAILED(timeGetCurrentTime(TimeType_Default, &time_stamp)))
    {
        print_message_loop_lock("failed to get sys fw version.\n\n Press B to exit\n\n");
        return false;
    }
    if (R_FAILED(timeGetDeviceLocationName(&name)))
    {
        print_message_loop_lock("failed to get sys fw version.\n\n Press B to exit\n\n");
        return false;
    }
    if (R_FAILED(timeLoadTimeZoneRule(&name, &rule)))
    {
        print_message_loop_lock("failed to get sys fw version.\n\n Press B to exit\n\n");
        return false;
    }
    if (R_FAILED(timeToCalendarTime(&rule, time_stamp, out, &info)))
    {
        print_message_loop_lock("failed to get sys fw version.\n\n Press B to exit\n\n");
        return false;
    }
    return true;
}

bool set_up_output_dir(void)
{
    TimeCalendarTime cal;
    SetSysFirmwareVersion ver;

    if (!get_date(&cal) || !get_sys_version(&ver))
        return false;

    char year[0x6];
    char month[0x4];
    char day[0x4];

    sprintf(year, "%u", cal.year);
    sprintf(month, "%u", cal.month);
    sprintf(day, "%u", cal.day);
    sprintf(output_dir, "%s/%s/%s/%s", ver.display_version, year, month, day);
    
    if (!create_dir(ver.display_version))
        return false;
    if (!create_dir("%s/%s", ver.display_version, year))
        return false;
    if (!create_dir("%s/%s/%s", ver.display_version, year, month))
        return false;
    if (!create_dir(output_dir))
        return false;

    return true;
}

const char *get_output_dir(void)
{
    return output_dir;
}

Result ncm_open_storage(NcmContentStorage *ncm_storage, NcmStorageId storage_id)
{
    Result rc = ncmOpenContentStorage(ncm_storage, storage_id);
    if (R_FAILED(rc))
        print_message_loop_lock("failed to open content storage\n");
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
        print_message_loop_lock("failed to get free storage space\n");
    ncm_close_storage(&ncm_storage);
    return size;
}

int64_t ncm_get_storage_free_space_sd_card(void)
{
    return ncm_get_storage_free_space(NcmStorageId_SdCard);
}