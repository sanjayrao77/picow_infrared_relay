cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(irserever C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)
pico_sdk_init()

add_executable(picow_irserver_background
				irserver.c
				)
target_compile_definitions(picow_irserver_background PRIVATE
				)
target_include_directories(picow_irserver_background PRIVATE
				${CMAKE_CURRENT_LIST_DIR}
				${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
				)
target_link_libraries(picow_irserver_background
				pico_cyw43_arch_lwip_threadsafe_background
				pico_stdlib
				)

# enable usb output, disable uart output
pico_enable_stdio_usb(picow_irserver_background 1)
pico_enable_stdio_uart(picow_irserver_background 0)

pico_add_extra_outputs(picow_irserver_background)

add_executable(picow_irserver_poll
				irserver.c
				)
target_compile_definitions(picow_irserver_poll PRIVATE
				)
target_include_directories(picow_irserver_poll PRIVATE
				${CMAKE_CURRENT_LIST_DIR}
				${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
				)
target_link_libraries(picow_irserver_poll
				pico_cyw43_arch_lwip_poll
				pico_stdlib
				)
pico_add_extra_outputs(picow_irserver_poll)
