#pragma once

#include "nv.hpp" // also includes imgui
#include "logger.hpp"
#include "gamepad_mapping.hpp"
#include "tinycar.hpp"

class TinycarViewController {
public:
    TinycarViewController(std::shared_ptr<Tinycar> tinycar);
    ~TinycarViewController();

    /// @brief Shows the frame for the window
    void show();

    /// @brief Read the input values from the gamepad
    void readGamepadInput();

    void sendControlMessage();
private:
    void showAxis(const char* name, float* value, float min, float max);
    void tinycarTelemetryCallback(TinycarTelemetry telemetry);

    // Input values
    float forward, reverse, steering;

    // light control values
    bool blinkerLeft, blinkerRight, blinkerHazard,blinkerOff,lightOn, lightOff, fullBeam, next, prev;

    bool gamepadAvailable;
    std::shared_ptr<Tinycar> tinycar;
    TinycarTelemetry lastTelemetry;


};

