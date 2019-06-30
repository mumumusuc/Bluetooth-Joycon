#ifndef DROIDPRO_UTILS_H
#define DROIDPRO_UTILS_H

#define DEBUG 1
#define INPUT_TAG   "[INP]"
#define OUTPUT_TAG  "[OUT]"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

#if DEBUG

#include <stdio.h>
#include <memory.h>

#define TMP_SIZE    128
#define TAG "jc_jni"
static char tmp[TMP_SIZE] = {0};

#define dump(tag, fmt, buf, size)                   \
do{                                                 \
    int len = 0;                                    \
    char *_tmp = tmp;                               \
    memset(_tmp, 0, TMP_SIZE);                      \
    if ((tag)) {                                    \
        len = sprintf(_tmp, "%s", (tag));           \
        _tmp += len;                                \
    }                                               \
    for (int _i = 0; _i < (size); _i++) {           \
        if(_i > 64 && _i < 320)                     \
            continue;                               \
        if(_i == 64)                                \
            len = sprintf(_tmp,"... ");             \
        else                                        \
            len = sprintf(_tmp, (fmt), (buf)[_i]);  \
        _tmp += len;                                \
    }                                               \
    LOGD("%s",tmp);                                      \
}while(0)

#define hexdump(tag, buf, size)   dump(tag,"%02x ",buf,size)

#else
#define hexdump(tag,buf,size)   {}
#endif

#endif //DROIDPRO_UTILS_H
