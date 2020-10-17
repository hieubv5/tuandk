cmake_minimum_required(VERSION 3.5)

project (tuanml)

include_directories(inc)

set(SOURCES src/main.c src/crc32.c src/RingBuffer.c src/slist.c)

add_executable(tuanml ${SOURCES})