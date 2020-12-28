#ifndef ARP_H
#define ARP_H

#include <stdint.h>
#include <time.h>
#include "config.h"
#include "net.h"
#include "utils.h"
#define ARP_HW_ETHER 0x1 // 以太网
#define ARP_REQUEST 0x1  // ARP请求包
#define ARP_REPLY 0x2    // ARP响应包

typedef enum arp_state
{
    ARP_PENDING, //等待响应
    ARP_VALID,   //有效
    ARP_INVALID, //无效
} arp_state_t;

typedef struct arp_entry
{
    arp_state_t state;        //状态
    time_t timeout;           //超时时间戳
    uint8_t ip[NET_IP_LEN];   //ip地址
    uint8_t mac[NET_MAC_LEN]; //mac地址
} arp_entry_t;

typedef struct arp_buf
{
    int valid;               //有效位
    buf_t buf;               //数据包
    uint8_t ip[NET_IP_LEN];  //目的ip地址
    net_protocol_t protocol; //上层协议
} arp_buf_t;

#pragma pack(1)
typedef struct arp_pkt
{
    uint16_t hw_type, pro_type;      // 硬件类型和协议类型
    uint8_t hw_len, pro_len;         // 硬件地址长 + 协议地址长
    uint16_t opcode;                 // 请求/响应
    uint8_t sender_mac[NET_MAC_LEN]; // 发送包硬件地址
    uint8_t sender_ip[NET_IP_LEN];   // 发送包协议地址
    uint8_t target_mac[NET_MAC_LEN]; // 接收方硬件地址
    uint8_t target_ip[NET_IP_LEN];   // 接收方协议地址
} arp_pkt_t;

#pragma pack()

/**
 * @brief 初始化arp协议
 * 
 */
void arp_init();

/**
 * @brief 处理一个收到的数据包
 * 
 * @param buf 要处理的数据包
 */
void arp_in(buf_t *buf);

/**
 * @brief 处理一个要发送的数据包
 * 
 * @param buf 要处理的数据包
 * @param ip 目标ip地址
 * @param protocol 上层协议
 */
void arp_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol);

/**
 * @brief 更新arp表
 * 
 * @param ip ip地址
 * @param mac mac地址
 * @param state 表项的状态
 */
void arp_update(uint8_t *ip, uint8_t *mac, arp_state_t state);
#endif