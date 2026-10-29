#include "MobileTouchSystem.h"
#include <cstring>
#include <algorithm>

MobileTouchSystem::MobileTouchSystem()
    : m_screenWidth(1280), m_screenHeight(720), m_buttonCount(0) {
    std::memset(m_pointers, 0, sizeof(m_pointers));
    std::memset(&m_joystick, 0, sizeof(m_joystick));
    std::memset(&m_cameraPan, 0, sizeof(m_cameraPan));
    std::memset(m_buttonTriggered, 0, sizeof(m_buttonTriggered));
}

void MobileTouchSystem::Init(int screenWidth, int screenHeight) {
    UpdateDimensions(screenWidth, screenHeight);
}

void MobileTouchSystem::UpdateDimensions(int screenWidth, int screenHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    SetupDefaultButtons();
}

void MobileTouchSystem::SetupDefaultButtons() {
    m_buttonCount = 0;

    // Right-hand side Action Cluster
    float attackX = m_screenWidth - 120.0f;
    float attackY = m_screenHeight - 120.0f;
    float attackRadius = 60.0f;

    // Main Attack
    m_buttons[m_buttonCount++] = { MobileActionButton::ATTACK, attackX, attackY, attackRadius };

    // Skill 1 (Top of Attack)
    m_buttons[m_buttonCount++] = { MobileActionButton::SKILL_1, attackX - 10.0f, attackY - 110.0f, 38.0f };

    // Skill 2 (Top-Left of Attack)
    m_buttons[m_buttonCount++] = { MobileActionButton::SKILL_2, attackX - 90.0f, attackY - 80.0f, 38.0f };

    // Skill 3 (Left of Attack)
    m_buttons[m_buttonCount++] = { MobileActionButton::SKILL_3, attackX - 120.0f, attackY, 38.0f };

    // Skill 4 (Bottom-Left of Attack)
    m_buttons[m_buttonCount++] = { MobileActionButton::SKILL_4, attackX - 180.0f, attackY + 20.0f, 34.0f };

    // NPC / Interact button
    m_buttons[m_buttonCount++] = { MobileActionButton::INTERACT_NPC, attackX - 110.0f, attackY + 70.0f, 34.0f };

    // HP & MP Quick Potions (Near bottom middle)
    m_buttons[m_buttonCount++] = { MobileActionButton::POTION_HP, m_screenWidth * 0.5f - 40.0f, m_screenHeight - 50.0f, 30.0f };
    m_buttons[m_buttonCount++] = { MobileActionButton::POTION_MP, m_screenWidth * 0.5f + 40.0f, m_screenHeight - 50.0f, 30.0f };
}

MobileActionButton MobileTouchSystem::HitTestButtons(float x, float y) {
    for (int i = 0; i < m_buttonCount; ++i) {
        float dx = x - m_buttons[i].centerX;
        float dy = y - m_buttons[i].centerY;
        if ((dx * dx + dy * dy) <= (m_buttons[i].radius * m_buttons[i].radius)) {
            return m_buttons[i].buttonId;
        }
    }
    return MobileActionButton::NONE;
}

void MobileTouchSystem::OnTouchEvent(TouchEventType type, int32_t pointerId, float x, float y) {
    float joyRegionWidth = m_screenWidth * 0.40f;

    if (type == TouchEventType::DOWN) {
        // Check UI buttons first
        MobileActionButton btn = HitTestButtons(x, y);
        if (btn != MobileActionButton::NONE) {
            m_buttonTriggered[static_cast<int>(btn)] = true;
            return;
        }

        // Left quadrant: Virtual Joystick Touch
        if (x < joyRegionWidth && !m_joystick.active) {
            m_joystick.active = true;
            m_joystick.pointerId = pointerId;
            m_joystick.originX = x;
            m_joystick.originY = y;
            m_joystick.currentX = x;
            m_joystick.currentY = y;
            m_joystick.dirX = 0.0f;
            m_joystick.dirY = 0.0f;
            m_joystick.intensity = 0.0f;
            return;
        }

        // Right quadrant: Camera Pan
        if (x >= joyRegionWidth && !m_cameraPan.active) {
            m_cameraPan.active = true;
            m_cameraPan.pointerId = pointerId;
            m_cameraPan.lastX = x;
            m_cameraPan.lastY = y;
            m_cameraPan.deltaX = 0.0f;
            m_cameraPan.deltaY = 0.0f;
            return;
        }
    }
    else if (type == TouchEventType::MOVE) {
        if (m_joystick.active && m_joystick.pointerId == pointerId) {
            m_joystick.currentX = x;
            m_joystick.currentY = y;

            float dx = x - m_joystick.originX;
            float dy = y - m_joystick.originY;
            float dist = std::sqrt(dx * dx + dy * dy);
            float maxRadius = 80.0f;

            if (dist > 8.0f) {
                m_joystick.dirX = dx / dist;
                m_joystick.dirY = -dy / dist; // Forward is positive Y
                m_joystick.intensity = std::min(dist / maxRadius, 1.0f);
            } else {
                m_joystick.dirX = 0.0f;
                m_joystick.dirY = 0.0f;
                m_joystick.intensity = 0.0f;
            }
        }
        else if (m_cameraPan.active && m_cameraPan.pointerId == pointerId) {
            m_cameraPan.deltaX += (x - m_cameraPan.lastX);
            m_cameraPan.deltaY += (y - m_cameraPan.lastY);
            m_cameraPan.lastX = x;
            m_cameraPan.lastY = y;
        }
    }
    else if (type == TouchEventType::UP || type == TouchEventType::CANCEL) {
        if (m_joystick.active && m_joystick.pointerId == pointerId) {
            m_joystick.active = false;
            m_joystick.dirX = 0.0f;
            m_joystick.dirY = 0.0f;
            m_joystick.intensity = 0.0f;
        }
        if (m_cameraPan.active && m_cameraPan.pointerId == pointerId) {
            m_cameraPan.active = false;
            m_cameraPan.deltaX = 0.0f;
            m_cameraPan.deltaY = 0.0f;
        }
    }
}

void MobileTouchSystem::ResetCameraDeltas() {
    m_cameraPan.deltaX = 0.0f;
    m_cameraPan.deltaY = 0.0f;
}

bool MobileTouchSystem::ConsumeButtonPress(MobileActionButton btn) {
    int idx = static_cast<int>(btn);
    if (idx >= 0 && idx < 16 && m_buttonTriggered[idx]) {
        m_buttonTriggered[idx] = false;
        return true;
    }
    return false;
}
