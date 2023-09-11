#pragma once
#include "pch.h"

namespace KHAS{

    // user input
    struct InputData{
        int port;
        int max_timeout;
        int max_connection;
    };

    // error information
    struct ErrorInformation{ 
        int return_value;
        std::string text;
    };

    // implements the server interface
    class TCPServer final{
        private:
            int port_;
            int max_connection_;
            int max_timeout_;
            int socket_;
            sockaddr_in addr_;
            std::optional<ErrorInformation> is_error_;
            std::set<int> clients_;
        private:
            // initiates variables
            void init() noexcept;

            // incoming connection processed
            // returns an error, 
            // if there was one and if there was not, then std::nullopt            
            std::optional<int> isGetRequest(const fd_set& readset) noexcept;

            // sprocesses data that came from the client
            bool isSendData(const fd_set& readset) noexcept;

            // send data to the user
            void outputData(int sock_client, std::string&& cmd_str) noexcept;

            // removes whitespace characters from the beginning and end of a string
            std::string trim(std::string&& str) noexcept;
            
            // removes whitespace characters from the beginning of a string
            std::string ltrim(std::string&& str) noexcept;

            // removes whitespace characters from the end of a string
            std::string rtrim(std::string&& str) noexcept;
        public:

        explicit TCPServer(InputData&& input);          
        TCPServer() = delete;
        TCPServer(const TCPServer&) = delete;
        TCPServer(TCPServer&& other) noexcept;
        TCPServer& operator = (const TCPServer&) = delete;
        TCPServer& operator = (TCPServer&& other) noexcept;
        ~TCPServer();

        public:
        
        // starts the client
        void start() noexcept;

    };

}