#include "TCPServer.h"
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
    int max_connection{ 10 };
    int max_timeout{ 30 }; // sec
    if(argc > 1){
        auto ret{ toType<int>(argv[1]) };
        if(ret.has_value()) port = ret.value();        
    }
    if(argc > 2){
        auto ret{ toType<int>(argv[2]) };
        if(ret.has_value()) max_timeout = ret.value(); 
    }
    if(argc > 3){
        auto ret{ toType<int>(argv[3]) };
        if(ret.has_value()) max_connection = ret.value(); 
    }

    KHAS::TCPServer server{ KHAS::InputData{ 
        .port = port
        , .max_timeout = max_timeout
        , .max_connection = max_connection } };
    server.start();

    return 0;
}
