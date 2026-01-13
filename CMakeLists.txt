cmake_minimum_required(VERSION 3.12)
project(Store)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(libpqxx REQUIRED)
find_package(PostgreSQL REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${PostgreSQL_INCLUDE_DIRS})
include_directories(${libpqxx_INCLUDE_DIRS})

set(SOURCES
    main.cpp
    src/Database.cpp
    src/Logger.cpp
    src/Order.cpp
    src/Payment.cpp
    src/Products.cpp
    src/User.cpp
    src/SqlScript.cpp
    src/AuditManager.cpp
    src/Menu.cpp
    src/AdminMenu.cpp
    src/ManagerMenu.cpp
    src/CustomerMenu.cpp
)

add_executable(Store ${SOURCES})

target_link_libraries(Store 
    ${PostgreSQL_LIBRARIES}
    libpqxx::pqxx
)

if(WIN32)
    target_link_libraries(Store ws2_32)
endif()