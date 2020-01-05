#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <switch.h>
#include <threads.h>

#include "util.h"
#include "usb.h"


#define CHUNK_SIZE  0xFFFF0000
#define BUF_SIZE    0x800000    // 8MiB


typedef struct
{
    char *out_dir;

    void *data;
    uint64_t data_size;
    uint64_t data_stored;
    uint64_t data_written;
    uint64_t read_offset;
    uint64_t total_size;
} thrd_struct_t;

// globals
mtx_t g_mtx;
cnd_t can_write;
cnd_t can_read;

int64_t nand_size = 0;
FsStorage storage;


int nand_read(void *in)
{
    thrd_struct_t *t = (thrd_struct_t *)in;

    void *buf = memalign(0x1000, BUF_SIZE);
    if (buf == NULL) return -1;

    for (uint64_t part = 0; t->read_offset < nand_size; part++)
    {
        char output_name[0x20];
        snprintf(output_name, 0x20, part < 10 ? "%s/0%lu" : "%s/%lu", t->out_dir, part);

        for (uint64_t chunk_done = 0, buf_size = BUF_SIZE; chunk_done < CHUNK_SIZE && t->read_offset < nand_size; chunk_done += BUF_SIZE, t->read_offset += BUF_SIZE)
        {
            if (t->read_offset + buf_size > nand_size)
                buf_size = nand_size - t->read_offset;

            fsStorageRead(&storage, t->read_offset, buf, buf_size);

            mtx_lock(&g_mtx);
            if (t->data_size > 0)
            {
                cnd_wait(&can_read, &g_mtx);
            }

            t->data_size = buf_size;
            t->data_stored += buf_size;
            memcpy(t->data, buf, buf_size);

            mtx_unlock(&g_mtx);
            cnd_signal(&can_write);
        }
    }

    // cleanup then exit.
    free(buf);
    return 0;
}

int nand_write(void *in)
{
    thrd_struct_t *t = (thrd_struct_t *)in;

    while (t->data_written < t->total_size)
    {
        mtx_lock(&g_mtx);
        if (t->data_size == 0)
        {
            cnd_wait(&can_write, &g_mtx);
        }

        usb_write(t->data, t->data_size);
        t->data_written += t->data_size;
        t->data_size = 0;
        
        mtx_unlock(&g_mtx);
        cnd_signal(&can_read);
    }

    return 0;
}

bool nand_mount(int64_t free_space)
{
    Result rc = 0;

    rc = fsOpenBisStorage(&storage, FsBisPartitionId_UserDataRoot);
    if (R_FAILED(rc))
        return false;

    fsStorageGetSize(&storage, &nand_size);
    if (R_SUCCEEDED(rc) && nand_size < free_space + 0x2000000) // add 32mb to free space just in case the sd card has issues writing to 0 free space.
        return true;

    fsStorageClose(&storage);
    return false;
}

bool nand_dump_start(int64_t free_space)
{
    if (!nand_mount(free_space))
        return false;
    
    TimeCalendarTime cal;
    get_date(&cal);
    char dir_buf[0x20];
    sprintf(dir_buf, "nand_dump_%u-%u-%u-%u-%u", cal.year, cal.month, cal.day, cal.hour, cal.minute);
    create_dir(dir_buf);

    usb_poll(1, 0, nand_size);
    thrd_struct_t t = { dir_buf, memalign(0x1000, BUF_SIZE), 0, 0, 0, 0, nand_size };

    mtx_init(&g_mtx, mtx_plain);
    cnd_init(&can_read);
    cnd_init(&can_write);

    thrd_t t_read;
    thrd_t t_write;

    thrd_create(&t_read, nand_read, &t);
    thrd_create(&t_write, nand_write, &t);

    print_message_display("starting dump process...\n\n\n");
    while (t.data_written != t.total_size)
    {
        print_message_display("dumping... %luMiB %luMiB   %ldMiB\r", t.data_written / 0x100000, t.read_offset / 0x100000, t.total_size / 0x100000);
    }

    thrd_join(t_read, NULL);
    thrd_join(t_write, NULL);
    
    mtx_destroy(&g_mtx);
    cnd_destroy(&can_read);
    cnd_destroy(&can_write);

    free(t.data);
    fsStorageClose(&storage);
    usb_exit();
    return true;
}