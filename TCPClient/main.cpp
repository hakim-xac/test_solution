#include "TCPClient.h"
#include <string>
#include <charconv>


int main(int argc, char* argv[]){

    int port{ 3333 };
    std::string ip_address{"127.0.0.1"};

    if(argc > 1){
        std::string tmp{ argv[1] };
        int ret{};
        auto [ptr, ec]{ std::from_chars(tmp.data(), tmp.data() + tmp.size(), ret) }; 
        if(ec == std::errc()) port = ret;
    }
    if(argc > 2){
        ip_address = std::string{ argv[2] };        
    }
    
    KHAS::TCPClient client{ KHAS::InputData{ .ip_address = std::move(ip_address), .port = port } };
    client.start();

    
    return 0;
}
