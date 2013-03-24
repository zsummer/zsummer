cmake_minimum_required(VERSION 2.6)
project(zsummer_server)

aux_source_directory(../../utility UT_S)
aux_source_directory(../../tools/thread4z TH_S)
aux_source_directory(../../tools/log4z LG_S)
aux_source_directory(../../network/epoll EP_S)
aux_source_directory(./ LL_S)

set(source ${UT_S} ${TH_S} ${LG_S} ${EP_S} ${LL_S})
message(${source})
link_libraries( rt pthread)
add_definitions(-Wall -g -ggdb )

set(executable_output_path ./)
add_executable(zsumeer_server ${source})


