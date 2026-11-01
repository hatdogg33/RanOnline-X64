# Android Port Status and Remaining Work

## Scope of this branch

This branch repairs the Android EGL/GLES platform shell. It does not claim to complete the Ran Online engine port.

## Fixes applied

- Removed invalid CMake include paths that pointed inside `RanMobileAndroid` instead of the repository root.
- Kept the Android native target limited to the files that are currently platform-compatible: `RanAndroidMain.cpp` and `MobileTouchSystem.cpp`.
- Excluded `RanMobileInputBridge.cpp` from the Android target because it directly includes DirectX 9 / Windows game-engine headers and calls engine APIs that have not been ported to Android.
- Moved EGL context binding out of the UI-thread surface callback and into the render thread. EGL contexts must be current on the thread issuing GLES commands.
- Added EGL initialization/configuration failure checks, swap error logging, and native-window release.

## Known unfinished work (not fixed by this scaffold patch)

1. **Actual world renderer:** the current frame still clears the GLES framebuffer. No Ran Online terrain, models, characters, effects, UI, or scene are submitted.
2. **DirectX 9 replacement:** the game engine still uses D3D9/D3DX types and interfaces. An Android renderer/backend or a substantial rendering abstraction is required.
3. **Game-loop integration:** the original client stage, resource lifecycle, scene update, animation, and render pipeline are not connected to the Android Activity.
4. **Touch-to-game integration:** `RanMobileInputBridge.cpp` is a prototype with direct Windows/DirectX dependencies. Its character movement, camera, attacks, quick slots, and NPC calls must be adapted to real engine APIs and integrated into the game loop.
5. **Touch UI rendering:** touch hit regions exist in native code, but visible joystick and action-button graphics are not drawn.
6. **Asset loading:** Android-compatible loading for the actual client data (maps, terrain, meshes, skeletons, textures, effects, and audio) has not been demonstrated.
7. **Networking/session lifecycle:** login, character selection, server connection, reconnect, and session shutdown have not been validated on Android.
8. **Thread safety and lifecycle:** touch input and game state need a defined ownership/queueing model before the game loop is connected. Surface recreation, pause/resume, and context loss need device testing.
9. **Build verification:** a successful CI build and on-device smoke test are still required for this branch. Do not treat a generated APK as proof that the game engine renders.

## Required validation

- Run the GitHub Actions workflow for this branch.
- Confirm logs show EGL initialization and render-thread context binding without EGL errors.
- Install the resulting APK and verify repeated surface create/destroy and pause/resume.
- Only after the platform shell is stable should the DirectX replacement and game-loop integration proceed.

## Build status

Not yet verified in this branch. No claim of a working Ran Online Android client is made.
