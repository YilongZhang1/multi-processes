cmake_minimum_required (VERSION 3.6)
project(multiProcesses)

set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -Wshadow -Werror")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Werror")

include_directories (${PROJECT_SOURCE_DIR}/include)

add_library (csvFileParser SHARED src/csvFileParser.cpp)
add_library (argMax_and_distanceMatrix SHARED src/argMax_and_distanceMatrix.cpp)

install (TARGETS csvFileParser DESTINATION lib)
install (TARGETS argMax_and_distanceMatrix DESTINATION lib)

INSTALL(FILES include/fileParsing.h DESTINATION include)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} csvFileParser argMax_and_distanceMatrix)
