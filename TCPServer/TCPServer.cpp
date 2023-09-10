#include "TCPServer.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>


namespace KHAS{

    TCPServer::TCPServer(InputData&& input)
    : port_{ std::move(input.port) }
    , max_connection_{ std::move(input.max_connection) }
    , socket_{ -1 }
    , addr_{}
    , is_error_{ std::nullopt }
    {
        init();
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
        if(int ret = listen(socket_, 5)
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
        std::cout << "server started successfully!\n" 
        << "ip_address: " << inet_ntoa(addr_.sin_addr) << " \n"
        << "port: " << port_ << "\n"
        << "max_connection: " << max_connection_ << std::endl;

        bool running{ true };
        while(running){

            fd_set readset;
            FD_ZERO(&readset);
            FD_SET(socket_, &readset);

            for(auto&& sock_client: clients_) FD_SET(sock_client, &readset);            

            timeval timeout{ .tv_sec = 150, .tv_usec = 0 };
            
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
                        close(sock_client);
                    }
                }
                clients_.clear();                
            }           
        }
        std::cout << "Server disconected!" << std::endl;
        close(socket_);

        if(is_error_.has_value()){
            std::cerr << "return value: " <<  is_error_.value().return_value << "\n"
                << "text: " << is_error_.value().text << std::endl;
            return;
        }
    }


    std::optional<int> TCPServer::isGetRequest(const fd_set& readset) noexcept
    {

        if(FD_ISSET(socket_, &readset)){      
            // Поступил новый запрос на соединение, используем accept
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

        for(auto&& sock_client: clients_)
        {
            if(FD_ISSET(sock_client, &readset))
            {
                // Поступили данные от клиента, читаем их
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
                    // Соединение разорвано, удаляем сокет из множества
                    close(sock_client);
                    clients_.erase(sock_client);
                    continue;
                }
                // выход из программы
                if(tmp[0] == '#') return false;

                // Отправляем данные обратно клиенту

                outputData(sock_client, std::move(cmd_str));
            }
        }
        return true;
    }


    void TCPServer::outputData(int sock_client, std::string&& cmd_str) noexcept
    {
        std::cout << "input cmd: ";            
        std::cout << cmd_str << " ";                
        std::cout << std::endl;
        cmd_str += " > ./tmpfile.txt";
        
        std::cout << "return system: " << std::system(cmd_str.c_str());


        std::stringstream ss{"+"};
        ss << std::ifstream("./tmpfile.txt").rdbuf();
        auto _ { std::system("rm -f tmpfile.txt") };

        send(sock_client, ss.str().c_str(), ss.str().length(), 0);
    }

    void TCPServer::start2() noexcept
    {

    int main_socket{  socket(AF_INET, SOCK_STREAM, 0) };
    if(main_socket < 0){
        std::cout << "Create socket error!" << std::endl;
        exit(0);
    }


    std::cout << "SERVER: Socked was success created" << std::endl;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3333);
    addr.sin_addr.s_addr = htons(INADDR_ANY);

    int ret{ bind(main_socket, reinterpret_cast<sockaddr*>(&addr), sizeof addr) };
    if(ret < 0) {
        std::cout << "Server error: binding connection.";
        return;
    }

    std::cout << "Server: Listening clients..." << std::endl;
    if(listen(main_socket, 5) == -1){
        std::cout << "error listen!\n";
        exit(2);
    }

    std::set<int> clients;  
    

    bool running{ true };
    while(running){
        
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(main_socket, &readset);
        for(auto&& sock_client: clients){
            FD_SET(sock_client, &readset);
        }
        timeval timeout;
        timeout.tv_sec = 150;
        timeout.tv_usec = 0;

        auto iter_max_elem{ std::max_element(clients.begin(), clients.end()) };

        int mx = std::max(main_socket, iter_max_elem == clients.end() ? main_socket : *iter_max_elem);
        if(select(mx+1, &readset, NULL, NULL, &timeout) <= 0)
        {
            perror("select");
            exit(3);
        }

        if(FD_ISSET(main_socket, &readset))
        {
            // Поступил новый запрос на соединение, используем accept
            int sock = accept(main_socket, NULL, NULL);
            if(sock < 0)
            {
                perror("accept");
                exit(3);
            }
            
            fcntl(sock, F_SETFL, O_NONBLOCK);

            clients.insert(sock);
        }

        for(auto&& sock_client: clients)
        {
            if(FD_ISSET(sock_client, &readset))
            {
                // Поступили данные от клиента, читаем их
                std::string cmd_str;                
                char tmp[256]{};
                ssize_t bytes_read{};
                do{
                    bytes_read = recv(sock_client, tmp, 256, 0);
                    cmd_str += tmp;
                } while(bytes_read == 256);


                if(bytes_read <= 0)
                {
                    // Соединение разорвано, удаляем сокет из множества
                    close(sock_client);
                    clients.erase(sock_client);
                    continue;
                }
                 if(tmp[0] == '#') {
                    running = false;
                    break;
                }

                // Отправляем данные обратно клиенту

                std::cout << "input cmd: ";            
                std::cout << cmd_str << " ";                
                std::cout << std::endl;
                cmd_str += " > ./tmpfile.txt";
                
                std::system(cmd_str.c_str());
                std::stringstream ss{"+"};
                ss << std::ifstream("./tmpfile.txt").rdbuf();
                std::system("rm -f tmpfile.txt");

                send(sock_client, ss.str().c_str(), ss.str().length(), 0);

            }
        }
        if(!running){
            
            for(auto&& sock_client: clients)
            {
                if(FD_ISSET(sock_client, &readset))
                {
                    std::string ss{"server closed!\n"};
                    send(sock_client, ss.c_str(), ss.length(), 0);
                    close(sock_client);
                }
            }
            clients.clear();
            
        }
    }
    close(main_socket);
    }
}