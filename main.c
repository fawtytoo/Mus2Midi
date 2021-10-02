// mus2midi

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mus2midi.h"

typedef struct stat     STATUS;

int main(int argc, char **argv)
{
    STATUS      status;
    FILE        *file;
    BYTE        *musData, *midData;
    int         size;
    char        *filename;

    if (argc != 2)
    {
        printf("Usage: %s MUSFILE\nThe MIDI filename will use the MUS filename with .mid extension appended\n", argv[0]);
        return 1;
    }

    if (stat(argv[1], &status) < 0)
    {
        printf("Cannot read %s\n", argv[1]);
        return 1;
    }

    if ((file = fopen(argv[1], "rb")) == NULL)
    {
        printf("Cannot open %s\n", argv[1]);
        return 1;
    }

    filename = malloc(strlen(argv[1]) + 5);
    sprintf(filename, "%s.mid", argv[1]);

    musData = malloc(status.st_size);
    fread(musData, status.st_size, 1, file);
    fclose(file);

    size = status.st_size;

    midData = mus2midi(musData, &size);

    free(musData);

    if (midData == NULL)
        printf("MUS file is corrupted or invalid\n");
    else
    {
        printf("Completed successfully\n");
        file = fopen(filename, "wb");
        fwrite(midData, size, 1, file);
        fclose(file);
        printf("MIDI file written to %s\n", filename);
    }

    free(filename);
    free(midData);

    return 0;
}

// mus2midi
