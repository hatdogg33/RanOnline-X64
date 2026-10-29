#pragma once

#include "MobileTouchSystem.h"

// Forward declare Ran Online types
class GLCharacter;

class RanMobileInputBridge {
public:
    static RanMobileInputBridge& GetInstance() {
        static RanMobileInputBridge instance;
        return instance;
    }

    // Called once per frame in the main game loop (DxGlobalStage / BasicWnd frame move)
    void UpdateMovementAndActions(float fElapsedTime);

    // Camera yaw and pitch adjustments based on touch drag
    void UpdateCameraOrbit();

private:
    RanMobileInputBridge() = default;
};
