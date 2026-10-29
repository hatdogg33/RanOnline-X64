#pragma once

#include <cstdint>
#include <cmath>

#ifndef MAX_TOUCH_POINTERS
#define MAX_TOUCH_POINTERS 10
#endif

enum class TouchEventType {
    DOWN,
    MOVE,
    UP,
    CANCEL
};

struct TouchPoint {
    int32_t id;
    float x;
    float y;
    float startX;
    float startY;
    bool active;
};

struct VirtualJoystickState {
    bool active;
    int32_t pointerId;
    float originX;
    float originY;
    float currentX;
    float currentY;
    float dirX;      // Normalized [-1.0f .. 1.0f]
    float dirY;      // Normalized [-1.0f .. 1.0f] (Y+ forward)
    float intensity; // [0.0f .. 1.0f]
};

struct MobileCameraPanState {
    bool active;
    int32_t pointerId;
    float lastX;
    float lastY;
    float deltaX;    // Yaw delta
    float deltaY;    // Pitch delta
};

enum class MobileActionButton {
    NONE = 0,
    ATTACK = 1,
    SKILL_1 = 2,
    SKILL_2 = 3,
    SKILL_3 = 4,
    SKILL_4 = 5,
    INTERACT_NPC = 6,
    POTION_HP = 7,
    POTION_MP = 8
};

struct ActionButtonArea {
    MobileActionButton buttonId;
    float centerX;
    float centerY;
    float radius;
};

class MobileTouchSystem {
public:
    static MobileTouchSystem& GetInstance() {
        static MobileTouchSystem instance;
        return instance;
    }

    void Init(int screenWidth, int screenHeight);
    void UpdateDimensions(int screenWidth, int screenHeight);

    // Feed Android AMOTION_EVENT
    void OnTouchEvent(TouchEventType type, int32_t pointerId, float x, float y);

    // State getters
    const VirtualJoystickState& GetJoystick() const { return m_joystick; }
    const MobileCameraPanState& GetCameraPan() const { return m_cameraPan; }
    void ResetCameraDeltas();

    // Query if an action button was triggered this frame
    bool ConsumeButtonPress(MobileActionButton btn);

private:
    MobileTouchSystem();

    int m_screenWidth;
    int m_screenHeight;

    TouchPoint m_pointers[MAX_TOUCH_POINTERS];
    VirtualJoystickState m_joystick;
    MobileCameraPanState m_cameraPan;

    ActionButtonArea m_buttons[10];
    int m_buttonCount;
    bool m_buttonTriggered[16];

    void SetupDefaultButtons();
    MobileActionButton HitTestButtons(float x, float y);
};
