#include "TCPServer.h"
#include <string>
#include <charconv>

int main(int argc, char* argv[]){

    int port{ 3333 };
    int max_connection{ 10 };

    if(argc > 1){
        std::string tmp{ argv[1] };
        int ret{};
        auto [ptr, ec]{ std::from_chars(tmp.data(), tmp.data() + tmp.size(), ret) }; 
        if(ec == std::errc()) port = ret;
    }
    if(argc > 2){
        std::string tmp{ argv[2] };
        int ret{};
        auto [ptr, ec]{ std::from_chars(tmp.data(), tmp.data() + tmp.size(), ret) }; 
        if(ec == std::errc()) max_connection = ret;
    }

    KHAS::TCPServer server{ KHAS::InputData{ .port = port, .max_connection = max_connection } };
    server.start();

    return 0;
}
