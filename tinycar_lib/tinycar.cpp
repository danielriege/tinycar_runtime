#include "tinycar.hpp"

Tinycar::Tinycar(const std::string& hostname): tccp_client(hostname), hostname(hostname), telemetryListenerRunning(false), tcfp_client() {
    last_control_message = {0};
    last_control_message.header.type = TCCP_TYPE_CONTROL;
    frameMatPulled = true;

    tcfp_client.registerFramePacketCallback(std::bind(&Tinycar::tcfpFramePacketCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    tccp_client.registerRTTCallback(std::bind(&Tinycar::tccpRTTCallback, this, std::placeholders::_1));
    tcfp_client.startListener();
}

int Tinycar::getImage(cv::Mat& out) {
    if (!frameMatPulled) {
        out = frameMat;
        frameMatPulled = true;
        return true;
    }
    return false;
}

double Tinycar::getFPS() {
    return current_fps;
}

////// CONTROL FUNCTIONS

void Tinycar::setMotorDutyCycle(int16_t dutyCycle) {
    last_control_message.motor_duty_cycle = dutyCycle;
    sendControlMessage();
}

void Tinycar::setServoAngle(uint16_t angle) {
    last_control_message.servo_angle = angle;
    sendControlMessage();
}

void Tinycar::setBlinkerLeft() {
    last_control_message.blinker = 1;
    sendControlMessage();
}

void Tinycar::setBlinkerRight() {
    last_control_message.blinker = 2;
    sendControlMessage();
}

void Tinycar::setBlinkerHazard() {
    last_control_message.blinker = 3;
    sendControlMessage();
}

void Tinycar::setBlinkerOff() {
    last_control_message.blinker = 0;
    sendControlMessage();
}

void Tinycar::setHeadlightOff() {
    last_control_message.headlight = 0;
    sendControlMessage();
}

void Tinycar::setHeadlightOn() {
    last_control_message.headlight = 1;
    sendControlMessage();
}

void Tinycar::setHeadlightFullBeam() {
    last_control_message.headlight = 2;
    sendControlMessage();
}

void Tinycar::setTaillightOff() {
    last_control_message.taillight = 0;
    sendControlMessage();
}

void Tinycar::setTaillightOn() {
    last_control_message.taillight = 1;
    sendControlMessage();
}

void Tinycar::setTaillightBrake() {
    last_control_message.taillight = 2;
    sendControlMessage();
}

bool Tinycar::isAlive() {
    // check alive status by checking if last message was sent less than 1 second ago
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_telemetry_time);
    return diff.count() < ALIVE_TIMEOUT;
}

void Tinycar::registerTelemetryCallback(std::function<void(TinycarTelemetry)> callback) {
    tccp_client.registerTelemetryCallback([=](tccp_telemetry_t telemetry) {
        // setting internal state
        current_fps = telemetry.current_fps;
        // prepare telemetry message
        last_telemetry_time = std::chrono::system_clock::now();
        TinycarTelemetry tinycarTelemetry;
        tinycarTelemetry.battery_voltage = telemetry.battery_voltage;
        tinycarTelemetry.current_fps = telemetry.current_fps;
        tinycarTelemetry.interarrival_jitter = this->jitter;
        tinycarTelemetry.packet_loss_percentage = this->packet_loss_percentage;
        tinycarTelemetry.packets_per_frame = this->packets_per_frame;
        tinycarTelemetry.wifi_rssi = telemetry.wifi_rssi;
        tinycarTelemetry.frame_latency = this->frame_latency + telemetry.min_frame_latency;
        callback(tinycarTelemetry);
    });
}

void Tinycar::sendControlMessage() {
    // check if listener is running
    if (!telemetryListenerRunning) {
        tccp_client.startListener();
        telemetryListenerRunning = true;
    }
    // Check if we are spamming
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_message_time);
    if (diff.count() < ANTISPAM_DELAY) {
        return;
    }
    tccp_client.sendControlMessage(&last_control_message);
    last_message_time = std::chrono::system_clock::now();
}

void Tinycar::tccpRTTCallback(uint32_t timestamp) {
    uint32_t diff_frame_rtt = timestamp - last_rtt_frame_timestamp; // difference between frame sending and receiving our rtt message
    auto now = std::chrono::system_clock::now();
    uint32_t diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_rtt_start_time).count();
    uint32_t latency_for_rtt_message = diff/2; // we assume that the rtt message latency is the same in both directions
    frame_latency = diff_frame_rtt - latency_for_rtt_message;
}

void Tinycar::tcfpFramePacketCallback(uint8_t *data, size_t len, tcfp_sender_report_t senderReport) {
    // do some analysis on the sender report to send new control messages for frame rate control
    // calculate jitter
    if (last_sender_timestamp != 0 && last_arrival_time.time_since_epoch().count() != 0) {
        auto arrival_time = std::chrono::system_clock::now();
        double D = std::chrono::duration_cast<std::chrono::milliseconds>(arrival_time - last_arrival_time).count()  - (senderReport.timestamp - last_sender_timestamp);
        jitter += (std::abs(D) - jitter) / 16.0;
    }

    last_sender_timestamp = senderReport.timestamp;
    last_arrival_time = std::chrono::system_clock::now();

    // for packet loss calculation
    if (last_packet_loss_calculation.time_since_epoch().count() == 0) {
        last_packet_loss_calculation = std::chrono::system_clock::now();
    }
    total_expected_packets += senderReport.fragement_count;
    total_received_packets += senderReport.fragments_included;
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_packet_loss_calculation);
    // since telemetry is updated every 2 s, we measure packet loss for 2s
    if (diff.count() > 2000) {
        packet_loss_percentage = 100.0 * (1.0 - (double)total_received_packets / (double)total_expected_packets);
        last_packet_loss_calculation = std::chrono::system_clock::now();
        total_expected_packets = 0;
        total_received_packets = 0;
    }


    packets_per_frame = senderReport.fragement_count;
    // send rtt message
    if (senderReport.start_rtt) {
        last_rtt_frame_timestamp = senderReport.timestamp;
        last_rtt_start_time = std::chrono::system_clock::now();
        tccp_client.sendRTTMessage();
    }

    // decode image
    if (senderReport.fragments_included == senderReport.fragement_count && len > 0) {
        frameMat = cv::imdecode(cv::Mat(len, 1, CV_8UC1, data), cv::IMREAD_COLOR);
        cv::flip(frameMat, frameMat, -1);
        frameMatPulled = false;
    } else {
        printf("Did not receive all fragments to decode image\n");
    }
}
