#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <switch.h>


#define OUTPUT_NAME "nand.bin"
#define CHUNK_SIZE  0xFFFF0000
#define BUF_SIZE    0x800000    // 8MiB


void app_init()
{
    consoleInit(NULL);
}

void app_exit()
{
    consoleExit(NULL);
}

int main(int argc, char *argv[])
{
    app_init();

    printf("welcome to nand dumper...\n\n\n");
    consoleUpdate(NULL);

    int64_t nand_size = 0;
    FsStorage storage;
    fsOpenBisStorage(&storage, FsBisPartitionId_UserDataRoot);
    fsStorageGetSize(&storage, &nand_size);

    void *buf = malloc(BUF_SIZE);
    if (buf == NULL)
        return 1;

    printf("starting dump process...\n\n\n");
    consoleUpdate(NULL);

    for (uint64_t offset = 0, part = 0; offset < nand_size; part++)
    {
        char output_name[0x20];
        if (part < 10)
            snprintf(output_name, 0x20, "%s.0%lu", OUTPUT_NAME, part);
        else
            snprintf(output_name, 0x20, "%s.%lu", OUTPUT_NAME, part);
        FILE *f = fopen(output_name, "wb");

        for (uint64_t chunk_done = 0, buf_size = BUF_SIZE; chunk_done < CHUNK_SIZE; chunk_done += BUF_SIZE, offset += BUF_SIZE)
        {
            if (offset + buf_size > nand_size)
                buf_size = nand_size - offset;

            fsStorageRead(&storage, offset, buf, buf_size);
            fwrite(buf, buf_size, 1, f);    // will multi thread after.
            printf("dumping... %luMiB   %ldMiB\r", offset / 0x100000, nand_size / 0x100000);
            consoleUpdate(NULL);
        }
        fclose(f);
    }

    // cleanup then exit.
    free(buf);
    fsStorageClose(&storage);
    app_exit();
    return 0;
}