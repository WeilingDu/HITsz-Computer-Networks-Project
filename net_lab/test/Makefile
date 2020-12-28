SRC=../src/

CC=gcc

LFLAG=-lpcap -I../include/

test_icmp:
	$(CC) icmp_test.c $(SRC)ethernet.c $(SRC)arp.c $(SRC)ip.c $(SRC)icmp.c faker/udp.c faker/driver.c global.c $(SRC)utils.c -o icmp_test $(LFLAG)
	./icmp_test

test_ip_frag:
	$(CC) ip_frag_test.c faker/arp.c $(SRC)ip.c faker/icmp.c faker/udp.c global.c $(SRC)utils.c -o ip_frag_test $(LFLAG)
	./ip_frag_test

test_ip:
	$(CC) ip_test.c $(SRC)ethernet.c $(SRC)arp.c $(SRC)ip.c faker/icmp.c faker/udp.c faker/driver.c global.c $(SRC)utils.c -o ip_test $(LFLAG)
	./ip_test

test_arp:
	$(CC) arp_test.c $(SRC)ethernet.c $(SRC)arp.c faker/ip.c faker/driver.c global.c $(SRC)utils.c -o arp_test $(LFLAG)
	./arp_test

test_eth_out:
	$(CC) eth_out_test.c $(SRC)ethernet.c faker/arp.c faker/ip.c faker/driver.c global.c $(SRC)utils.c -o eth_out_test $(LFLAG)
	./eth_out_test

test_eth_in:
	$(CC) eth_in_test.c $(SRC)ethernet.c faker/arp.c faker/ip.c faker/driver.c global.c $(SRC)utils.c -o eth_in_test $(LFLAG)
	./eth_in_test

clean:
	find -maxdepth 1 -type f -name "*_test" -delete
	find -type f -name "log" -delete
	find -type f -name "out.pcap" -delete

# Following not in use for testing
test_dv:
	$(CC) driver_test.c faker/driver.c global.c $(SRC)utils.c -o driver_test $(LFLAG)
	./driver_test 

demo:
	mv data/$(DIR)/log data/$(DIR)/demo_log
	mv data/$(DIR)/out.pcap data/$(DIR)/demo_out.pcap
