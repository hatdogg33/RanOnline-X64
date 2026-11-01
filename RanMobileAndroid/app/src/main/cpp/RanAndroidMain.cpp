#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "MobileTouchSystem.h"

#define LOG_TAG "RanOnlineMobile"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static EGLDisplay g_EglDisplay = EGL_NO_DISPLAY;
static EGLSurface g_EglSurface = EGL_NO_SURFACE;
static EGLContext g_EglContext = EGL_NO_CONTEXT;
static ANativeWindow* g_NativeWindow = nullptr;
static bool g_EglReady = false;
static int g_ScreenWidth = 1280;
static int g_ScreenHeight = 720;

static void LogEglError(const char* operation) {
    const EGLint error = eglGetError();
    LOGE("%s failed: EGL error 0x%x", operation, error);
}

extern "C" {

JNIEXPORT void JNICALL
Java_com_ranonline_mobile_RanNativeActivity_nativeInitSurface(
    JNIEnv* env, jobject /* thiz */, jobject surface, jint width, jint height) {
    LOGI("Initializing EGL surface: %d x %d", width, height);

    g_ScreenWidth = width;
    g_ScreenHeight = height;

    g_NativeWindow = ANativeWindow_fromSurface(env, surface);
    if (!g_NativeWindow) {
        LOGE("ANativeWindow_fromSurface returned null");
        return;
    }

    g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_EglDisplay == EGL_NO_DISPLAY) {
        LogEglError("eglGetDisplay");
        ANativeWindow_release(g_NativeWindow);
        g_NativeWindow = nullptr;
        return;
    }

    if (eglInitialize(g_EglDisplay, nullptr, nullptr) != EGL_TRUE) {
        LogEglError("eglInitialize");
        eglTerminate(g_EglDisplay);
        g_EglDisplay = EGL_NO_DISPLAY;
        ANativeWindow_release(g_NativeWindow);
        g_NativeWindow = nullptr;
        return;
    }

    const EGLint configAttributes[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_NONE
    };

    EGLConfig config = nullptr;
    EGLint configCount = 0;
    if (eglChooseConfig(g_EglDisplay, configAttributes, &config, 1, &configCount) != EGL_TRUE ||
        configCount < 1) {
        LogEglError("eglChooseConfig");
        eglTerminate(g_EglDisplay);
        g_EglDisplay = EGL_NO_DISPLAY;
        ANativeWindow_release(g_NativeWindow);
        g_NativeWindow = nullptr;
        return;
    }

    const EGLint contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    g_EglContext = eglCreateContext(g_EglDisplay, config, EGL_NO_CONTEXT, contextAttributes);
    if (g_EglContext == EGL_NO_CONTEXT) {
        LogEglError("eglCreateContext");
        eglTerminate(g_EglDisplay);
        g_EglDisplay = EGL_NO_DISPLAY;
        ANativeWindow_release(g_NativeWindow);
        g_NativeWindow = nullptr;
        return;
    }

    g_EglSurface = eglCreateWindowSurface(g_EglDisplay, config, g_NativeWindow, nullptr);
    if (g_EglSurface == EGL_NO_SURFACE) {
        LogEglError("eglCreateWindowSurface");
        eglDestroyContext(g_EglDisplay, g_EglContext);
        g_EglContext = EGL_NO_CONTEXT;
        eglTerminate(g_EglDisplay);
        g_EglDisplay = EGL_NO_DISPLAY;
        ANativeWindow_release(g_NativeWindow);
        g_NativeWindow = nullptr;
        return;
    }

    // Do not call eglMakeCurrent here. This JNI method runs on the UI thread,
    // while nativeRenderFrame runs on the dedicated render thread. EGL contexts
    // are thread-affine, so the render thread must bind the context itself.
    MobileTouchSystem::GetInstance().Init(width, height);
    g_EglReady = true;
    LOGI("EGL resources created; context will be bound by the render thread");
}

JNIEXPORT void JNICALL
Java_com_ranonline_mobile_RanNativeActivity_nativeRenderFrame(
    JNIEnv* /* env */, jobject /* thiz */, jfloat /* elapsedTime */) {
    if (!g_EglReady || g_EglDisplay == EGL_NO_DISPLAY ||
        g_EglSurface == EGL_NO_SURFACE || g_EglContext == EGL_NO_CONTEXT) {
        return;
    }

    // Bind the EGL context on the same thread that issues GLES commands.
    if (eglGetCurrentContext() != g_EglContext) {
        if (eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext) != EGL_TRUE) {
            LogEglError("eglMakeCurrent on render thread");
            return;
        }
        LOGI("EGL context bound to render thread");
    }

    glViewport(0, 0, g_ScreenWidth, g_ScreenHeight);
    glClearColor(0.08f, 0.10f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (eglSwapBuffers(g_EglDisplay, g_EglSurface) != EGL_TRUE) {
        LogEglError("eglSwapBuffers");
    }
}

JNIEXPORT void JNICALL
Java_com_ranonline_mobile_RanNativeActivity_nativeTouchEvent(
    JNIEnv* /* env */, jobject /* thiz */, jint action, jint pointerId, jfloat x, jfloat y) {
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
    JNIEnv* /* env */, jobject /* thiz */) {
    g_EglReady = false;

    if (g_EglDisplay != EGL_NO_DISPLAY) {
        // The Java render thread is joined before this method is called.
        eglMakeCurrent(g_EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (g_EglSurface != EGL_NO_SURFACE) {
            eglDestroySurface(g_EglDisplay, g_EglSurface);
        }
        if (g_EglContext != EGL_NO_CONTEXT) {
            eglDestroyContext(g_EglDisplay, g_EglContext);
        }
        eglTerminate(g_EglDisplay);
    }

    g_EglDisplay = EGL_NO_DISPLAY;
    g_EglSurface = EGL_NO_SURFACE;
    g_EglContext = EGL_NO_CONTEXT;

    if (g_NativeWindow) {
        ANativeWindow_release(g_NativeWindow);
        g_NativeWindow = nullptr;
    }

    LOGI("EGL resources destroyed");
}

}
