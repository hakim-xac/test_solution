#include "TCPClient.h"

namespace KHAS{


    TCPClient::TCPClient(InputData&& input)
    : ip_address_{ std::move(input.ip_address) }
    , port_{ std::move(input.port) }
    {
        init();
    }

    TCPClient::TCPClient(TCPClient &&other) noexcept
    : ip_address_{ std::move(other.ip_address_) }
    , addr_{ std::move(other.addr_) }
    , port_{ std::move(other.port_) }
    , socket_{ std::move(other.socket_) }
    , is_error_{ std::move(other.is_error_) }
    {
    }

    TCPClient &TCPClient::operator=(TCPClient &&other) noexcept
    {        
        if(this != &other){
            ip_address_ = std::move(other.ip_address_);
            addr_ = std::move(other.addr_);
            port_ = std::move(other.port_);
            socket_ = std::move(other.socket_);
            is_error_ = std::move(other.is_error_);            
        }
        return *this;
    }

    TCPClient::~TCPClient()
    {
        if(socket_ != -1) close(socket_);
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
            << std::setw(25) << "ip_address: " << inet_ntoa(addr_.sin_addr) << "\n"
            << std::setw(25) << "port: " << port_ << "\n"
            << std::setw(25) << "client shutdown command: *\n"
            << std::setw(15) << "server shutdown command: #\n"
            << "----------------------------" << std::endl;

        if(is_error_.has_value()){
            std::cerr << "text: " << is_error_.value().text << "\n"
             << "return value: " <<  is_error_.value().return_value << std::endl;
            return;
        }

        std::stack<std::string> buf;
        std::string cmd;
        do{
            //buf.clear();
            std::cout << "Enter cmd: ";
            do{
                std::getline(std::cin, cmd, '\n');
                cmd = trim(std::move(cmd));
            } while(cmd.empty());
            
            if(cmd == "*") break;
            send(socket_, cmd.c_str(), cmd.length(), 0);
            if(cmd == "#") break;
            std::cout << "waiting for server connections..\n";

            char tmp[256]{};
            ssize_t read_size{};
            do{
                read_size = recv(socket_, tmp, 256, 0);
                buf.push(tmp);
            } while(read_size == 256);
            
            // if the data is not received, then the server is not working
            if(read_size <= 0) break;
            std::cout << "answer:\n";
            
            while(!buf.empty()){
                std::cout << buf.top() << "\n";
                buf.pop();
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

    std::string TCPClient::trim(std::string&& str) noexcept
    {           
        return rtrim(ltrim(std::move(str)));
    }

    std::string TCPClient::ltrim(std::string &&str) noexcept
    {                
        auto fn{ std::find_if(str.begin(), str.end(), [](char c){ return !std::isspace(c); }) };

        return fn != str.end() ? std::string(fn, str.end()) : str;
    }

    std::string TCPClient::rtrim(std::string &&str) noexcept
    {        
        auto fn{ std::find_if(str.rbegin(), str.rend(), [](char c){ return !std::isspace(c); }) };
        if (fn != str.rend()) str.erase(fn.base(), str.rbegin().base());
        return str;
    }
}