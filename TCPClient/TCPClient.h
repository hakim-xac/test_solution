#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <optional>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>

namespace KHAS{

    struct InputData{
        std::string ip_address;
        int port;
    };

    struct OutputData{ 
        int return_value;
        std::string text;
    };

    class TCPClient final{
        private:
        std::string ip_address_;
        sockaddr_in addr_;
        int port_;
        int socket_;
        std::optional<OutputData> is_error_;
        
        private:
        void init() noexcept;

        public:
        explicit TCPClient(InputData&& input);
        void start() noexcept;
        void start2() noexcept;


    };
}