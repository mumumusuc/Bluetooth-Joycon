#include "com_mumumusuc_joycon_Joycon.h"
#include "include/joycon.h"
#include <android/log.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#define TAG         "joycon_jni"

#include "include/jni_utils.h"

static struct jc_dev {
    joycon_t jc;
    uint8_t *input_buffer;
    uint8_t *output_buffer;
    jc_status_t last_status;
} jcdev;

static inline void notify_Java_status_changed(JNIEnv *env, jobject obj, jc_status_t *status) {
    jclass clazz = (*env)->GetObjectClass(env, obj);
    jmethodID jMethod = (*env)->GetMethodID(env, clazz, "onNativeStatusChanged", "(BZZZZ)V");
    if (!jMethod) {
        LOGE("[%s] error get method : onNativeStatusChanged", __func__);
        return;
    }
    (*env)->CallVoidMethod(env, obj, jMethod,
                           status->player,
                           status->enable_vib,
                           status->enable_imu,
                           status->start_push,
                           status->enable_nfc);
}

static void check_jc_status(JNIEnv *env, jobject obj) {
    bool notify = false;
    jc_status_t *last = &jcdev.last_status;
    jc_status_t *current = jcdev.jc.status;
    if (last->enable_vib != current->enable_vib)
        notify = true;
    if (last->enable_imu != current->enable_imu)
        notify = true;
    if (last->player != current->player)
        notify = true;
    if (last->start_push != current->start_push)
        notify = true;
    if (last->enable_nfc != current->enable_nfc)
        notify = true;
    /* call Java method */
    if (notify) {
        notify_Java_status_changed(env, obj, current);
        current->start_push = false;
    }

    memmove(last, current, sizeof(jc_status_t));
}

JNIEXPORT jint JNICALL Java_com_mumumusuc_joycon_Joycon_nativeInit
        (JNIEnv *env, jobject _, jobject byteBuffer, jobject assetManager, jstring file) {
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
    if (!mgr) {
        LOGE("error get AAssetManager");
        return -errno;
    }
    jboolean is_copy;
    const char *name = (*env)->GetStringUTFChars(env, file, &is_copy);
    LOGD("assets name = %s", name);
    AAsset *asset = AAssetManager_open(mgr, name, AASSET_MODE_BUFFER);
    (*env)->ReleaseStringUTFChars(env, file, name);
    if (!asset) {
        LOGE("error AAssetManager_open");
        return -errno;
    }
    loff_t start, len;
    int fd = AAsset_openFileDescriptor(asset, &start, &len);
    LOGD("asset start = %lu , len = %lu", start, len);
    AAsset_close(asset);
    if (fd < 0) {
        LOGE("error AAsset_openFileDescriptor : %s", strerror(errno));
        return -errno;
    }
    LOGD("asset fd = %d , INPUT_SIZE = %d , STATUS_SIZE = %d", fd, JC_INPUT_SIZE, JC_STATUS_SIZE);
    void *buffer = (*env)->GetDirectBufferAddress(env, byteBuffer);
    jlong capacity = (*env)->GetDirectBufferCapacity(env, byteBuffer);
    if (!buffer || capacity <= 0) {
        LOGE("error get sharedBuffer : %s", strerror(errno));
        return -errno;
    }
    LOGD("get sharedBuffer(%p,%lu)", buffer, capacity);
    jcdev.jc.spi_fd = fd;
    jcdev.jc.offset = start;
    ssize_t offset = 0;
    /* anchor output report */
    jcdev.output_buffer = buffer + offset;
    offset += REPORT_OUTPUT_SIZE + 1;
    /* anchor input report */
    jcdev.input_buffer = buffer + offset;
    offset += REPORT_INPUT_LARGE_SIZE + 1;
    /* anchor input */
    jcdev.jc.input = buffer + offset;
    offset += JC_INPUT_SIZE;
    /* anchor status */
    jcdev.jc.status = buffer + offset;
    //offset += JC_STATUS_SIZE;
    int ret = jc_init(&jcdev.jc);
    if (ret < 0) {
        LOGE("error init joycon");
        close(fd);
        return ret;
    }
    memmove(&jcdev.last_status, &jcdev.jc.status, sizeof(jc_status_t));
    return 0;
}

JNIEXPORT jint JNICALL Java_com_mumumusuc_joycon_Joycon_nativeFree
        (JNIEnv *env, jobject obj) {
    jc_free(&jcdev.jc);
    return 0;
}

JNIEXPORT jint JNICALL Java_com_mumumusuc_joycon_Joycon_nativeReplayOutputReport
        (JNIEnv *env, jobject obj) {
    jint ret = 0;
    joycon_t *jc = &jcdev.jc;
    uint8_t *input = jcdev.input_buffer;
    uint8_t *output = jcdev.output_buffer;
    memset(input, 0, REPORT_INPUT_SIZE);
    ret = jc_replay_output_report(jc, output, input);
    hexdump(OUTPUT_TAG, output, REPORT_OUTPUT_SIZE + 1);
    check_jc_status(env, obj);
    ret = jc_makeup_input_report(jc, input);
    hexdump(INPUT_TAG, input, ret);
    memset(output, 0, REPORT_OUTPUT_SIZE + 1);
    return ret;
}

JNIEXPORT jint JNICALL Java_com_mumumusuc_joycon_Joycon_nativeMakeupInputReport
        (JNIEnv *env, jobject obj) {
    jint ret = 0;
    joycon_t *jc = &jcdev.jc;
    uint8_t *input = jcdev.input_buffer;
    memset(input + 1, 0, REPORT_INPUT_SIZE);
    ret = jc_makeup_input_report(jc, input);
    hexdump(INPUT_TAG, input, ret);
    return ret;
}

JNIEXPORT void JNICALL Java_com_mumumusuc_joycon_Joycon_nativeUpdateButtons
        (JNIEnv *env, jobject _, jbooleanArray buttons) {
    joycon_t *jc = &jcdev.jc;
    uint32_t button = 0;
    int len = (*env)->GetArrayLength(env, buttons);
    jboolean *values = (*env)->GetBooleanArrayElements(env, buttons, JNI_FALSE);
    for (int i = 0; i < len; i++) {
        if (values[i])
            button |= NS_BTN_MASK(i);
    }
    (*env)->ReleaseBooleanArrayElements(env, buttons, values, JNI_FALSE);
    jc->input->buttons = button;
}

JNIEXPORT void JNICALL Java_com_mumumusuc_joycon_Joycon_nativeUpdateSticks
        (JNIEnv *env, jobject _, jcharArray sticks) {
    joycon_t *jc = &jcdev.jc;
    int len = (*env)->GetArrayLength(env, sticks);
    (*env)->GetCharArrayRegion(env, sticks, 0, len, jc->input->sticks);
}

JNIEXPORT void JNICALL Java_com_mumumusuc_joycon_Joycon_nativeUpdateIMU
        (JNIEnv *env, jobject _, jshortArray accl, jshortArray gyro) {
    joycon_t *jc = &jcdev.jc;
    if (accl) {
        int len = (*env)->GetArrayLength(env, accl) / 3;
        for (int i = 0; i < 3; i++) {
            (*env)->GetShortArrayRegion(env, accl, len * i, len, jc->input->axes[i]);
        }
    }
    if (gyro) {
        int len = (*env)->GetArrayLength(env, gyro) / 3;
        for (int i = 0; i < 3; i++) {
            (*env)->GetShortArrayRegion(env, gyro, len * i, len, &jc->input->axes[i]);
        }
    }
}

JNIEXPORT void JNICALL Java_com_mumumusuc_joycon_Joycon_nativeUpdateNFC
        (JNIEnv *env, jobject _, jbyteArray nfc) {

}

JNIEXPORT void JNICALL Java_com_mumumusuc_joycon_Joycon_nativeUpdateIR
        (JNIEnv *env, jobject _, jbyteArray ir) {

}

