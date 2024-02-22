#pragma once

#include <stdint.h>
#include <cstring>
#include <string>
#include <functional>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <condition_variable>


#define RTP_PORT 4998
#define DGRAM_SIZE 1024
#define DEFAULT_FRAGMENT_COUNT 20

typedef struct {
    uint32_t timestamp;
    uint16_t frame_num;
    uint32_t seq_num: 6;
    uint32_t rtt_start: 1;
    uint32_t marker: 1;
    uint32_t fragment_offset: 24;
    uint8_t fragment_count;
    uint8_t width; // mutiple of 8
    uint8_t height; // mutiple of 8
} tcfp_header_t;

typedef struct {
    uint32_t timestamp;
    uint8_t fragement_count; // total numbers of fragments in frame
    uint8_t fragments_included; // number of fragments included in frame (fragments received)
    uint8_t width; // mutiple of 8
    uint8_t height; // mutiple of 8
    uint16_t frame_num;
    uint8_t start_rtt;
} tcfp_sender_report_t;

class TCFP_Client {
public:
    TCFP_Client();
    ~TCFP_Client();
    void startListener();
    
    void registerFramePacketCallback(std::function<void(uint8_t *data, size_t len, tcfp_sender_report_t)> callback);
private:
    void listener_task();
    void frameComplete_task();

    // for frame assembly
    uint8_t* frameBuffer; // dynamically allocated
    uint32_t frameBufferSize;
    uint32_t frameBufferActualSize; // actual size of frame inside buffer. frameBufferSize >= frameBufferActualSize
    uint16_t current_frame_num;
    uint8_t packets_received;
    tcfp_sender_report_t senderReport;

    std::condition_variable cv;
    std::mutex cv_m;
    bool frameComplete = false;

    int sockfd;
    char buffer[DGRAM_SIZE];
    std::thread listenerThread;
    std::thread frameCompleteThread;
    std::function<void(uint8_t *data, size_t len, tcfp_sender_report_t)> framePacketCallback;
};