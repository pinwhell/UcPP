find_package(simplistic-io REQUIRED)
file(GLOB UCPP_SRCS *.cpp)
add_library_ns(ucpp ucpp STATIC ${UCPP_SRCS})
target_compile_features(ucpp-ucpp PUBLIC cxx_std_17)
target_link_libraries(ucpp-ucpp 
unicorn::unicorn
simplistic::io)
install_target_and_headers(ucpp ucpp)