cmake_minimum_required(VERSION 3.12)
project(Store)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQXX REQUIRED libpqxx)

include_directories(include)
include_directories(${LIBPQXX_INCLUDE_DIRS})

set(SOURCES
    src/Database.cpp
    src/Logger.cpp
    src/Order.cpp
    src/Payment.cpp
    src/Products.cpp
    src/User.cpp
)

add_executable(Store main.cpp ${SOURCES})

target_compile_features(Store PRIVATE cxx_std_17)
target_link_libraries(Store ${LIBPQXX_LIBRARIES})

if(WIN32)
    target_link_libraries(Store ws2_32)
endif()