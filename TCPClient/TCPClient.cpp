#include "TCPClient.h"

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
            is_error_ = ErrorInformation{ .return_value = socket_, .text = "Create socket error!" };
            return;   
        }

        if(auto ret = connect(socket_
        , reinterpret_cast<sockaddr*>(&addr_)
        , sizeof addr_); ret != 0){
            is_error_ = ErrorInformation{ .return_value = ret, .text = "Connection to server FAILED!!!" };
            return; 
        }
        
    }


    void TCPClient::start() noexcept
    {
        std::cout << "----------------------------\n"
            << "client started successfully!\n"
            << "----------------------------\n" << std::left
            << std::setw(25) << "ip_address: " << std::setw(10) << inet_ntoa(addr_.sin_addr) << "\n"
            << std::setw(25) << "port: "  << std::setw(10) << port_ << "\n"
            << std::setw(25) << "client shutdown command: "  << std::setw(10) << "*" << "\n"
            << std::setw(15) << "server shutdown command: " << std::setw(10) << "#" << "\n"
            << "----------------------------" << std::endl;

        if(is_error_.has_value()){
            std::cerr << "text: " << is_error_.value().text << "\n"
             << "return value: " <<  is_error_.value().return_value << std::endl;
            return;
        }

        std::vector<std::string> buf;
        std::string cmd;
        do{
            buf.clear();
            std::cout << "Enter cmd: ";
            std::getline(std::cin, cmd, '\n');
            if(cmd == "*") break;
            send(socket_, cmd.c_str(), cmd.length(), 0);

            std::cout << "waiting for server connections..\n";

            char tmp[256]{};
            ssize_t read_size{};
            do{
                read_size = recv(socket_, tmp, 256, 0);
                buf.emplace_back(tmp);
            } while(read_size == 256);
            
            // if the data is not received, then the server is not working
            if(read_size <= 0) break;
            std::cout << "answer:\n";
            
            for(auto&& elem: buf){
                std::cout << elem << "\n";
            }
        } while(cmd != "*");
        close(socket_);
        
        std::cout << "client closed";
            if(is_error_.has_value()){
                std::cerr << "return value: " <<  is_error_.value().return_value << "\n"
                << "text: " << is_error_.value().text << std::endl;
            return;
        }
    }
}