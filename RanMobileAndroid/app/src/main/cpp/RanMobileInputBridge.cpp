#include "RanMobileInputBridge.h"
#include <cmath>

// Include Ran Core Headers
#include "DxViewPort.h"
#include "GLGaeaClient.h"
#include "GLCharacter.h"
#include "InnerInterface.h"

void RanMobileInputBridge::UpdateMovementAndActions(float fElapsedTime) {
    MobileTouchSystem& touchSys = MobileTouchSystem::GetInstance();
    GLCharacter* pChar = GLGaeaClient::GetInstance().GetCharacter();
    if (!pChar) return;

    // 1. Process Virtual Joystick Movement
    const VirtualJoystickState& joy = touchSys.GetJoystick();
    if (joy.active && joy.intensity > 0.05f) {
        // Retrieve 3D camera forward and right directions
        D3DXVECTOR3 vFrom = DxViewPort::GetInstance().GetFromPt();
        D3DXVECTOR3 vLookAt = DxViewPort::GetInstance().GetLookatPt();
        D3DXVECTOR3 vCamForward = vLookAt - vFrom;
        vCamForward.y = 0.0f;
        D3DXVec3Normalize(&vCamForward, &vCamForward);

        D3DXVECTOR3 vCamRight;
        D3DXVECTOR3 vUp(0.0f, 1.0f, 0.0f);
        D3DXVec3Cross(&vCamRight, &vUp, &vCamForward);
        D3DXVec3Normalize(&vCamRight, &vCamRight);

        // Calculate world move direction
        // joy.dirY is forward/back, joy.dirX is right/left
        D3DXVECTOR3 vWorldMoveDir = (vCamForward * joy.dirY) + (vCamRight * joy.dirX);
        D3DXVec3Normalize(&vWorldMoveDir, &vWorldMoveDir);

        // Target movement position ahead of character
        D3DXVECTOR3 vCharPos = pChar->GetPosition();
        float stepDist = pChar->GetMoveVelo() * fElapsedTime * (1.2f + joy.intensity * 2.0f);
        D3DXVECTOR3 vTargetPos = vCharPos + (vWorldMoveDir * stepDist);

        // Direct Ran character controller pathfinder
        pChar->ActionMoveTo(fElapsedTime, vCharPos, vTargetPos, TRUE, FALSE);
    }

    // 2. Process Mobile Action Buttons
    if (touchSys.ConsumeButtonPress(MobileActionButton::ATTACK)) {
        // Trigger default weapon attack on current selected target
        pChar->ReqAttack();
    }

    if (touchSys.ConsumeButtonPress(MobileActionButton::SKILL_1)) {
        // Trigger Quick Slot 1
        pChar->ReqActionQuickSlot(0);
    }
    if (touchSys.ConsumeButtonPress(MobileActionButton::SKILL_2)) {
        // Trigger Quick Slot 2
        pChar->ReqActionQuickSlot(1);
    }
    if (touchSys.ConsumeButtonPress(MobileActionButton::SKILL_3)) {
        // Trigger Quick Slot 3
        pChar->ReqActionQuickSlot(2);
    }
    if (touchSys.ConsumeButtonPress(MobileActionButton::SKILL_4)) {
        // Trigger Quick Slot 4
        pChar->ReqActionQuickSlot(3);
    }

    if (touchSys.ConsumeButtonPress(MobileActionButton::INTERACT_NPC)) {
        // Interact with targeted NPC or nearest NPC
        pChar->ReqNpcDialogue();
    }

    if (touchSys.ConsumeButtonPress(MobileActionButton::POTION_HP)) {
        // Fast potion recovery slot
        pChar->ReqUseQuickSlot(4);
    }
    if (touchSys.ConsumeButtonPress(MobileActionButton::POTION_MP)) {
        pChar->ReqUseQuickSlot(5);
    }

    // 3. Update Camera Orbit from Touch Drag
    UpdateCameraOrbit();
}

void RanMobileInputBridge::UpdateCameraOrbit() {
    MobileTouchSystem& touchSys = MobileTouchSystem::GetInstance();
    const MobileCameraPanState& camPan = touchSys.GetCameraPan();

    if (camPan.active && (camPan.deltaX != 0.0f || camPan.deltaY != 0.0f)) {
        float fSensitivity = 0.005f;
        float dYaw = camPan.deltaX * fSensitivity;
        float dPitch = camPan.deltaY * fSensitivity;

        DxViewPort::GetInstance().CameraAngleAdjustment(dYaw, dPitch);
        touchSys.ResetCameraDeltas();
    }
}
