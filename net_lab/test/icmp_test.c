#include <stdio.h>
#include <string.h>
#include "driver.h"
#include "ethernet.h"
#include "arp.h"
#include "ip.h"

extern FILE *pcap_in;
extern FILE *pcap_out;
extern FILE *pcap_demo;
extern FILE *control_flow;
extern FILE *udp_fout;
extern FILE *demo_log;
extern FILE *out_log;
extern FILE *arp_log_f;

char* print_ip(uint8_t *ip);
char* print_mac(uint8_t *mac);

uint8_t my_mac[] = DRIVER_IF_MAC;
uint8_t boardcast_mac[] = {0xff,0xff,0xff,0xff,0xff,0xff};



int check_log();
int check_pcap();

void log_tab_buf();

buf_t buf;
int main(){
        int ret;
        printf("\e[0;34mTest begin.\n");
        pcap_in = fopen("data/icmp_test/in.pcap","r");
        pcap_out = fopen("data/icmp_test/out.pcap","w");
        control_flow = fopen("data/icmp_test/log","w");
        if(pcap_in == 0 || pcap_out == 0 || control_flow == 0){
                if(pcap_in) fclose(pcap_in); else printf("\e[1;31mFailed to open in.pcap\n");
                if(pcap_out)fclose(pcap_out); else printf("\e[1;31mFailed to open out.pcap\n");
                if(control_flow) fclose(control_flow); else printf("\e[1;31mFailed to open log\n");
                return 0;
        }
        udp_fout = control_flow;
        arp_log_f = control_flow;

        if(ethernet_init()){
                fprintf(stderr,"\e[1;31mDriver open failed,exiting\n");
                fclose(pcap_in);
                fclose(pcap_out);
                fclose(control_flow);
                return 0;
        }
        arp_init();
        log_tab_buf();
        int i = 1;
        printf("\e[0;34mFeeding input %02d",i);
        while((ret = driver_recv(&buf)) > 0){
                printf("\b\b%02d",i);
                fprintf(control_flow,"\nRound %02d -----------------------------\n",i++);
                if(memcmp(buf.data,my_mac,6) && memcmp(buf.data,boardcast_mac,6)){
                        buf_t buf2;
                        buf_copy(&buf2, &buf);
                        memset(buf2.data,0,sizeof(ether_hdr_t));
                        buf_remove_header(&buf2, sizeof(ether_hdr_t));
                        int len = (buf2.data[0] & 0xf) << 2;
                        uint8_t * ip = buf.data + 30;
                        net_protocol_t pro = buf2.data[9];
                        memset(buf2.data,0,sizeof(len));
                        buf_remove_header(&buf2, len);
                        ip_out(&buf2,ip,pro);
                }else{
                        ethernet_in(&buf);
                }
                log_tab_buf();
        }
        if(ret < 0){
                fprintf(stderr,"\e[1;31m\nError occur on loading input,exiting\n");
        }
        driver_close();
        printf("\e[0;34m\nSample input all processed, checking output\n");

        fclose(control_flow);

        demo_log = fopen("data/icmp_test/demo_log","r");
        out_log = fopen("data/icmp_test/log","r");
        pcap_out = fopen("data/icmp_test/out.pcap","r");
        pcap_demo = fopen("data/icmp_test/demo_out.pcap","r");
        if(demo_log == 0 || out_log == 0 || pcap_out == 0 || pcap_demo == 0){
                if(demo_log) fclose(demo_log); else printf("\e[1;31mFailed to open demo_log\n\e[0m");
                if(out_log) fclose(out_log); else printf("\e[1;31mFailed to open log\n\e[0m");
                if(pcap_demo) fclose(pcap_demo); else printf("\e[1;31mFailed to open demo_out.pcap\n\e[0m");
                if(pcap_out) fclose(pcap_out); else printf("\e[1;31mFailed to open out.pcap\n\e[0m");
                return 0;
        }
        check_log();
        check_pcap();
        printf("\e[1;33mFor this test, log is only a reference. Your implementation is OK if your pcap file is the same to the demo pcap file.\n");
        fclose(demo_log);
        fclose(out_log);
        return 0;
}