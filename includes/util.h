#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>
#include <stdbool.h>
#include <switch.h>


typedef struct
{
    uint64_t down;
    uint64_t held;
} poll_input_t;


//
uint32_t move_cursor_up(uint32_t cursor, uint32_t cursor_max);
uint32_t move_cursor_down(uint32_t cursor, uint32_t cursor_max);

//
bool create_dir(const char *dir, ...);
bool change_dir(const char *path, ...);

//
void print_message_display(const char* message, ...);
void print_message_loop_lock(const char *message, ...);

//
void poll_input(poll_input_t *k);

//
bool get_sys_version(SetSysFirmwareVersion *ver);
bool get_date(TimeCalendarTime *out);

//
bool set_up_output_dir(void);
const char *get_output_dir(void);

//
int64_t ncm_get_storage_free_space_sd_card(void);

#endif