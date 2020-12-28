#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "utils.h"
#include "config.h"
#include "driver.h"
extern FILE *pcap_in;
extern FILE *pcap_out;
extern FILE *control_flow;

static const int cap_in[] = {4,5,23,24,26,28,54,64,65,74,80,92,98,112};
static const int cap_out[] = {2,3,21,22,25,27,34,35,53,73,75,79,95,114};
static const uint8_t my_mac[] = DRIVER_IF_MAC;
static const uint8_t my_ip[] = DRIVER_IF_IP;

static const uint8_t bc_mac[] = {0xff,0xff,0xff,0xff,0xff,0xff};
static const uint8_t blk_mac[] = {0x00,0x00,0x00,0x00,0x00,0x00}; 

uint8_t mac[][6] =
{
        {0x01,0x12,0x23,0x34,0x45,0x56},
        {0x21,0x32,0x43,0x54,0x65,0x06},
        {0x1a,0x94,0xf0,0x3c,0x49,0xaa}
};

uint8_t ip[][4] =
{
        {192,168,163,110},
        {192,168,163,10},
        {192,168,163,2}
};

buf_t buf;
buf_t* redirect_in(int id)
{
        memcpy(buf.data,my_mac,6);
        memcpy(buf.data+6,mac[id],6);
        if(buf.data[13] == 0x06){
                memcpy(buf.data+22,mac[id],6);
                memcpy(buf.data+28,ip[id],4);
                memcpy(buf.data+32,my_mac,6);
                memcpy(buf.data+38,my_ip,4);
        }else{
                memcpy(buf.data+26,ip[id],4);
                memcpy(buf.data+30,my_ip,4);
                buf.data[24] = 0; buf.data[25] = 0;
                int hd_len = (buf.data[14] & 0xf) << 2;
                int cs = checksum16((uint16_t*)(buf.data+14),hd_len);
                printf("cs:%x\n",cs);
                buf.data[25] = (cs >> 8);
                buf.data[24] = cs;
        }
        return &buf;
}

buf_t* redirect_out(int id)
{
        memcpy(buf.data+6,my_mac,6);
        memcpy(buf.data,mac[id],6);
        if(buf.data[13] == 0x06){
                memcpy(buf.data+22,my_mac,6);
                memcpy(buf.data+28,my_ip,4);
        }else{
                memcpy(buf.data+26,my_ip,4);
                memcpy(buf.data+30,ip[id],4);
                buf.data[24] = 0; buf.data[25] = 0;
                int hd_len = (buf.data[14] & 0xf) << 2;
                int cs = checksum16((uint16_t*)(buf.data+14),hd_len);
                printf("cs:%x\n",cs);
                buf.data[25] = cs >> 8;
                buf.data[24] = cs;
        }
        return &buf;
}

static char arp_pkt[64];
void initArp()
{
        arp_pkt[12] = 0x08; arp_pkt[13] = 0x06;
        arp_pkt[14] = 0x00; arp_pkt[15] = 0x01;
        arp_pkt[16] = 0x08; arp_pkt[17] = 0x00;
        arp_pkt[18] = 0x06;
        arp_pkt[19] = 0x04;
}

buf_t* genArp(uint8_t id,uint8_t type)
{
        if(type == 0){
                memcpy(arp_pkt, bc_mac,6);
                memcpy(arp_pkt+6,mac[id],6);
                arp_pkt[20] = 0x00; arp_pkt[21] = 0x01;
                memcpy(arp_pkt+22,mac[id], 6);
                memcpy(arp_pkt+28,ip[id], 4);
                memcpy(arp_pkt+32,blk_mac,6);
                memcpy(arp_pkt+38,my_ip,4);
        }else{
                memcpy(arp_pkt, my_mac, 6);
                memcpy(arp_pkt+6, mac[id], 6);
                arp_pkt[20] = 0x00; arp_pkt[21] = 0x02;
                memcpy(arp_pkt+22,mac[id], 6);
                memcpy(arp_pkt+28,ip[id],4);
                memcpy(arp_pkt+32,my_mac,6);
                memcpy(arp_pkt+38,my_ip,4);
        }
        buf_init(&buf,42);
        memcpy(buf.data,arp_pkt,42);
        return &buf;
}

int main()
{
        int ret;
        srand(time(0));
        pcap_in = fopen("/home/null/pcap_file/1.pcap","r");
        pcap_out = fopen("data/in.pcap","w");
        control_flow = fopen("data/control.txt","w");

        if(driver_open()){
                fprintf(stderr,"driver open failed,exiting\n");
                return 0;
        }
        int i = 1;
        int j = 0;
        int k = 0;
        initArp();
        while((ret = driver_recv(&buf)) > 0){
                if(i == 21){
                        driver_send(redirect_out(1));
                        driver_send(genArp(1,1));
                }
                if(i == 22 || i == 25){
                        driver_send(redirect_out(1));
                }
                if(i == 23 || i == 24 || i == 26){
                        driver_send(redirect_in(1));
                }
                if(i == 27){
                        driver_send(redirect_in(0));
                        driver_send(genArp(0,1));
                        driver_send(genArp(2,0));
                }
                if(i == 28){
                        driver_send(redirect_out(0));
                }
                if(i == 33){
                        driver_send(redirect_in(2));
                }
                if(i == 34){
                        driver_send(redirect_out(2));
                }
                if(i == 114){
                        driver_send(redirect_out(1));
                }
                if(i == 115){
                        driver_send(redirect_in(1));
                }
                // if(i == cap_in[j]){
                //         j++;
                //         redirect_in();
                //         driver_send(&buf);
                // }else if(i == cap_out[k]){
                //         k++;
                //         redirect_out();
                //         driver_send(&buf);
                // }
                i++;
        }
        // for(int i = 0; i < 3; i++){
        //         buf_init(&buf,60);
        //         for(int j = 0; j < 60; j++){
        //                 buf.data[j] = rand();
        //         }
        //         redirect_in();
        //         // redirect_out();
        //         driver_send(&buf);
        // }
        if(ret < 0){
                fprintf(stderr,"error occur on receive,exiting\n");
        }
        driver_close();
        
        // fclose(pcap_in);
        // fclose(pcap_out);
        fclose(control_flow);
        return 0;
}