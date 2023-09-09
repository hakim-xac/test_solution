#include "TCPClient.h"
#include <algorithm>
namespace KHAS{


    TCPClient::TCPClient(InputData&& input)
    : ip_address_{ std::move(input.ip_address) }
    , port_{ std::move(input.port) }
    {
        init();
    }
    

    void TCPClient::init() noexcept
    {
        addr_.sin_port = htons(port_);
        addr_.sin_family = AF_INET;
        inet_pton(AF_INET, ip_address_.c_str()
        , &addr_.sin_addr);
        
        
        socket_ = socket(AF_INET, SOCK_STREAM, 0);  
        if(socket_ == -1)    {
            is_error_ = OutputData{ .return_value = socket_, .text = "Create socket error!" };
            return;   
        }

        if(auto ret = connect(socket_
        , reinterpret_cast<sockaddr*>(&addr_)
        , sizeof addr_); ret != 0){
            is_error_ = OutputData{ .return_value = ret, .text = "Connection to server FAILED!!!" };
            return; 
        }
        
    }


    void TCPClient::start() noexcept
    {
        std::cout << "client started successfully!\n" 
        << "ip_address: " << inet_ntoa(addr_.sin_addr) << "\n"
        << "port: " << port_ << std::endl;

        if(is_error_.has_value()){
            std::cerr << "return value: " <<  is_error_.value().return_value << "\n"
                << "text: " << is_error_.value().text << std::endl;
            return;
        }

        std::vector<std::string> buf;
        std::string cmd;
        do{

            buf.clear();
            std::cout << "Enter cmd: ";
            std::getline(std::cin, cmd, '\n');
            send(socket_, cmd.c_str(), cmd.length(), 0);
            if(cmd == "#") break;

            std::cout << "waiting for server connections..\n";

            char tmp[256]{};
            ssize_t read_size{};
            do{
                read_size = recv(socket_, tmp, 256, 0);
                buf.emplace_back(tmp);
            } while(read_size == 256);
            
            if(read_size <= 0) break;
            std::cout << "answer:\n";
            for(auto&& elem: buf){
                std::cout << elem << "\n";
            }
        } while(cmd != "#");
        close(socket_);
        
        std::cout << "client closed";
            if(is_error_.has_value()){
                std::cerr << "return value: " <<  is_error_.value().return_value << "\n"
                << "text: " << is_error_.value().text << std::endl;
            return;
        }
    }

    void TCPClient::start2() noexcept
    {

    int client{  socket(AF_INET, SOCK_STREAM, 0) };
    if(client < 0){
        std::cout << "Create socket error!" << std::endl;
        exit(0);
    }

    std::cout << "Client: Socked was success created" << std::endl;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    inet_pton(AF_INET, ip_address_.c_str()
    , &addr.sin_addr);

    std::cout << "\nClient socket created.\n";

    if(!connect(client
    , reinterpret_cast<sockaddr*>(&addr)
    , sizeof addr) == 0){
        std::cout << "Connection to server FAILED!!!" << std::endl;
        exit(1);
    }
    std::cout << "Connection to server "
        << inet_ntoa(addr.sin_addr) << "\n"
        << "port: " << 1500 << "\n\n";
    std::vector<std::string> buf;
    std::string cmd;
    do{

        buf.clear();
        std::cout << "Enter cmd: ";
        std::getline(std::cin, cmd, '\n');
        send(client, cmd.c_str(), cmd.length(), 0);
        if(cmd == "#") break;

        char buffer[256]{};
        std::cout << "waiting for server connections..\n";

        char tmp[256]{};
        ssize_t read_size{};
        do{
            read_size = recv(client, tmp, 256, 0);
            buf.emplace_back(tmp);
        } while(read_size == 256);
        
        if(read_size <= 0) break;
        std::cout << "answer:\n";
        for(auto&& elem: buf){
            std::cout << elem << "\n";
        }
    } while(cmd != "#");
    close(client);
    }
}