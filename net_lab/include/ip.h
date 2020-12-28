#ifndef IP_H
#define IP_H
#include <stdint.h>
#include "net.h"
#include "utils.h"
#pragma pack(1)
typedef struct ip_hdr
{
    uint8_t hdr_len : 4;         // 首部长, 4字节为单位
    uint8_t version : 4;         // 版本号
    uint8_t tos;                 // 服务类型
    uint16_t total_len;          // 总长度
    uint16_t id;                 // 标识符
    uint16_t flags_fragment;     // 标志与分段
    uint8_t ttl;                 // 存活时间
    uint8_t protocol;            // 上层协议
    uint16_t hdr_checksum;       // 首部校验和
    uint8_t src_ip[NET_IP_LEN];  // 源IP
    uint8_t dest_ip[NET_IP_LEN]; // 目标IP
} ip_hdr_t;
#pragma pack()

#define IP_HDR_LEN_PER_BYTE (4)    //ip包头长度单位
#define IP_HDR_OFFSET_PER_BYTE (8) //ip分片偏移长度单位
#define IP_VERSION_4 (4)           //ipv4
#define IP_MORE_FRAGMENT 1 << 5    //ip分片mf位


/**
 * @brief 处理一个收到的数据包
 * 
 * @param buf 要处理的包
 */
void ip_in(buf_t *buf);

/**
 * @brief 处理一个要发送的ip数据包
 * 
 * @param buf 要处理的包
 * @param ip 目标ip地址
 * @param protocol 上层协议
 */
void ip_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol);
#endif