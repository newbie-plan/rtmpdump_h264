#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "rtmp_send_h264.h"




int main(int argc, const char *argv[])
{
    RtmpInfo_t rtmp;
    FILE *fp = NULL;
    long int file_size = 0;
    unsigned int time_stamp = 0;
    int tiker = 1000 / 50;          /*fps = 25*/

    if (argc < 3)
    {
        printf("Usage:%s <h264 file> <rtmp url>.\n", argv[0]);
        return -1;
    }

    fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        printf("fopen [%s] filed.\n", argv[1]);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    memset(&rtmp, 0, sizeof(RtmpInfo_t));
    rtmp.h264_data = (char *)malloc(file_size);
    if (rtmp.h264_data == NULL)
    {
        printf("malloc for file data failed.\n");
        return -1;
    }
    rtmp.size = fread(rtmp.h264_data, 1, file_size, fp);


    rtmp_h264_connect(&rtmp, argv[2]);


    while (rtmp.nal_head_pos < rtmp.size)
    {
        H264Packet_t nal;
        NalType_e ret = H264_UNKNOW;
        memset(&nal, 0, sizeof(H264Packet_t));

get_one_nal:
        ret = rtmp_h264_get_nal(&rtmp, &nal);
        if (ret != H264_I_FRAME && ret != H264_P_FRAME)
        {
            goto get_one_nal;
        }
        else
        {
            if (nal.is_iframe == 1)
            {
                printf("->> I frame : ");
                if (nal.pps != NULL && nal.sps != NULL)
                {
                    printf("[%02x] = %d : [%02x] = %d : ", nal.sps[0] & 0x1F, nal.sps_size, nal.pps[0] & 0x1F, nal.pps_size);
                }
                printf("[%02x] = %d\n", nal.frame[0] & 0x1F, nal.frame_size);
            }
            else
            {
                printf("->> P frame : [%02x] = %d\n", nal.frame[0] & 0x1F, nal.frame_size);
            }
            printf("  nal_head_pos = [%d]\n", rtmp.nal_head_pos);


            rtmp_h264_send(&rtmp, &nal, time_stamp);
            time_stamp += tiker;
            usleep(tiker*1000);
        }
    }

    rtmp_h264_close(&rtmp);
    if (rtmp.h264_data != NULL)
    {
        free(rtmp.h264_data);
    }
    fclose(fp);
    return 0;
}
