#include "net.h"
#include "arp.h"
#include "udp.h"
#include "ethernet.h"

/**
 * @brief 初始化协议栈
 * 
 */
void net_init()
{
    ethernet_init();
    arp_init();
    udp_init();
}

/**
 * @brief 一次协议栈轮询
 * 
 */
void net_poll()
{
    ethernet_poll();
}