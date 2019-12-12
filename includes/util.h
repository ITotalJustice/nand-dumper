#ifndef _UTIL_H_
#define _UTIL_H_


typedef struct
{
    uint64_t down;
    uint64_t held;
} poll_input_t;


//
uint32_t move_cursor_up(uint32_t cursor, uint32_t cursor_max);

//
uint32_t move_cursor_down(uint32_t cursor, uint32_t cursor_max);

//
void print_message_display(const char* message, ...);

//
void poll_input(poll_input_t *k);

//
int64_t ncm_get_storage_free_space_sd_card(void);

#endif