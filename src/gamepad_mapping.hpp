#pragma once

#include <GLFW/glfw3.h>

// MAPPING FOR PS4 CONTROLLER ON GLFW
#define STEERING_AXIS 0
#define FORWARD_AXIS 4
#define REVERSE_AXIS 3
#define NEXT_BUTTON 5
#define PREV_BUTTON 4

#define BLINKER_LEFT_BUTTON 17
#define BLINKER_RIGHT_BUTTON 15
#define BLINKER_HAZARD_BUTTON 16
#define BLINKER_OFF_BUTTON 14

#define LIGHT_ON_BUTTON 3
#define LIGHT_OFF_BUTTON 5
#define FULL_BEAM_BUTTON 10

namespace Gamepad {

    static int getAxisValue(int axis, float* value) {
        if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
            int count;
            const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);
            if (count <= 0) {
                return -1;
            }
            *value = axes[axis];
            return 0;
        }
        return -1;
    }

    static int getButtonValue(int button, bool* value) {
        if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
            int count;
            const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
            if (count <= 0) {
                return -1;
            }
            if (buttons[button] == GLFW_PRESS) {
                *value = true;
            }
            else {
                *value = false;
            }
            return 0;
        }
        return -1;
    }

    /// @brief Get the Steering value from the gamepad
    /// @param value Pointer to the value
    /// @return 0 if success, -1 if no gamepad is connected
    inline int getSteering(float* value) {
        return getAxisValue(STEERING_AXIS, value);
    }

    /// @brief Get the Throttle value from the gamepad
    /// @param value Pointer to the value
    /// @return 0 if success, -1 if no gamepad is connected
    inline int getForward(float* value) {
        int res = getAxisValue(FORWARD_AXIS, value);
        *value = (*value+1)/2;
        return res;
    }

    inline int getBackward(float* value) {
        int res = getAxisValue(REVERSE_AXIS, value);
        *value = (*value+1)/2;
        return res;
    }

    inline int nextPressed() {
        bool value;
        if (getButtonValue(NEXT_BUTTON, &value) == 0) {
            return value;
        }
        return -1;
    }

    inline int prevPressed() {
        bool value;
        if (getButtonValue(PREV_BUTTON, &value) == 0) {
            return value;
        }
        return -1;
    }

    inline int blinkerLeftPressed() {
        bool value;
        if (getButtonValue(BLINKER_LEFT_BUTTON, &value) == 0) {
            return value;
        }
        return -1;
    }

    inline int blinkerRightPressed() {
        bool value;
        if (getButtonValue(BLINKER_RIGHT_BUTTON, &value) == 0) {
            return value;
        }
        return -1;
    }

    inline int blinkerHazardPressed() {
        bool value;
        if (getButtonValue(BLINKER_HAZARD_BUTTON, &value) == 0) {
            return value;
        }
        return -1;
    }

    inline int blinkerOffPressed() {
        bool value;
        if (getButtonValue(BLINKER_OFF_BUTTON, &value) == 0) {
            return value;
        }
        return -1;
    }

    inline int lightOnPressed() {
        bool value;
        if (getButtonValue(LIGHT_ON_BUTTON, &value) == 0) {
            return value;
        }
        return -1;
    }

    inline int lightOffPressed() {
        bool value;
        if (getButtonValue(LIGHT_OFF_BUTTON, &value) == 0) {
            return value;
        }
        return -1;
    }

    inline int fullBeamPressed() {
        bool value;
        if (getButtonValue(FULL_BEAM_BUTTON, &value) == 0) {
            return value;
        }
        return -1;
    }
}