cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(project_name TCPClient)

project(${project_name})


add_executable(${project_name} main.cpp TCPClient.cpp )

