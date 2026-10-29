package com.ranonline.mobile;

import android.app.Activity;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;

public class RanNativeActivity extends Activity implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("ran_engine");
    }

    private SurfaceView mSurfaceView;
    private boolean mIsRunning = false;
    private Thread mRenderThread;

    public native void nativeInitSurface(Surface surface, int width, int height);
    public native void nativeRenderFrame(float elapsedTime);
    public native void nativeTouchEvent(int action, int pointerId, float x, float y);
    public native void nativeDestroySurface();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Keep screen on & immersive full screen for mobile gaming
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
        );

        mSurfaceView = new SurfaceView(this);
        mSurfaceView.getHolder().addCallback(this);
        setContentView(mSurfaceView);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int actionMasked = event.getActionMasked();
        int actionIndex = event.getActionIndex();
        int pointerId = event.getPointerId(actionIndex);

        int nativeAction = -1;
        switch (actionMasked) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_DOWN:
                nativeAction = 0;
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
                nativeAction = 1;
                break;
            case MotionEvent.ACTION_MOVE:
                nativeAction = 2;
                break;
            case MotionEvent.ACTION_CANCEL:
                nativeAction = 3;
                break;
        }

        if (nativeAction != -1) {
            if (nativeAction == 2) {
                // Send all active pointers on move
                for (int i = 0; i < event.getPointerCount(); i++) {
                    nativeTouchEvent(2, event.getPointerId(i), event.getX(i), event.getY(i));
                }
            } else {
                nativeTouchEvent(nativeAction, pointerId, event.getX(actionIndex), event.getY(actionIndex));
            }
        }
        return true;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {}

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        nativeInitSurface(holder.getSurface(), width, height);

        mIsRunning = true;
        mRenderThread = new Thread(() -> {
            long lastTime = System.nanoTime();
            while (mIsRunning) {
                long currentTime = System.nanoTime();
                float elapsedTime = (currentTime - lastTime) / 1_000_000_000.0f;
                lastTime = currentTime;

                nativeRenderFrame(elapsedTime);

                try {
                    Thread.sleep(16); // Target ~60 FPS
                } catch (InterruptedException ignored) {}
            }
        });
        mRenderThread.start();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mIsRunning = false;
        try {
            if (mRenderThread != null) mRenderThread.join();
        } catch (InterruptedException ignored) {}
        nativeDestroySurface();
    }
}
