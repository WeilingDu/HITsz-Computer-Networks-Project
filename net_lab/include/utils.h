#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
#include "config.h"
#define BUF_MAX_LEN (UINT16_MAX + 14) //最大udp包 + 以太网帧报头长度

typedef struct buf
{
    uint16_t len;                       // 包中有效数据大小
    uint8_t *data;                      // 包的数据起始地址
    uint8_t payload[BUF_MAX_LEN];       // 最大负载数据量
} buf_t;
static buf_t rxbuf, txbuf;          //一个buf足够单线程使用

/**
 * @brief 初始化buffer为给定的长度，用于装载数据包
 * 
 * @param buf 要初始化的buffer
 * @param len 长度
 */
void buf_init(buf_t *buf, int len); //buf可以在头部装卸数据，以供协议头的添加和去除

/**
 * @brief 为buffer在头部增加一段长度，用于添加协议头
 * 
 * @param buf 要修改的buffer
 * @param len 增加的长度
 */
void buf_add_header(buf_t *buf, int len);

/**
 * @brief 为buffer在头部减少一段长度，去除协议头
 * 
 * @param buf 要修改的buffer
 * @param len 减少的长度
 */
void buf_remove_header(buf_t *buf, int len);

/**
 * @brief 复制一个buffer到新buffer
 * 
 * @param dst 目的buffer
 * @param src 源buffer
 */
void buf_copy(buf_t *dst, buf_t *src);

/**
 * @brief 计算16位校验和
 * 
 * @param buf 要计算的数据包
 * @param len 要计算的长度
 * @return uint16_t 校验和
 */
uint16_t checksum16(uint16_t *buf, int len);

/**
 * @brief ip转字符串
 * 
 * @param ip ip地址
 * @return char* 生成的字符串
 */
char *iptos(uint8_t *ip);

#endif