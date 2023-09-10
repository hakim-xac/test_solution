#pragma once
#include "phc.h"

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

        // starts the client
        void start() noexcept;

    };
}