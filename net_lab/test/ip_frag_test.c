#include <stdio.h>
#include <string.h>

#include "net.h"
#include "ip.h"
#include "utils.h"

extern FILE *control_flow;
extern FILE *arp_fout;

buf_t buf;
int main()
{
        FILE *in = fopen("data/ip_frag_test/in.txt","r");
        control_flow = fopen("data/ip_frag_test/log","w");
        if(in == 0 || control_flow == 0){
                in && fclose(in);
                control_flow && fclose(control_flow);
                return 0;
        }
        arp_fout = control_flow;
        char * p = buf.payload + 1000;
        buf.data = p;
        buf.len = 0;
        char c;
        while(fread(&c,1,1,in)){
                *p = c;
                p++;
                buf.len++;
        }
        printf("\e[0;34mFeeding input.\n");
        ip_out(&buf,net_if_ip,NET_PROTOCOL_TCP);

        fclose(in);
        fclose(control_flow);

        FILE * demo = fopen("data/ip_frag_test/demo_log","r");
        FILE * log = fopen("data/ip_frag_test/log","r");
        int line = 1;
        int column = 0;
        int diff = 0;
        char c1,c2;
        printf("\e[0;34mComparing logs.\n");
        while(fread(&c1,1,1,demo)){
                column++;
                if(fread(&c2,1,1,log) <= 0){
                        printf("\e[0;31mLog file shorter than expected.\n");
                        diff = 1;
                        break;
                }
                if(c1 != c2){
                        printf("\e[0;31mDifferent char found at line %d column %d.\n",line,column);
                        diff = 1;
                        break;
                }
                if(c1 == '\n'){
                        line ++;
                        column = 0;
                }
        }
        if(diff == 0 && fread(&c2,1,1,log) == 1){
                printf("\e[0;31mLog file longer than expected.\n");
                diff = 1;
        }
        if(diff == 0){
                printf("\e[1;32mLog file check passed\n");
        }
        fclose(log);
        fclose(demo);
        return 0;
}

