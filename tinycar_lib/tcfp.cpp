#include "tcfp.hpp"

TCFP_Client::TCFP_Client() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    frameBufferSize = DEFAULT_FRAGMENT_COUNT * DGRAM_SIZE;
    frameBuffer = (uint8_t*)malloc(frameBufferSize);
    senderReport = {0};
}

TCFP_Client::~TCFP_Client() {
    close(sockfd);
    free(frameBuffer);
}

void TCFP_Client::startListener() {
    listenerThread = std::thread(&TCFP_Client::listener_task, this);
    frameCompleteThread = std::thread(&TCFP_Client::frameComplete_task, this);
}

void TCFP_Client::registerFramePacketCallback(std::function<void( uint8_t *data, size_t len, tcfp_sender_report_t)> callback) {
    framePacketCallback = callback;
}

void TCFP_Client::frameComplete_task() {
    std::unique_lock<std::mutex> lk(cv_m);
    while (true) {
        cv.wait(lk, [this]{ return frameComplete; });
        // do stuff with frame completion
        if (framePacketCallback) {
            framePacketCallback(frameBuffer, frameBufferActualSize, senderReport);
        }
        senderReport = {0};
        packets_received = 0;
        frameBufferActualSize = 0;
        frameComplete = false;
    }
}

void TCFP_Client::listener_task() {
    int sockfdl = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(RTP_PORT);
    if(bind(sockfdl, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        std::cerr << "\033[1;31m[Tinycar] TCFP Error: bind error %d" << errno << "\033[0m" << std::endl;
        return;
    }

    struct sockaddr_in source_addr;
    socklen_t socklen = sizeof(source_addr);

    std::cout << "\033[1;32m[Tinycar] TCFP Info: Listener started." << "\033[0m" << std::endl;

    while (true) {
        int n = recvfrom(sockfdl, buffer, sizeof(buffer), 0, (struct sockaddr *)&source_addr, &socklen);
        if (n < 0) {
            std::cerr << "\033[1;31m[Tinycar] TCFP Error: recvfrom error %d" << errno << "\033[0m" << std::endl;
            return;
        }
        if (n >= sizeof(tcfp_header_t)) {
            tcfp_header_t* header = reinterpret_cast<tcfp_header_t*>(buffer);
            // Allocate more memory if needed
            if (frameBufferSize < header->fragment_count * DGRAM_SIZE) {
                frameBufferSize = header->fragment_count * DGRAM_SIZE;
                frameBuffer = (uint8_t*)realloc(frameBuffer, frameBufferSize);
            }
            if (current_frame_num == 0) {
                current_frame_num = header->frame_num;
            }
            // check if frame number matches with expected frame number
            if (header->frame_num != current_frame_num) {
                std::cerr << "\033[1;33m[Tinycar] TCFP Warning: frame number mismatch. Expected " << current_frame_num << " but got " << header->frame_num << "\033[0m" << std::endl;
                // reset frame number
                current_frame_num = header->frame_num;
                // reset frame buffer
                packets_received = 0;
                frameBufferActualSize = 0;
            }
            // Copy data to frame buffer
            memcpy(frameBuffer + header->fragment_offset, buffer + sizeof(tcfp_header_t), n - sizeof(tcfp_header_t));
            packets_received++;
            // Check if frame is complete
            if (header->marker) {
                // increase frame number to prepare for next frame
                current_frame_num = (header->frame_num + 1) % 65536;

                frameBufferActualSize = header->fragment_offset + n - sizeof(tcfp_header_t);
                senderReport.timestamp = header->timestamp;
                senderReport.fragement_count = header->fragment_count;
                senderReport.width = header->width;
                senderReport.height = header->height;
                senderReport.frame_num = header->frame_num;
                senderReport.fragments_included = packets_received;
                senderReport.start_rtt = header->rtt_start;
                // do stuff with frame completion on different thread
                {
                    std::lock_guard<std::mutex> lk(cv_m);
                    frameComplete = true;
                }
                cv.notify_one();
            }

        } else {
            std::cerr << "\033[1;31m[Tinycar] TCFP Error: packet too small for JPEG stream. Will be ignored. \033[0m" << std::endl;
        }
    }

    close(sockfdl);
}