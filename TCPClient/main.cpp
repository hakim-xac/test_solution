#include "TCPClient.h"
#include "pch.h"

// converting from type From to type To
template <typename To, typename From>
    requires std::is_convertible_v<std::remove_reference_t<From>, std::string>
    && (!std::is_same_v<To, std::string>)
inline std::optional<To> toType(From&& value){
    std::string str{ std::forward<From>(value) };
    To ret{};
    auto [ptr, ec]{ std::from_chars(str.data(), str.data() + str.size(), ret) }; 
    if(ec == std::errc()) return ret;
    return std::nullopt;
}

int main(int argc, char* argv[]){

    int port{ 3333 };
    std::string ip_address{"127.0.0.1"};

    if(argc > 1){
        auto ret{ toType<int>(argv[1]) };
        if(ret.has_value()) port = ret.value(); 
    }
    if(argc > 2){
        ip_address = std::string(argv[2]); 
    }
    
    KHAS::TCPClient client{ KHAS::InputData{ .ip_address = std::move(ip_address), .port = port } };
    client.start();

    
    return 0;
}
