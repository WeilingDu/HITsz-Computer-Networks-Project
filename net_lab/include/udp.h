#ifndef UDP_H
#define UDP_H
#include <stdint.h>
#include "utils.h"
#pragma pack(1)
typedef struct udp_hdr
{
    uint16_t src_port;  // 源端口
    uint16_t dest_port; // 目标端口
    uint16_t total_len; // 整个数据包的长度
    uint16_t checksum;  // 校验和
} udp_hdr_t;

typedef struct udp_peso_hdr
{
    uint8_t src_ip[4];   // 源IP地址
    uint8_t dest_ip[4];  // 目的IP地址
    uint8_t placeholder; // 必须置0,用于填充对齐
    uint8_t protocol;    // 协议号
    uint16_t total_len;  // 整个数据包的长度
} udp_peso_hdr_t;
#pragma pack()

typedef struct udp_entry udp_entry_t;
typedef void (*udp_handler_t)(udp_entry_t *entry, uint8_t *src_ip, uint16_t src_port, buf_t *buf);
struct udp_entry
{
    int valid;             //有效位
    int port;              //端口号
    udp_handler_t handler; //处理程序
};

/**
 * @brief 初始化udp协议
 * 
 */
void udp_init();

/**
 * @brief 处理一个收到的udp数据包
 * 
 * @param buf 要处理的包
 * @param src_ip 源ip地址
 */
void udp_in(buf_t *buf, uint8_t *src_ip);

/**
 * @brief 处理一个要发送的数据包
 * 
 * @param buf 要处理的包
 * @param src_port 源端口号
 * @param dest_ip 目的ip地址
 * @param dest_port 目的端口号
 */
void udp_out(buf_t *buf, uint16_t src_port, uint8_t *dest_ip, uint16_t dest_port);

/**
 * @brief 发送一个udp包
 * 
 * @param data 要发送的数据
 * @param len 数据长度
 * @param src_port 源端口号
 * @param dest_ip 目的ip地址
 * @param dest_port 目的端口号
 */
void udp_send(uint8_t *data, uint16_t len, uint16_t src_port, uint8_t *dest_ip, uint16_t dest_port);

/**
 * @brief 打开一个udp端口并注册处理程序
 * 
 * @param port 端口号
 * @param handler 处理程序
 * @return int 成功为0，失败为-1
 */
int udp_open(uint16_t port, udp_handler_t handler);

/**
 * @brief 关闭一个udp端口
 * 
 * @param port 端口号
 */
void udp_close(uint16_t port);
#endif