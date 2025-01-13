if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
set(UNICORN_LIBS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/unicorn64)
else()
set(UNICORN_LIBS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/unicorn32)
endif()

add_library_ns(unicorn unicorn STATIC d.cpp)
file(GLOB UNICORN_LIBS ${UNICORN_LIBS_PATH}/*.lib)
target_link_libraries(unicorn-unicorn ${UNICORN_LIBS})
install_target_and_headers(unicorn unicorn)

install(FILES ${UNICORN_LIBS} DESTINATION lib/cmake/ucpp)