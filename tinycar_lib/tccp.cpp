#include "tccp.hpp"

TCCP_Client::TCCP_Client(const std::string& hostname): hostname(hostname) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
}

void TCCP_Client::startListener() {
    listenerThread = std::thread(&TCCP_Client::listener_task, this);
}

int TCCP_Client::sendData(uint8_t* data, size_t len) {
    sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(TCCP_PORT);
    if (inet_pton(AF_INET, hostname.c_str(), &servaddr.sin_addr) <= 0) {
        std::cerr << "\033[1;31m[Tinycar] TCCP Error: inet:pron error for " << hostname << "\033[0m" << std::endl;
        return -1;
    }

    ssize_t sent = sendto(sockfd, data, len, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (sent < 0) {
        return -1;
    }

    return 0;
}

int TCCP_Client::sendControlMessage(tccp_control_t* control_msg_data) {
    return sendData(reinterpret_cast<uint8_t*>(control_msg_data), sizeof(tccp_control_t));
}

int TCCP_Client::sendRTTMessage() {
    tccp_rtt_t rtt = {0};
    rtt.header.type = TCCP_TYPE_RTT;
    return sendData(reinterpret_cast<uint8_t*>(&rtt), sizeof(tccp_rtt_t));
}

void TCCP_Client::registerTelemetryCallback(std::function<void(tccp_telemetry_t)> callback) {
    this->telemetryCallback = callback;
}

void TCCP_Client::registerRTTCallback(std::function<void(uint32_t)> callback) {
    this->rttCallback = callback;
}

void TCCP_Client::listener_task() {
    int sockfdl = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(TCCP_PORT);
    if(bind(sockfdl, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        std::cerr << "\033[1;31m[Tinycar] TCCP Error: bin error %d" << errno << "\033[0m" << std::endl;
        return;
    }

    char buffer[128];

    struct sockaddr_in source_addr;
    socklen_t socklen = sizeof(source_addr);

    while (true) {
        int n = recvfrom(sockfdl, buffer, sizeof(buffer), 0, (struct sockaddr *)&source_addr, &socklen);

        if (n >= sizeof(tccp_header_t)) {
            tccp_header_t* header = reinterpret_cast<tccp_header_t*>(buffer);

            if (header->type == TCCP_TYPE_TELEMETRY && n >= sizeof(tccp_telemetry_t)) {
                tccp_telemetry_t* telemetry = reinterpret_cast<tccp_telemetry_t*>(buffer);

                if (telemetryCallback) {
                    telemetryCallback(*telemetry);
                }
            } else if (header->type == TCCP_TYPE_RTT && n >= sizeof(tccp_rtt_t)) {
                tccp_rtt_t* rtt = reinterpret_cast<tccp_rtt_t*>(buffer);
                if (rttCallback) {
                    rttCallback(rtt->timestamp);
                }
            }
        } else {
            std::cerr << "\033[1;31m[Tinycar] TCCP Error: Received invalid packet" << "\033[0m" << std::endl;
        }
    }

    close(sockfdl);
}

