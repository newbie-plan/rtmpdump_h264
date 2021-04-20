#ifndef __RTMP_SEND_H264_H__
#define __RTMP_SEND_H264_H__

#include <librtmp/rtmp.h>

typedef struct
{
    RTMP* rtmp_h264;
    char *h264_data;
    int size;
    int nal_head_pos;
    int width;
    int height;
}RtmpInfo_t;

typedef struct
{
    int is_iframe;
    char *sps;
    int sps_size;
    char *pps;
    int pps_size;
    char *frame;
    int frame_size;
}H264Packet_t;

typedef enum
{
    H264_UNKNOW,
    H264_SPS,
    H264_PPS,
    H264_I_FRAME,
    H264_P_FRAME,
}NalType_e;

int rtmp_h264_connect(RtmpInfo_t *rtmp, const char *url);
int rtmp_h264_send(RtmpInfo_t *rtmp, H264Packet_t *nal, unsigned int time_stamp);
int rtmp_h264_close(RtmpInfo_t *rtmp);
NalType_e rtmp_h264_get_nal(RtmpInfo_t *rtmp, H264Packet_t *nal);


#endif


