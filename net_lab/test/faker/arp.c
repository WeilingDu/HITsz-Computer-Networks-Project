#include "arp.h"
#include "utils.h"
#include "ethernet.h"
#include "config.h"
#include <string.h>
#include <stdio.h>

extern FILE *control_flow;

extern FILE *arp_fin;
extern FILE *arp_fout;

char* print_ip(uint8_t *ip);
char* print_mac(uint8_t *mac);
void fprint_buf(FILE* f, buf_t* buf);

arp_entry_t arp_table[ARP_MAX_ENTRY];
arp_buf_t arp_buf;

void arp_update(uint8_t *ip, uint8_t *mac, arp_state_t state)
{
        fprintf(arp_fout,"arp update:\t");
        fprintf(arp_fout,"ip:%s\t",print_ip(ip));
        fprintf(arp_fout,"mac:%s\t",print_mac(mac));
        fprintf(arp_fout,"state:%d\n",state);
}

void arp_in(buf_t *buf)
{
        fprintf(arp_fout,"arp_in:");
        fprint_buf(arp_fout,buf);
}

void arp_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol)
{
        fprintf(arp_fout,"arp_out\t");
        fprintf(arp_fout,"ip:%s\t",print_ip(ip));
        fprintf(arp_fout,"protocol: %d\t",protocol);
        fprint_buf(arp_fout,buf);
}

void arp_init()
{
        fprintf(arp_fout,"arp_init\n");
}