#include <pcap.h>
#include <string.h>
#include "utils.h"
#include "config.h"
#include "driver.h"

static pcap_t *pcap;
static char pcap_errbuf[PCAP_ERRBUF_SIZE];

/**
 * @brief 打开网卡
 * 
 * @return int 成功为0，失败为-1
 */
int driver_open()
{
    uint32_t net, mask;

    // 根据DRIVER_IF_NAME网卡名，获取网卡的网络号net和子网掩码mask
    if (pcap_lookupnet(DRIVER_IF_NAME, &net, &mask, pcap_errbuf) == -1) //查找网卡
    {
        fprintf(stderr, "Error in pcap_lookupnet: %s\n", pcap_geterr(pcap));
        return -1;
    }

    // 获取一个数据包捕获的描述符，以便用来查看网络上的数据包。
    // 第二个参数表示捕获的最大字节数，通常来说数据包的大小不会超过65535
    // 第三个参数表示开启混杂模式，0表示非混杂模式，任何其他值表示混合模式
    // 第四个参数指定需要等待的毫秒数，0表示一直等待直到有数据包到来
    if ((pcap = pcap_open_live(DRIVER_IF_NAME, 65536, 1, 0, pcap_errbuf)) == NULL) //混杂模式打开网卡
    {
        fprintf(stderr, "Error in pcap_open_live: %s.\n", pcap_geterr(pcap));
        return -1;
    }
    if (pcap_setnonblock(pcap, 1, pcap_errbuf) != 0) //设置非阻塞模式
    {
        fprintf(stderr, "Error in pcap_setnonblock: %s\n", pcap_geterr(pcap));
        return -1;
    }
    char filter_exp[PCAP_BUF_SIZE];
    struct bpf_program fp;
    uint8_t mac_addr[6] = DRIVER_IF_MAC;
    sprintf(filter_exp, //过滤数据包
            "(ether dst %02x:%02x:%02x:%02x:%02x:%02x or ether broadcast) and (not ether src %02x:%02x:%02x:%02x:%02x:%02x)",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5],
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    
    // 只捕获发往本网卡接口与广播的数据帧，也就是只处理发往这张网卡的数据包
    if (pcap_compile(pcap, &fp, filter_exp, 0, net) == -1)
    {
        fprintf(stderr, "Error in pcap_compile: %s\n", pcap_geterr(pcap));
        return -1;
    }
    if (pcap_setfilter(pcap, &fp) == -1)
    {
        fprintf(stderr, "Error in pcap_setfilter: %s\n", pcap_geterr(pcap));
        return -1;
    }
    return 0;
}

/**
 * @brief 试图从网卡接收数据包
 * 
 * @param buf 收到的数据包
 * @return int 数据包的长度，未收到为0，错误为-1
 */
int driver_recv(buf_t *buf)
{
    struct pcap_pkthdr *pkt_hdr;
    const uint8_t *pkt_data;

    // 从本网卡接口获取一个数据报文
    int ret = pcap_next_ex(pcap, &pkt_hdr, &pkt_data);
    if (ret == 0)
        return 0;
    else if (ret == 1)
    {
        memcpy(buf->data, pkt_data, pkt_hdr->len);
        buf->len = pkt_hdr->len;
        return pkt_hdr->len;
    }
    fprintf(stderr, "Error in driver_recv: %s\n", pcap_geterr(pcap));
    return -1;
}

/**
 * @brief 使用网卡发送一个数据包
 * 
 * @param buf 要发送的数据包
 * @return int 成功为0，失败为-1
 */
int driver_send(buf_t *buf)
{
    // 将数据包发往指定的网卡接口
    if (pcap_sendpacket(pcap, buf->data, buf->len) == -1)
    {
        fprintf(stderr, "Error in driver_send: %s\n", pcap_geterr(pcap));
        return -1;
    }

    return 0;
}

/**
 * @brief 关闭网卡
 * 
 */
void driver_close()
{
    pcap_close(pcap);
}
