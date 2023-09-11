#pragma once
#include "pch.h"

namespace KHAS{

    // user input
    struct InputData{
        std::string ip_address;
        int port;
    };

    // error information
    struct ErrorInformation{ 
        int return_value;
        std::string text;
    };

    // implements the server connection interface
    class TCPClient final{
        private:
        std::string ip_address_;
        sockaddr_in addr_;
        int port_;
        int socket_;
        std::optional<ErrorInformation> is_error_;
        
        private:

        // initiates variables
        void init() noexcept;

        public:

        explicit TCPClient(InputData&& input);
        TCPClient() = delete;
        TCPClient(const TCPClient&) = delete;
        TCPClient(TCPClient&& other) noexcept;
        TCPClient& operator = (const TCPClient&) = delete;
        TCPClient& operator = (TCPClient&& other) noexcept;
        ~TCPClient();

        public:
        // starts the client
        void start() noexcept;

    };
}