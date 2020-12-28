#include <stdio.h>
#include "driver.h"
#include "ethernet.h"

extern FILE *pcap_in;
extern FILE *pcap_out;
extern FILE *control_flow;
extern FILE *ip_fout;
extern FILE *arp_fout;
extern FILE *demo_log;
extern FILE *out_log;

int check_log();

buf_t buf;
int main(){
        int ret;
        pcap_in = fopen("data/eth_in/in.pcap","r");
        pcap_out = fopen("data/eth_in/out.pcap","w");
        ip_fout = fopen("data/eth_in/log","w");
        if(pcap_in == 0 || pcap_out == 0 || ip_fout == 0){
                if(pcap_in) fclose(pcap_in); else printf("\e[1;31mFailed to open in.pcap\n");
                if(pcap_out)fclose(pcap_out); else printf("\e[1;31mFailed to open out.pcap\n");
                if(ip_fout) fclose(ip_fout); else printf("\e[1;31mFailed to open log\n");
                return 0;
        }
        arp_fout = ip_fout;
        control_flow = ip_fout;

        printf("\e[0;34mTest start\n");
        if(ethernet_init()){
                fprintf(stderr,"\e[1;31mDriver open failed,exiting\n");
                fclose(pcap_in);
                fclose(pcap_out);
                fclose(ip_fout);
                return 0;
        }
        int i = 1;
        printf("\e[0;34mFeeding input %02d",i);
        while((ret = driver_recv(&buf)) > 0){
                printf("\b\b%02d",i);
                fprintf(control_flow,"\nRound %02d -----------------------------\n",i++);
                ethernet_in(&buf);
        }
        if(ret < 0){
                fprintf(stderr,"\e[1;31m\nError occur on loading input,exiting\n");
        }
        driver_close();
        printf("\e[0;34m\nSample input all processed, checking output\n");

        fclose(ip_fout);

        demo_log = fopen("data/eth_in/demo_log","r");
        out_log = fopen("data/eth_in/log","r");
        if(demo_log == 0 || out_log == 0){
                if(demo_log) fclose(demo_log); else printf("\e[1;31mFailed to open demo_log\n");
                if(out_log) fclose(out_log); else printf("\e[1;31mFailed to open log\n");
                return 0;
        }
        check_log();
        fclose(demo_log);
        fclose(out_log);
        return 0;
}