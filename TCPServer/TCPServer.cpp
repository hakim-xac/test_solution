#include "TCPServer.h"
#include "pch.h"


namespace KHAS{

    TCPServer::TCPServer(InputData&& input)
    : port_{ std::move(input.port) }
    , max_timeout_{ std::move(input.max_timeout) }
    , max_connection_{ std::move(input.max_connection) }
    , socket_{ -1 }
    , addr_{}
    , is_error_{ std::nullopt }
    {
        init();
    }
    
    TCPServer::~TCPServer(){
        if(socket_ != -1) close(socket_);
    }


    TCPServer::TCPServer(TCPServer&& other) noexcept
    : port_{std::move(other.port_) }
    , max_connection_{ std::move(other.max_connection_) }
    , max_timeout_{ std::move(other.max_timeout_) }
    , socket_{ std::move(other.socket_) }
    , addr_{ std::move(other.addr_) }
    , is_error_{ std::move(other.is_error_) }
    , clients_{ std::move(other.clients_) }
    {

    }

    TCPServer& TCPServer::operator=(TCPServer&& other) noexcept
    {
        if(this != &other){
            port_ = std::move(other.port_);
            max_connection_ = std::move(other.max_connection_);
            max_timeout_ = std::move(other.max_timeout_);
            socket_ = std::move(other.socket_);
            addr_ = std::move(other.addr_);
            is_error_ = std::move(other.is_error_);
            clients_ = std::move(other.clients_);
        }
        return *this;        
    }

    void TCPServer::init() noexcept
    {
        addr_.sin_port = htons(port_);
        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = htons(INADDR_ANY);
        
        socket_ = socket(AF_INET, SOCK_STREAM, 0);  
        if(socket_ == -1)    {
            is_error_ = ErrorInformation{ .return_value = socket_, .text = "Create socket error!" };
            return;   
        }

        if(int ret = bind(socket_, reinterpret_cast<sockaddr*>(&addr_), sizeof addr_)
            ; ret < 0){                
            is_error_ = ErrorInformation{ .return_value = ret, .text = "Bind failed!" };
            return; 
        }
        if(int ret = listen(socket_, max_connection_)
            ; ret < 0){                
            is_error_ = ErrorInformation{ .return_value = ret, .text = "Listen failed!" };
            return; 
        }
    }

    void TCPServer::start() noexcept
    {
        if(is_error_.has_value()){
            std::cerr << "return value: " <<  is_error_.value().return_value << "\n"
                << "text: " << is_error_.value().text << std::endl;
            return;
        }
        std::cout << "----------------------------\n"
        << "server started successfully!\n" 
        << "----------------------------\n" << std::left
        << std::setw(25) << "ip_address: " << inet_ntoa(addr_.sin_addr) << " \n"
        << std::setw(25) << "port: " << port_ << "\n"
        << std::setw(25) << "max_timeout: " << max_timeout_ << "sec.\n"
        << std::setw(25) << "max_connection: " << max_connection_ << "\n"
        << "----------------------------" << std::endl;

        bool running{ true };
        while(running){

            fd_set readset;
            FD_ZERO(&readset);
            FD_SET(socket_, &readset);

            for(auto&& sock_client: clients_) FD_SET(sock_client, &readset);            

            timeval timeout{ .tv_sec = max_timeout_, .tv_usec = 0 };
            
            auto iter_max_elem{ std::max_element(clients_.begin(), clients_.end()) };
            
            int mx{ iter_max_elem == clients_.end() ? socket_ : std::max(socket_, *iter_max_elem) };
            
            if(auto ret = select(mx+1, &readset, NULL, NULL, &timeout); ret <= 0)
            {
                is_error_ = ErrorInformation{ .return_value = ret, .text = "Select failed! or timeout limited!" };
                running = false;
                break;
            }
            
            if(auto ret = isGetRequest(readset); ret.has_value()){
               
                is_error_ = ret.value() == -1
                ? ErrorInformation{ .return_value = ret.value(), .text = "FD_ISSET failed!" }
                : ErrorInformation{ .return_value = ret.value(), .text = "Accept failed!" };

                running = false;
                break;
            }

            
            if(!sendData(readset)){
                running = false;
            }
            
            if(!running){
        
                for(auto&& sock_client: clients_)
                {
                    if(FD_ISSET(sock_client, &readset))
                    {
                        std::string ss{"server closed!\n"};
                        send(sock_client, ss.c_str(), ss.length(), 0);
                        send(sock_client, "*", 1, 0);
                        close(sock_client);
                    }
                }
                clients_.clear();                
            }           
        }
        std::cout << "Server disconected!" << std::endl;
        
        if(is_error_.has_value()){
            std::cerr 
                << "text: " << is_error_.value().text << "\n"
                << "return value: " <<  is_error_.value().return_value << std::endl;
            return;
        }
    }


    std::optional<int> TCPServer::isGetRequest(const fd_set& readset) noexcept
    {

        if(FD_ISSET(socket_, &readset)){      
            // if a client is connected, then we process it
            int sock = accept(socket_, NULL, NULL);
            if(sock < 0) return sock;
            fcntl(sock, F_SETFL, O_NONBLOCK);
            clients_.insert(sock);
        }
        return std::nullopt;
    }


    bool TCPServer::sendData(const fd_set& readset) noexcept
    {
        if(clients_.size() == 0) return false;

        std::stack<int> stack_to_delete;

        for(auto&& sock_client: clients_)
        {
            if(FD_ISSET(sock_client, &readset))
            {
                // if data has arrived, we process it
                constexpr const int MAX_BUFFER_SIZE{ 256 };
                std::string cmd_str;                
                char tmp[MAX_BUFFER_SIZE]{};
                ssize_t bytes_read{};
                do{
                    bytes_read = recv(sock_client, tmp, MAX_BUFFER_SIZE, 0);
                    cmd_str += tmp;
                } while(bytes_read == MAX_BUFFER_SIZE);

                if(bytes_read <= 0)
                {
                    stack_to_delete.push(sock_client);

                }
                // turn off the server
                if(tmp[0] == '#') 
                {
                    return false;
                }

                // send data to the client
                outputData(sock_client, std::move(cmd_str));
            }
        }
        while(!stack_to_delete.empty()){

            // if there is no data, then the client is no longer needed and can be disabled
            auto sock_id{ stack_to_delete.top() };
            stack_to_delete.pop();
            close(sock_id);
            clients_.erase(sock_id);
        }
        return true;
    }


    void TCPServer::outputData(int sock_client, std::string&& cmd_str) noexcept
    {
        std::cout << std::left
            << std::setw(10) << "client_id: " << std::setw(20) << sock_client << " \n"
            << std::setw(10) << "input cmd: " << std::setw(20) << cmd_str << " \n"            
            << std::endl;

        cmd_str += " > ./tmpfile.txt";
        
        std::cout << std::setw(10) << "return system: \n"
            << std::system(cmd_str.c_str()) << std::endl;
            


        std::stringstream ss{"+"};
        ss << std::ifstream("./tmpfile.txt").rdbuf();
        auto _ { std::system("rm -f tmpfile.txt") };

        send(sock_client, ss.str().c_str(), ss.str().length(), 0);
    }

}