# rtmpdump_h264
rtmp协议推流h264数据,依赖rtmpdump库<br>
rtmpdump项目：git://git.ffmpeg.org/rtmpdump<br>
编译：
1. 先编译rtmpdump库,安装路径随意，使用时能找到就行
2. 再编译此项目，确保编译或执行时能找到rtmpdump库就可以<br>
执行：
1. ./rtmp xxx.h264 rtmp://\<ip\>/\<app\>/\<stream name\>
2. eg：./rtmp english.h264 rtmp://192.168.10.21/live/english.h264
