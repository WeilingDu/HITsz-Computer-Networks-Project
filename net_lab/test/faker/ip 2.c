#include "ip.h"
#include "arp.h"
#include "icmp.h"
#include "udp.h"
#include <string.h>
#include <stdio.h>

extern FILE *control_flow;

extern FILE *ip_fin;
extern FILE *ip_fout;

char* print_ip(uint8_t *ip);
void fprint_buf(FILE* f, buf_t* buf);

void ip_in(buf_t *buf)
{
        fprintf(ip_fout,"ip_in:");
        fprint_buf(ip_fout, buf);
}

void ip_fragment_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol, int id, uint16_t offset, int mf)
{
        fprintf(ip_fout,"ip_fragment_out:\t");        
        fprintf(ip_fout,"ip: %s\t", print_ip(ip));
        fprintf(ip_fout,"protocol: %d\t",protocol);
        fprintf(ip_fout,"id: %d\t",id);
        fprintf(ip_fout,"offset: %d\t",offset);
        fprintf(ip_fout,"mf: %d\n",mf);
        fprint_buf(ip_fout, buf);
}

void ip_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol)
{
        fprintf(ip_fout,"ip_out:\t");
        fprintf(ip_fout,"ip: %s\t", print_ip(ip));
        fprintf(ip_fout,"protocol: %d\n",protocol);
        fprint_buf(ip_fout, buf);
}