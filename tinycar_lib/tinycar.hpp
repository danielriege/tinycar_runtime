#pragma once

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <chrono>
#include <functional>

#include "tccp.hpp"
#include "tcfp.hpp"

#define ANTISPAM_DELAY 20 // ms; Hardware is not faster anyway
#define ALIVE_TIMEOUT 2300 // ms; Usually telemetry is sent every 2 seconds

#define MAX_NUM_PACKETS_PER_FRAME 124

typedef struct {
    uint16_t battery_voltage;
    uint8_t current_fps; 
    double interarrival_jitter; 
    int8_t wifi_rssi;
    uint8_t packet_loss_percentage;
    uint8_t packets_per_frame;
    uint32_t frame_latency;
} TinycarTelemetry;

class Tinycar {
public:
    Tinycar(const std::string& hostname);
    // Getter
    int getImage(cv::Mat& out);
    
    void setMotorDutyCycle(int16_t dutyCycle);
    void setServoAngle(uint16_t angle);

    void setBlinkerLeft();
    void setBlinkerRight();
    void setBlinkerHazard();
    void setBlinkerOff();

    void setHeadlightOff();
    void setHeadlightOn();
    void setHeadlightFullBeam();

    void setTaillightOff();
    void setTaillightOn();
    void setTaillightBrake();

    void registerTelemetryCallback(std::function<void(TinycarTelemetry)> callback);
    bool isAlive();
private:
    /// @brief Sends the last control message to the car. However, it checks the time since the last message to avoid spamming the network.
    void sendControlMessage();

    void tcfpFramePacketCallback(uint8_t *data, size_t len, tcfp_sender_report_t senderReport);
    void tccpRTTCallback(uint32_t timestamp);

    // for tcfp analysis
    uint32_t last_sender_timestamp = 0;
    std::chrono::system_clock::time_point last_arrival_time;
    double jitter = 0.0;
    std::chrono::system_clock::time_point last_packet_loss_calculation;
    uint32_t total_expected_packets = 0;
    uint32_t total_received_packets = 0;
    uint8_t packet_loss_percentage;
    uint8_t packets_per_frame;
    uint8_t frame_latency;
    uint32_t last_rtt_frame_timestamp = 0;
    std::chrono::system_clock::time_point last_rtt_start_time;


    TCFP_Client tcfp_client;
    TCCP_Client tccp_client;
    std::string hostname;
    bool telemetryListenerRunning;

    bool frameMatPulled;
    cv::Mat frameMat;

    // Keeping the state since tccp is stateless
    tccp_control_t last_control_message; 
    // time of last message
    std::chrono::time_point<std::chrono::system_clock> last_message_time;
    std::chrono::time_point<std::chrono::system_clock> last_telemetry_time;
};