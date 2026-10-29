#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "MobileTouchSystem.h"
#include "RanMobileInputBridge.h"

#define LOG_TAG "RanOnlineMobile"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static EGLDisplay g_EglDisplay = EGL_NO_DISPLAY;
static EGLSurface g_EglSurface = EGL_NO_SURFACE;
static EGLContext g_EglContext = EGL_NO_CONTEXT;
static int g_ScreenWidth = 1280;
static int g_ScreenHeight = 720;

extern "C" {

JNIEXPORT void JNICALL
Java_com_ranonline_mobile_RanNativeActivity_nativeInitSurface(
    JNIEnv* env, jobject thiz, jobject surface, jint width, jint height) {
    LOGI("nativeInitSurface called with width: %d, height: %d", width, height);

    g_ScreenWidth = width;
    g_ScreenHeight = height;

    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) {
        LOGE("Failed to get ANativeWindow from surface");
        return;
    }

    g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(g_EglDisplay, nullptr, nullptr);

    const EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs;
    eglChooseConfig(g_EglDisplay, attribs, &config, 1, &numConfigs);

    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    g_EglContext = eglCreateContext(g_EglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    g_EglSurface = eglCreateWindowSurface(g_EglDisplay, config, window, nullptr);

    eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext);

    glViewport(0, 0, width, height);

    MobileTouchSystem::GetInstance().Init(width, height);
    LOGI("Ran Online Mobile Renderer & Touch Subsystem Initialized successfully");
}

JNIEXPORT void JNICALL
Java_com_ranonline_mobile_RanNativeActivity_nativeRenderFrame(
    JNIEnv* env, jobject thiz, jfloat elapsedTime) {

    // 1. Update Ran Mobile Touch Movement and Action inputs
    RanMobileInputBridge::GetInstance().UpdateMovementAndActions(elapsedTime);

    // 2. Render background / frame
    glClearColor(0.22f, 0.35f, 0.50f, 1.0f); // Ran Online classic sky clear color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 3. Swap buffers
    eglSwapBuffers(g_EglDisplay, g_EglSurface);
}

JNIEXPORT void JNICALL
Java_com_ranonline_mobile_RanNativeActivity_nativeTouchEvent(
    JNIEnv* env, jobject thiz, jint action, jint pointerId, jfloat x, jfloat y) {

    TouchEventType type;
    switch (action) {
        case 0: type = TouchEventType::DOWN; break;
        case 1: type = TouchEventType::UP; break;
        case 2: type = TouchEventType::MOVE; break;
        default: type = TouchEventType::CANCEL; break;
    }

    MobileTouchSystem::GetInstance().OnTouchEvent(type, pointerId, x, y);
}

JNIEXPORT void JNICALL
Java_com_ranonline_mobile_RanNativeActivity_nativeDestroySurface(
    JNIEnv* env, jobject thiz) {
    if (g_EglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (g_EglSurface != EGL_NO_SURFACE) eglDestroySurface(g_EglDisplay, g_EglSurface);
        if (g_EglContext != EGL_NO_CONTEXT) eglDestroyContext(g_EglDisplay, g_EglContext);
        eglTerminate(g_EglDisplay);
    }
    g_EglDisplay = EGL_NO_DISPLAY;
    g_EglSurface = EGL_NO_SURFACE;
    g_EglContext = EGL_NO_CONTEXT;
    LOGI("Ran Online Mobile Renderer Destroyed");
}

}
