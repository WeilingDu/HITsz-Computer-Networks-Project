#ifndef DRIVER_H
#define DRIVER_H
#include "utils.h"
#ifndef PCAP_BUF_SIZE
#define PCAP_BUF_SIZE 1024
#endif
/**
 * @brief 打开网卡
 * 
 * @return int 成功为0，失败为-1
 */
int driver_open();

/**
 * @brief 试图从网卡接收数据包
 * 
 * @param buf 收到的数据包
 * @return int 数据包的长度，未收到为0，错误为-1
 */
int driver_recv(buf_t *buf);

/**
 * @brief 使用网卡发送一个数据包
 * 
 * @param buf 要发送的数据包
 * @return int 成功为0，失败为-1
 */
int driver_send(buf_t *buf);

/**
 * @brief 关闭网卡
 * 
 */
void driver_close();
#endif