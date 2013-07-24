cmake_minimum_required(VERSION 2.6)
project(asio_client)


link_libraries(rt pthread)
link_libraries(boost_thread boost_thread boost_filesystem boost_system boost_date_time)
add_definitions(-Wall -g -ggdb )

set(executable_output_path ./)
add_executable(asio_client asio_client.cpp)



