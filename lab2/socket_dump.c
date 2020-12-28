#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#define BUFFER_MAX 2048
int main(int argc,char* argv[]){
    int fd;
    int proto;
    int str_len;
    char buffer[BUFFER_MAX];
    char *ethernet_head;
    char *ip_head;
    char *rfc_type_head;
    char *arp_head;
    unsigned char *p;
    if((fd=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL)))<0)
    {
        printf("error create raw socket\n");
        return -1;
    }
    while(1){
        str_len = recvfrom(fd,buffer,2048,0,NULL,NULL);
        if(str_len < 42)
        {
            printf("error when recv msg \n");
            return -1;
        }
        ethernet_head = buffer;
        p = ethernet_head;

        rfc_type_head = ethernet_head + 12;
        if(rfc_type_head[0] == 0x08){
            if(rfc_type_head[1] == 0x00){
                // 0800
            //     ip_head = ethernet_head + 14;
            //     p = ip_head + 12;
            //     proto = (ip_head + 9)[0];
            //     switch(proto){
            //         case IPPROTO_ICMP:
			//             printf("icmp\n");
			//             char *icmp_head = ip_head + 20;
			//             unsigned char *type = icmp_head;
			//             printf("Description: \n");
			//             if((type[0] == 0x00)&&(type[1] == 0x00))
            //                 printf("Type : 0 (Echo Reply)\nCode : 0\n");
            //             else if(type[0] == 0x03){
			// 	            if(type[1] == 0x00)
            //                     printf("Type : 3 (NetWork Unreachable)\nCode : 0\n");
			// 	            if(type[1] == 0x01)	
            //                     printf("Type : 3(Host Unreachable)\nCode : 1\n");
			// 	            if(type[1] == 0x02)	
            //                     printf("Type : 3(Protocol Unreachable)\nCode : 2\n");
			// 	            if(type[1] == 0x03)	
            //                     printf("Type : 3(Port Unreachable)\nCode : 3\n");
			//             }
			//             else if((type[0] == 0x08)&&(type[1] == 0x00))	
            //                 printf("Type : 8 (Echo Request)\nCode : 0\n");
			//             else if((type[0] == 0x11)&&(type[1] == 0x00))	
            //                 printf("Type : 11(TTL equals 0 during transit)\nCode : 0\n");
			//             else if((type[0] == 0x12)&&(type[1] == 0x00))	
            //                 printf("Type : 12(IP header bad(catchall error))\nCode : 0\n");
                            
			//             unsigned char *p = type + 2;
            //             printf("CheckSum : 0x%d%d\n",p[0],p[1]);
            //             p = p + 2;
			//             int be = (p[0] << 8) + p[1];
            //             printf("Identifier (BE):%d(0x%02x%02x)\n",be,p[0],p[1]);
			//             int le = (p[1] << 8) + p[0];
            //             printf("Identifier (LE):%d(0x%02x%02x)\n",le,p[1],p[0]);
            //             p = p + 2;
			//             be = (p[0] << 8) + p[1];
            //             printf("Sequence number (BE):%d(0x%02x%02x)\n",be,p[0],p[1]);
			//             le = (p[1] << 8) + p[0];
            //             printf("Sequence number (LE):%d(0x%02x%02x)\n\n",le,p[1],p[0]);
            //             break;
            //         default:
            //             break;
            //     }
            }
            else if(rfc_type_head[1] == 0x06){
                // 0806
		        printf("Dst MAC address: %.2x:%02x:%02x:%02x:%02x:%02x\n",p[0],p[1],p[2],p[3],p[4],p[5]);
        	    printf("Src MAC address: %.2x:%02x:%02x:%02x:%02x:%02x\n",p[6],p[7],p[8],p[9],p[10],p[11]);
                printf("Address Resolution Protocol\n");
		
                unsigned char *hardware_type;
                unsigned char *protocol_type;
                unsigned char *temp;
                arp_head = ethernet_head + 14;
                hardware_type = arp_head;
                protocol_type = arp_head + 2;
                
                if((hardware_type[0] == 0x00)&&(hardware_type[1] == 0x01))
                    printf("Hardware type : Ethernet\n");
                else
                    printf("Hardware type : unknown\n");
                if((protocol_type[0] == 0x08)&&(protocol_type[1] == 0x00))
                    printf("Protocal type : IPv4(0x0800)\n");
                else
                    printf("Protocal type : unknown\n");
                    
                printf("Hardware size : 6\n");
                printf("Protocol size : 4\n");
                
                temp = protocol_type + 4;
                if(temp[1] == 0x01)
                    printf("Opcode : request (1)\n" );
                else if(temp[1] == 0x02)
                    printf("Opcode : recieve (2)\n" );        
                temp = temp + 2;
                printf("Sender MAC address: %.2x:%02x:%02x:%02x:%02x:%02x\n",temp[0],temp[1],temp[2],temp[3],temp[4],temp[5]);
                temp = temp + 6;
                printf("Sender IP: %d.%d.%d.%d\n",temp[0],temp[1],temp[2],temp[3]);
                temp = temp + 4;
                printf("Target MAC address: %.2x:%02x:%02x:%02x:%02x:%02x\n",temp[0],temp[1],temp[2],temp[3],temp[4],temp[5]);
                temp = temp + 6;
                printf("Target IP: %d.%d.%d.%d\n\n",temp[0],temp[1],temp[2],temp[3]);
            }
            else if(rfc_type_head[1] == 0x35){
                printf("Reverse Address Resolution Protocol\n");
            }
        }    
    }
    return -1;
}
