#include <type.h>

#define VIDEO_MEM 0xB8000

void die()
{
    char * video_mem = (char *) VIDEO_MEM;

    video_mem[0] = 'D';
    video_mem[1] = 15;

    video_mem[2] = 'I';
    video_mem[3] = 15;

    video_mem[4] = 'E';
    video_mem[5] = 15;
    while(1);
}

void setup_main()
{
    int i = 0;
    die();
    while (1);
}