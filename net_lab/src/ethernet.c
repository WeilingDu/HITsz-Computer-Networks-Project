#include "ethernet.h"
#include "utils.h"
#include "driver.h"
#include "arp.h"
#include "ip.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief 处理一个收到的数据包
 *        你需要判断以太网数据帧的协议类型，注意大小端转换
 *        如果是ARP协议数据包，则去掉以太网包头，发送到arp层处理arp_in()
 *        如果是IP协议数据包，则去掉以太网包头，发送到IP层处理ip_in()
 * 
 * @param buf 要处理的数据包
 */
void ethernet_in(buf_t *buf)
{
	uint16_t type;
	uint16_t *type_p;
	type_p = (uint16_t *)(buf->data + 12);   // 得到以太网数据帧的协议类型
	type = swap16(*type_p);  // 大小端转换
	buf_remove_header(buf, 14);  // 去掉以太网包头（长度14）
	printf("%d", type);
	switch (type)
	{
		// 如果是ARP协议数据包, 发送到arp层处理
		case 0x0806:
			arp_in(buf);
			break;
		// 如果是IP协议数据包, 发送到IP层处理
		case 0x0800:
			ip_in(buf);
			break;
		default:
			break;
	}
}

/**
 * @brief 处理一个要发送的数据包
 *        你需添加以太网包头，填写目的MAC地址、源MAC地址、协议类型
 *        添加完成后将以太网数据帧发送到驱动层
 * 
 * @param buf 要处理的数据包
 * @param mac 目标ip地址
 * @param protocol 上层协议
 */
void ethernet_out(buf_t *buf, const uint8_t *mac, net_protocol_t protocol)
{
	ether_hdr_t header;
	uint8_t *p, *q;
	uint8_t src_MAC[6] = DRIVER_IF_MAC;  // 设置源MAC地址，即本机的MAC地址
	int i = 0;
	for (i = 0; i < NET_MAC_LEN; i++)
	{
		header.src[i] = src_MAC[i];  // 设置包头的源MAC地址
		header.dest[i] = mac[i];  // 设置包头的目的MAC地址
	}
	header.protocol = swap16(protocol);  // 设置包头的协议类型（注意大小端转换）
	buf_add_header(buf, sizeof(header));  // 添加以太网包头
	for (i = 0, p = (uint8_t *)&header, q = buf->data; i < sizeof(header); i++)
	{
		q[i] = p[i];
	}
	driver_send(buf);

}

/**
 * @brief 初始化以太网协议
 * 
 * @return int 成功为0，失败为-1
 */
int ethernet_init()
{
	buf_init(&rxbuf, ETHERNET_MTU + sizeof(ether_hdr_t));
	return driver_open();
}

/**
 * @brief 一次以太网轮询
 * 
 */
void ethernet_poll()
{
	if (driver_recv(&rxbuf) > 0)
		ethernet_in(&rxbuf);
}
