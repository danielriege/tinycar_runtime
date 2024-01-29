#pragma once

#include <stdint.h>
#include <string>
#include <functional>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#define TCCP_PORT 55002

#define TCCP_TYPE_CONTROL 0x00
#define TCCP_TYPE_TELEMETRY 0x01
#define TCCP_TYPE_STREAM_CONTROL 0x02
#define TCCP_TYPE_RTT 0x03

typedef struct {
    uint8_t type: 2; // 0 = control, 1 = telemetry
    uint8_t padding: 6;
} tccp_header_t;

typedef struct {
    tccp_header_t header;
    uint16_t battery_voltage;
    uint8_t current_fps; 
    uint16_t min_frame_latency;
    int8_t wifi_rssi;
} tccp_telemetry_t;

typedef struct {
    tccp_header_t header;
    uint8_t headlight:2; // 0 = off, 1 = on, 2 = full beam
    uint8_t taillight:2; // 0 = off, 1 = on, 2 = brake
    uint8_t blinker:2; // 0 = off, 1 = left, 2 = right, 3 = hazard
    uint8_t padding:2;
    uint16_t servo_angle; // 0-18000
    int8_t motor_duty_cycle; // 0-255
} tccp_control_t;

typedef struct {
    tccp_header_t header;
    uint8_t packet_loss;
} tccp_stream_control_t;

typedef struct {
    tccp_header_t header;
    uint32_t timestamp; // in ms
} tccp_rtt_t;

class TCCP_Client {
public:
    TCCP_Client(const std::string& hostname);
    void startListener();
    
    int sendControlMessage(tccp_control_t* control_msg_data); 
    int sendRTTMessage();
    void registerTelemetryCallback(std::function<void(tccp_telemetry_t)> callback);
    void registerRTTCallback(std::function<void(uint32_t)> callback);
private:
    int sendData(uint8_t* data, size_t len);
    void listener_task();

    std::string hostname;
    int sockfd;
    std::thread listenerThread;
    std::function<void(tccp_telemetry_t)> telemetryCallback;
    std::function<void(uint32_t)> rttCallback;
};

