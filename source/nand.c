#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <threads.h>

#include "util.h"


#define OUTPUT_NAME "nand.bin"
#define CHUNK_SIZE  0xFFFF0000
#define BUF_SIZE    0x800000    // 8MiB


typedef struct
{
    FILE *f;
    void *data;
    uint64_t data_size;
    uint64_t data_written;
    uint64_t read_offset;
    uint64_t total_size;
} thrd_struct_t;

// globals
static mtx_t g_mtx;
int64_t nand_size = 0;
FsStorage storage;


int nand_read(void *in)
{
    thrd_struct_t *t = (thrd_struct_t *)in;

    void *buf = malloc(BUF_SIZE);
    if (buf == NULL)
        return 1;

    for (uint64_t part = 0; t->read_offset < nand_size; part++)
    {
        char output_name[0x20];
        if (part < 10)
            snprintf(output_name, 0x20, "%s.0%lu", OUTPUT_NAME, part);
        else
            snprintf(output_name, 0x20, "%s.%lu", OUTPUT_NAME, part);

        mtx_lock(&g_mtx);
        {
            t->f = fopen(output_name, "wb");
            if (!t->f)
                return 1;
        }
        mtx_unlock(&g_mtx);

        for (uint64_t chunk_done = 0, buf_size = BUF_SIZE; chunk_done < CHUNK_SIZE && t->read_offset < nand_size; chunk_done += BUF_SIZE, t->read_offset += BUF_SIZE)
        {
            if (t->read_offset + buf_size > nand_size)
                buf_size = nand_size - t->read_offset;

            fsStorageRead(&storage, t->read_offset, buf, buf_size);

            mtx_lock(&g_mtx);
            {
                t->data_size = buf_size;
                memcpy(t->data, buf, buf_size);
            }
            mtx_unlock(&g_mtx);
        }

        mtx_lock(&g_mtx);
        {
            fclose(t->f);
        }
        mtx_unlock(&g_mtx);
    }

    // cleanup then exit.
    free(buf);
    fsStorageClose(&storage);
    return 0;
}

int nand_write(void *in)
{
    thrd_struct_t *t = (thrd_struct_t *)in;

    while (t->data_written != t->total_size)
    {
        mtx_lock(&g_mtx);
        {
            fwrite(t->data, t->data_size, 1, t->f);
            t->data_written += t->data_size;
            t->data_size = 0;
        }
        mtx_unlock(&g_mtx);
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
    
    thrd_struct_t t = { NULL, malloc(BUF_SIZE), 0, 0, 0, nand_size };

    mtx_init(&g_mtx, mtx_plain);

    thrd_t t_read;
    thrd_t t_write;

    thrd_create(&t_read, nand_read, &t);
    thrd_create(&t_write, nand_write, &t);

    print_message_display("starting dump process...\n\n\n");

    while (t.data_written != t.total_size)
    {
        print_message_display("dumping... %luMiB   %ldMiB\r", t.read_offset / 0x100000, t.total_size / 0x100000);
    }

    mtx_destroy(&g_mtx);
    free(t.data);
    return true;
}