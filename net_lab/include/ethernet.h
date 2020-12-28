#ifndef ETHERNET_H
#define ETHERNET_H
#include <stdint.h>
#include "net.h"
#include "utils.h"

#pragma pack(1)

typedef struct ether_hdr
{
    uint8_t dest[NET_MAC_LEN]; // 目标mac地址
    uint8_t src[NET_MAC_LEN];  // 源mac地址
    uint16_t protocol;         // 协议/长度
} ether_hdr_t;
#pragma pack()

/**
 * @brief 初始化以太网协议
 * 
 * @return int 成功为0，失败为-1
 */
int ethernet_init();

/**
 * @brief 处理一个收到的数据包
 * 
 * @param buf 要处理的数据包
 */
void ethernet_in(buf_t *buf);

/**
 * @brief 处理一个要发送的数据包
 * 
 * @param buf 要处理的数据包
 * @param mac 目标ip地址
 * @param protocol 上层协议
 */
void ethernet_out(buf_t *buf, const uint8_t *mac, net_protocol_t protocol);

/**
 * @brief 一次以太网轮询
 * 
 */
void ethernet_poll();

static const uint8_t ether_broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //以太网广播mac地址
#endif