#ifndef NET_H
#define NET_H
#include "config.h"
#include <stdint.h>
typedef enum net_protocol
{
    NET_PROTOCOL_ARP = 0x0806,
    NET_PROTOCOL_IP = 0x0800,
    NET_PROTOCOL_ICMP = 1,
    NET_PROTOCOL_UDP = 17,
    NET_PROTOCOL_TCP = 6,
} net_protocol_t;

static uint8_t net_if_mac[] = DRIVER_IF_MAC;
static uint8_t net_if_ip[] = DRIVER_IF_IP;

#define NET_MAC_LEN (6)                                     //mac地址长度
#define NET_IP_LEN (4)                                      //ip地址长度
#define swap16(x) ((((x)&0xFF) << 8) | (((x) >> 8) & 0xFF)) //为16位数据交换大小端

/**
 * @brief 初始化协议栈
 * 
 */
void net_init();

/**
 * @brief 一次协议栈轮询
 * 
 */
void net_poll();

#endif