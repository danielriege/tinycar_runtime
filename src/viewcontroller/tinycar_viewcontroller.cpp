#include "tinycar_viewcontroller.hpp"

TinycarViewController::TinycarViewController(std::shared_ptr<Tinycar> tinycar): tinycar(tinycar) {
    gamepadAvailable = false;
    tinycar->registerTelemetryCallback(std::bind(&TinycarViewController::tinycarTelemetryCallback, this, std::placeholders::_1));
}

TinycarViewController::~TinycarViewController() {
    // empty
}

void TinycarViewController::show() {
    ImGui::Begin("Tinycar");
    //////// CONTROL
    ImGui::SeparatorText("Control");
    if (gamepadAvailable) {
        showAxis("Forward", &forward, 0.0f, 1.0f);
        showAxis("Reverse", &reverse, 0.0f, 1.0f);
        showAxis("Steering", &steering, -1.0f, 1.0f);
    } else {
        ImGui::Text("Gamepad not connected");
    }

    ImGui::Text("Blinker");
    ImGui::SameLine();
    blinkerLeft = ImGui::ArrowButton("##blinker_left", ImGuiDir_Left);
    ImGui::SameLine();
    blinkerRight = ImGui::ArrowButton("##blinker_right", ImGuiDir_Right);
    ImGui::SameLine();
    blinkerHazard = ImGui::Button("Hazard");
    ImGui::SameLine();
    blinkerOff = ImGui::Button("Off");

    ImGui::Text("Lights");
    ImGui::SameLine();
    lightOn = ImGui::Button("Daylight");
    ImGui::SameLine();
    fullBeam = ImGui::Button("Full Beam");
    ImGui::SameLine();
    lightOff = ImGui::Button("Off");

    //////// TELEMETRY
    ImGui::SeparatorText("Telemetry"); 

    if (tinycar->isAlive()) {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "online");
    } else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "offline");
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::BeginItemTooltip())
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted("Tinycar is consideres offline if no message was received for 1 second. To start receiving telemetry messages, you need to make at least one control change.");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    ImGui::Text("Battery Voltage: %.2f V", lastTelemetry.battery_voltage / 1000.0);
    ImGui::Text("WiFi RSSI: %d dBm", lastTelemetry.wifi_rssi);
    ImGui::Text("FPS: %d", lastTelemetry.current_fps);
    ImGui::Text("Interarrival Jitter: %.2f ms", lastTelemetry.interarrival_jitter);
    ImGui::Text("Packet Loss: %d%%", lastTelemetry.packet_loss_percentage);
    ImGui::Text("Packets per Frame: %d", lastTelemetry.packets_per_frame);
    ImGui::Text("Frame Latency: %d ms", lastTelemetry.frame_latency);
    ImGui::End();
}

void TinycarViewController::readGamepadInput() {
    // Axis values determine if gamepad is connected (since return value only indicates error)
    int res = Gamepad::getForward(&forward);
    res |= Gamepad::getBackward(&reverse);
    res |= Gamepad::getSteering(&steering);
    if (res == 0) {
        gamepadAvailable = true;
    } else {
        gamepadAvailable = false;
    }

    // Buttons
    // Gamepad buttons are true as long as they are pressed. We only want to read them once.
    next = Gamepad::nextPressed() && !next;
    prev = Gamepad::prevPressed() && !prev;
    
    blinkerLeft = Gamepad::blinkerLeftPressed() && !blinkerLeft;
    blinkerRight = Gamepad::blinkerRightPressed() && !blinkerRight;
    blinkerHazard = Gamepad::blinkerHazardPressed() && !blinkerHazard;
    blinkerOff = Gamepad::blinkerOffPressed() && !blinkerOff;
    
    lightOn = Gamepad::lightOnPressed() && !lightOn;
    lightOff = Gamepad::lightOffPressed() && !lightOff;
    fullBeam = Gamepad::fullBeamPressed() && !fullBeam;

}

void TinycarViewController::sendControlMessage() {
    // Forward and reverse are mutually exclusive
    if (forward > 0.01f) {
        tinycar->setMotorDutyCycle(forward * 100);
    } else if (reverse > 0.01f) {
        tinycar->setMotorDutyCycle(-reverse * 100);
    } else {
        tinycar->setMotorDutyCycle(0);
    }
    // gamepad steering [-1, 1] to [6500, 11500]
    tinycar->setServoAngle((steering + 1.0f) * 2500 + 6500);

    if (blinkerLeft) {
        tinycar->setBlinkerLeft();
    } else if (blinkerRight) {
        tinycar->setBlinkerRight();
    } else if (blinkerHazard) {
        tinycar->setBlinkerHazard();
    } else if (blinkerOff) {
        tinycar->setBlinkerOff();
    }

    if (lightOn) {
        tinycar->setHeadlightOn();
        tinycar->setTaillightOn();
    } else if (lightOff) {
        tinycar->setHeadlightOff();
        tinycar->setTaillightOff();
    } else if (fullBeam) {
        tinycar->setHeadlightFullBeam();
    }
}

void TinycarViewController::tinycarTelemetryCallback(TinycarTelemetry telemetry) {
    lastTelemetry = telemetry;
}

/// @brief This shows a slider for the given axis. But can also manipulate the value due to the slider functionality.
/// @param name Name to show for the axis
/// @param value pointer to the axis value
void TinycarViewController::showAxis(const char* name, float* value, float min, float max) {
    char id[16];
    snprintf(id, 16, "##%s", name);
    ImGui::Text("%s", name);
    ImGui::SameLine((9+4)*7);
    ImGui::SliderFloat(id, value, min, max, "%.2f");
}
