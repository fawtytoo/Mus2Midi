// mus2midi

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "mus2midi.h"

typedef unsigned short  WORD;

typedef struct
{
    char        id[4];
    WORD        scoreLen;
    WORD        scoreStart;
} __attribute__ ((packed)) HDR_MUS;

typedef struct
{
    char        id[4];
    int         length;
    WORD        type;
    WORD        ntracks;
    WORD        ticks;
} __attribute__ ((packed)) HDR_MID;

char        magicMus[4] = {'M', 'U', 'S', 0x1a};
char        magicMid[4] = {'M', 'T', 'h', 'd'};
char        magicTrk[4] = {'M', 'T', 'r', 'k'};

int         controllerMap[16] = {-1, 0, 1, 7, 10, 11, 91, 93, 64, 67, 120, 123, 126, 127, 121, -1};

BYTE        *midData;
int         midSize;

BYTE        *musPos;
int         musEOT;

BYTE        deltaBytes[4];
int         deltaCount;

// maintain a list of channel volume
BYTE        musChannel[16];

void convert()
{
    BYTE        data, last, channel;
    BYTE        event[3];
    int         count;

    data = *musPos++;
    last = data & 0x80;
    channel = data & 0xf;

    switch (data & 0x70)
    {
      case 0x00:
        event[0] = 0x80;
        event[1] = *musPos++ & 0x7f;
        event[2] = musChannel[channel];
        count = 3;
        break;

      case 0x10:
        event[0] = 0x90;
        data = *musPos++;
        event[1] = data & 0x7f;
        event[2] = data & 0x80 ? *musPos++ : musChannel[channel];
        musChannel[channel] = event[2];
        count = 3;
        break;

      case 0x20:
        event[0] = 0xe0;
        event[1] = (*musPos & 0x01) << 6;
        event[2] = *musPos++ >> 1;
        count = 3;
        break;

      case 0x30:
        event[0] = 0xb0;
        event[1] = controllerMap[*musPos++ & 0xf];
        event[2] = 0x7f;
        count = 3;
        break;

      case 0x40:
        data = *musPos++;
        if (data == 0)
        {
            event[0] = 0xc0;
            event[1] = *musPos++;
            count = 2;
            break;
        }
        event[0] = 0xb0;
        event[1] = controllerMap[data & 0xf];
        event[2] = *musPos++;
        count = 3;
        break;

      case 0x50:
        return;

      case 0x60:
        event[0] = 0xff;
        event[1] = 0x2f;
        event[2] = 0x00;
        count = 3;

        // this prevents deltaBytes being read past the end of the MUS data
        last = 0;

        musEOT = 1;
        break;

      case 0x70:
        musPos++;
        return;
    }

    if (channel == 9)
        channel = 15;
    else if (channel == 15)
        channel = 9;

    event[0] |= channel;

    midData = realloc(midData, midSize + deltaCount + count);

    memcpy(midData + midSize, &deltaBytes, deltaCount);
    midSize += deltaCount;
    memcpy(midData + midSize, &event, count);
    midSize += count;

    if (last)
    {
        deltaCount = 0;
        do
        {
            data = *musPos++;
            deltaBytes[deltaCount] = data;
            deltaCount++;
        } while (data & 128);
    }
    else
    {
        deltaBytes[0] = 0;
        deltaCount = 1;
    }
}

BYTE *mus2midi(BYTE *data, int *length)
{
    HDR_MUS     *hdrMus = (HDR_MUS *)data;
    HDR_MID     hdrMid;
    int         midTrkLenOffset;
    int         trackLen;
    int         i;

    if (strncmp(hdrMus->id, magicMus, 4) != 0)
        return NULL;

    if (*length != hdrMus->scoreStart + hdrMus->scoreLen)
        return NULL;

    midSize = sizeof(HDR_MID);
    memcpy(hdrMid.id, magicMid, 4);
    hdrMid.length = __builtin_bswap32(6);
    hdrMid.type = __builtin_bswap16(0); // single track should be type 0
    hdrMid.ntracks = __builtin_bswap16(1);
    // maybe, set 140ppqn and set tempo to 1000000µs
    hdrMid.ticks = __builtin_bswap16(70); // 70 ppqn = 140 per second @ tempo = 500000µs (default)
    midData = malloc(midSize);
    memcpy(midData, &hdrMid, midSize);

    midData = realloc(midData, midSize + 8);
    memcpy(midData + midSize, magicTrk, 4);
    midSize += 4;
    midTrkLenOffset = midSize;
    midSize += 4;

    trackLen = 0;

    musPos = data + hdrMus->scoreStart;
    musEOT = 0;
    deltaBytes[0] = 0;
    deltaCount = 1;

    for (i = 0; i < 16; i++)
        musChannel[i] = 0;

    while (!musEOT)
        convert();

    trackLen = __builtin_bswap32(midSize - sizeof(HDR_MID) - 8);
    memcpy(midData + midTrkLenOffset, &trackLen, 4);

    *length = midSize;

    return midData;
}

// mus2midi
