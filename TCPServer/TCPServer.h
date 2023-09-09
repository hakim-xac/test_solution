#pragma once
#include <string>
#include <optional>
#include <set>
// #include <sys/types.h>
// #include <sys/time.h>
// #include <sys/socket.h>
#include <netinet/in.h>
// #include <arpa/inet.h>
// #include <fcntl.h>

namespace KHAS{

    struct InputData{
        int port;
        int max_connection;
    };

    struct OutputData{ 
        int return_value;
        std::string text;
    };

    class TCPServer final{
        private:
            int port_;
            int max_connection_;
            int socket_;
            sockaddr_in addr_;
            std::optional<OutputData> is_error_;
            std::set<int> clients_;
        private:
            void init() noexcept;
            std::optional<int> isGetRequest(const fd_set& readset) noexcept;
            bool sendData(const fd_set& readset) noexcept;
            void outputData(int sock_client, std::string&& cmd_str) noexcept;
        public:

        explicit TCPServer(InputData&& input);  
        void start() noexcept;

        void start2() noexcept;

    };

}