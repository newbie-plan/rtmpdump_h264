#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtmp_send_h264.h"

#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)

int rtmp_h264_connect(RtmpInfo_t *rtmp, const char *url)
{
    if (rtmp == NULL)
    {
        printf("rtmp info is NULL.\n");
        return -1;
    }

    rtmp->rtmp_h264 = RTMP_Alloc();
    if (rtmp->rtmp_h264 == NULL)
    {
        printf("RTMP_Alloc failed.\n");
        return -1;
    }

    RTMP_Init(rtmp->rtmp_h264);

    if (RTMP_SetupURL(rtmp->rtmp_h264, (char *)url) <= 0)
    {
        printf("RTMP_SetupURL failed.\n");
        RTMP_Free(rtmp->rtmp_h264);
        return -1;
    }

    RTMP_EnableWrite(rtmp->rtmp_h264);

    if (RTMP_Connect(rtmp->rtmp_h264, NULL) <= 0)
    {
        printf("RTMP_Connect failed.\n");
        RTMP_Free(rtmp->rtmp_h264);
        return -1;
    }

    if (RTMP_ConnectStream(rtmp->rtmp_h264, 0) <= 0)
    {
        printf("RTMP_ConnectStream failed.\n");
        RTMP_Close(rtmp->rtmp_h264);
        RTMP_Free(rtmp->rtmp_h264);
        return -1;
    }

    return 0;  
}


static int rtmp_h264_send_pps_sps(RTMP* rtmp_h264, const char *pps, const int pps_len, const char *sps, const int sps_len)
{
    int i = 0;
    int ret = 0;
    RTMPPacket packet;
    char *body = NULL;
    int packet_size = RTMP_MAX_HEADER_SIZE + pps_len + sps_len;

    if (pps == NULL || sps == NULL)
    {
        printf("pps or sps is NULL.\n");
        return -1;
    }
    
    memset(&packet, 0, sizeof(RTMPPacket));
    RTMPPacket_Alloc(&packet, packet_size);
    RTMPPacket_Reset(&packet);
    body = packet.m_body;

    body[i++] = 0x17;
    body[i++] = 0x00;

    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;

    body[i++] = 0x01;
    body[i++] = sps[1];
    body[i++] = sps[2];
    body[i++] = sps[3];
    body[i++] = 0xff;

    body[i++] = 0xe1;
    body[i++] = (sps_len >> 8) & 0xff;
    body[i++] = sps_len & 0xff;
    memcpy(&body[i],sps,sps_len);
    i +=  sps_len;

    /*pps*/
    body[i++]   = 0x01;
    body[i++] = (pps_len >> 8) & 0xff;
    body[i++] = (pps_len) & 0xff;
    memcpy(&body[i],pps,pps_len);
    i +=  pps_len;

    packet.m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet.m_nBodySize = i;
    packet.m_nChannel = 0x04;
    packet.m_nTimeStamp = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet.m_nInfoField2 = rtmp_h264->m_stream_id;

    ret = RTMP_SendPacket(rtmp_h264, &packet, TRUE);

    RTMPPacket_Free(&packet);
    return ret;
}



int rtmp_h264_send(RtmpInfo_t *rtmp, H264Packet_t *nal, unsigned int time_stamp)
{
    int i = 0;
    int ret = -1;
    RTMPPacket packet;
    char *body = NULL;

    if (rtmp == NULL)
    {
        printf("rtmp info is NULL.\n");
        return -1;
    }
    if (nal == NULL)
    {
        printf("h264 data is NULL.\n");
        return -1;
    }

    memset(&packet, 0, sizeof(RTMPPacket));
    RTMPPacket_Alloc(&packet, RTMP_MAX_HEADER_SIZE + nal->frame_size);
    RTMPPacket_Reset(&packet);
    body = packet.m_body;

#if 1
    if (nal->is_iframe)
    {
        rtmp_h264_send_pps_sps(rtmp->rtmp_h264, nal->pps, nal->pps_size, nal->sps, nal->sps_size);

        body[i++] = 0x17;
        body[i++] = 0x01;
        body[i++] = 0x00;
        body[i++] = 0x00;
        body[i++] = 0x00;
    }
    else
    {
        body[i++] = 0x27;
        body[i++] = 0x01;
        body[i++] = 0x00;
        body[i++] = 0x00;
        body[i++] = 0x00;
    }

    body[i++] = nal->frame_size >> 24 & 0xff;
    body[i++] = nal->frame_size >> 16 & 0xff;
    body[i++] = nal->frame_size >> 8  & 0xff;
    body[i++] = nal->frame_size       & 0xff;
    memcpy(&body[i], nal->frame, nal->frame_size);
    i += nal->frame_size;
#endif
//    memcpy(packet.m_body, data, size);
    packet.m_nBodySize = i;
    packet.m_hasAbsTimestamp = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet.m_nInfoField2 = rtmp->rtmp_h264->m_stream_id;
    packet.m_nChannel = 0x04;
    packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet.m_nTimeStamp = time_stamp;

    if (RTMP_IsConnected(rtmp->rtmp_h264))
    {        
        ret = RTMP_SendPacket(rtmp->rtmp_h264, &packet, 0);
        if (ret <= 0)
        {
            printf("RTMP_SendPacket failed.\n");
        }
    }

    RTMPPacket_Free(&packet);
    return ret;
}



int rtmp_h264_close(RtmpInfo_t *rtmp)
{
    if (rtmp == NULL)
    {
        printf("rtmp info is NULL.\n");
        return -1;
    }

    if (rtmp->rtmp_h264)
    {
        RTMP_Close(rtmp->rtmp_h264);
        RTMP_Free(rtmp->rtmp_h264);
        rtmp->rtmp_h264 = NULL;
    }

    return 0;
}

NalType_e rtmp_h264_get_nal(RtmpInfo_t *rtmp, H264Packet_t *nal)
{
    NalType_e ret = H264_UNKNOW;
    int nal_head_pos = rtmp->nal_head_pos;
    int file_size = rtmp->size;
    char *data = rtmp->h264_data;
    int nal_tail_pos = 0;
    int nal_size = 0;
    int nal_type = 0;

    if (rtmp->nal_head_pos != 0 && nal_head_pos < file_size)
    {
        goto get_nal;
    }

    while (nal_head_pos < file_size)
    {
        if (data[nal_head_pos++] == 0x00 && data[nal_head_pos++] == 0x00)           /*找起始码*/
        {
            if (data[nal_head_pos] == 0x01)                                         /*起始码 00 00 01*/
            {
                nal_head_pos++;                                                     /*nal_head_pos指向nal type地址*/
                goto get_nal;
            }
            else if (data[nal_head_pos++] == 0x00 && data[nal_head_pos++] == 0x01)  /*起始码00 00 00 01 nal_head_pos指向nal type地址*/
            {
                goto get_nal;
            }
            else
            {
                continue;                                                           /*找到00 00,但后面不对,跳过继续找*/
            }
        }
        else
        {
            continue;                                                               /*00 00都没有找到,跳过继续找*/
        }

get_nal:
//        printf("  %s : nal_head_pos = [%d].\n", __func__, nal_head_pos);
        nal_tail_pos = nal_head_pos;
        while (nal_tail_pos < file_size)
        {
            if (data[nal_tail_pos++] == 0x00 && data[nal_tail_pos++] == 0x00)           /*找起始码*/
            {
                if (data[nal_tail_pos] == 0x01)                                         /*起始码 00 00 01*/
                {
                    nal_tail_pos++;                                                     /*nal_tail_pos指向nal type地址*/
                    nal_size = nal_tail_pos - nal_head_pos - 3;                    
                    break;
                }
                else if (data[nal_tail_pos++] == 0x00 && data[nal_tail_pos++] == 0x01)  /*起始码00 00 00 01 nal_tail_pos指向nal type地址*/
                {
                    nal_size = nal_tail_pos - nal_head_pos - 4;
                    break;
                }
            }
        }

//        printf("  %s : nal_tail_pos = [%d].\n", __func__, nal_tail_pos);
        nal_size = nal_size == 0 ? nal_tail_pos - nal_head_pos : nal_size;
        nal_type = data[nal_head_pos] & 0x1F;
//        printf("%s : nal_type = [%d].\n", __func__, nal_type);

        if (nal_type == 0x7)
        {
            nal->sps = &data[nal_head_pos];
            nal->sps_size = nal_size;
            ret = H264_SPS;
        }
        else if (nal_type == 0x8)
        {
            nal->pps = &data[nal_head_pos];
            nal->pps_size = nal_size;
            ret = H264_PPS;
        }
        else if (nal_type == 0x5)
        {
            nal->frame = &data[nal_head_pos];
            nal->frame_size = nal_size;
            nal->is_iframe = 1;
            ret = H264_I_FRAME;
        }
        else if (nal_type == 0x1)
        {
            nal->frame = &data[nal_head_pos];
            nal->frame_size = nal_size;
            nal->is_iframe = 0;
            ret = H264_P_FRAME;
        }

        rtmp->nal_head_pos = nal_tail_pos;
        break;
    }

    return ret;
}


