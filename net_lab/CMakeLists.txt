cmake_minimum_required(VERSION 3.0.0)
project(net VERSION 0.1.0)

include_directories(./include ./pcap)
aux_source_directory(./src DIR_SRCS)
add_executable(main ${DIR_SRCS})
target_link_libraries(main pcap)


SET(EXECUTABLE_OUTPUT_PATH ../test) 
add_executable(ctest_icmp ./test/icmp_test.c ./src/ethernet.c ./src/arp.c ./src/ip.c ./src/icmp.c ./test/faker/udp.c ./test/faker/driver.c ./test/global.c ./src/utils.c)
target_link_libraries(ctest_icmp pcap)

add_executable(ctest_ip_frag ./test/ip_frag_test.c ./test/faker/arp.c ./src/ip.c ./test/faker/icmp.c ./test/faker/udp.c ./test/global.c ./src/utils.c)
target_link_libraries(ctest_ip_frag pcap)

add_executable(ctest_ip ./test/ip_test.c ./src/ethernet.c ./src/arp.c ./src/ip.c ./test/faker/icmp.c ./test/faker/udp.c ./test/faker/driver.c ./test/global.c ./src/utils.c)
target_link_libraries(ctest_ip pcap)

add_executable(ctest_arp ./test/arp_test.c ./src/ethernet.c ./src/arp.c ./test/faker/ip.c ./test/faker/driver.c ./test/global.c ./src/utils.c)
target_link_libraries(ctest_arp pcap)

add_executable(ctest_eth_out ./test/eth_out_test.c ./src/ethernet.c ./test/faker/arp.c ./test/faker/ip.c ./test/faker/driver.c ./test/global.c ./src/utils.c)
target_link_libraries(ctest_eth_out pcap)

add_executable(ctest_eth_in ./test/eth_in_test.c ./src/ethernet.c ./test/faker/arp.c ./test/faker/ip.c ./test/faker/driver.c ./test/global.c ./src/utils.c)
target_link_libraries(ctest_eth_in pcap)

