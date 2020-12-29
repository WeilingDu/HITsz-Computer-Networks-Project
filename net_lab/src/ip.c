#include "ip.h"
#include "arp.h"
#include "icmp.h"
#include "udp.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>

/**
 * @brief 处理一个收到的数据包
 *        你首先需要做报头检查，检查项包括：版本号、总长度、首部长度等。
 * 
 *        接着，计算头部校验和，注意：需要先把头部校验和字段缓存起来，再将校验和字段清零，
 *        调用checksum16()函数计算头部检验和，比较计算的结果与之前缓存的校验和是否一致，
 *        如果不一致，则不处理该数据报。
 * 
 *        检查收到的数据包的目的IP地址是否为本机的IP地址，只处理目的IP为本机的数据报。
 * 
 *        检查IP报头的协议字段：
 *        如果是ICMP协议，则去掉IP头部，发送给ICMP协议层处理
 *        如果是UDP协议，则去掉IP头部，发送给UDP协议层处理
 *        如果是本实验中不支持的其他协议，则需要调用icmp_unreachable()函数回送一个ICMP协议不可达的报文。
 *          
 * @param buf 要处理的包
 */
void ip_in(buf_t *buf)
{
	ip_hdr_t *ip = (ip_hdr_t *)buf->data;
	// 报头检查
	if (ip->version != IP_VERSION_4
		|| ip->total_len > swap16(65535)
		|| (ip->hdr_len * IP_HDR_LEN_PER_BYTE) != 20
		)
	{
		return;
	}
	uint16_t hdr_checksum = swap16(ip->hdr_checksum);  // 缓存头部校验和字段（注意大小端转换）
	ip->hdr_checksum = 0;  // 将校验和字段清零
	if (hdr_checksum != checksum16((uint16_t *)buf->data, 20))  // ip头部是20字节
	{// 计算的结果与之前缓存的校验和不一致, 不处理
		return;
	}
	if (memcmp(net_if_ip, ip->dest_ip, NET_IP_LEN) != 0)
	{// 目的IP不是本机IP, 不处理
		return;
	}
	// 注意：要把缓存的校验和字段赋值回去
	ip->hdr_checksum = swap16(hdr_checksum);

	// 检查IP报头的协议字段
	uint8_t protocol;
	protocol = ip->protocol;
	switch (protocol)
	{
	case NET_PROTOCOL_ICMP:
		buf_remove_header(buf, 20);  // 去掉IP报头，20字节
		icmp_in(buf, ip->src_ip);  // 发送给ICMP协议层处理
		break;
	case NET_PROTOCOL_UDP:
		buf_remove_header(buf, 20);  // 去掉IP报头，20字节
		udp_in(buf, ip->src_ip);  // 发送给UDP协议层处理
		break;
	default:
		// 注意：这里不用去除IP报头
		icmp_unreachable(buf, ip->src_ip, ICMP_CODE_PROTOCOL_UNREACH);  // 发送ICMP协议不可达的报文
		break;
	}
	return;
}

/**
 * @brief 处理一个要发送的分片
 *        你需要调用buf_add_header增加IP数据报头部缓存空间。
 *        填写IP数据报头部字段。
 *        将checksum字段填0，再调用checksum16()函数计算校验和，并将计算后的结果填写到checksum字段中。
 *        将封装后的IP数据报发送到arp层。
 * 
 * @param buf 要发送的分片
 * @param ip 目标ip地址
 * @param protocol 上层协议
 * @param id 数据包id
 * @param offset 分片offset，必须被8整除
 * @param mf 分片mf标志，是否有下一个分片
 */
void ip_fragment_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol, int id, uint16_t offset, int mf)
{
	ip_hdr_t header;
    
	// 填写IP数据报头部字段
    header.version = IP_VERSION_4;  // 版本号
	header.hdr_len = 5;  // IP包头长度为20字节，该字段的单位为4个字节
	header.tos = 0;  // 服务类型
	header.total_len = swap16(buf->len + sizeof(ip_hdr_t));  // 总长度
	header.id = swap16(id);  // 标识符
	if (mf)
	{// 如果有更多片段
		header.flags_fragment = swap16(0x2000 | offset);
	}else
	{
		header.flags_fragment = swap16(offset);
	}
	header.hdr_checksum = swap16(0);  // 将checksum字段填0
	header.ttl = IP_DEFALUT_TTL;  // 存活时间
	header.protocol = protocol;  // 上层协议
	memcpy(header.src_ip, net_if_ip, NET_IP_LEN);  // 源IP
    memcpy(header.dest_ip, ip, NET_IP_LEN);  // 目标ip
    
	buf_add_header(buf, sizeof(ip_hdr_t));  // 增加IP数据报头部缓存空间
	memcpy(buf->data, &header, sizeof(ip_hdr_t));  // 将填充完的头部数据填到buf中
    header.hdr_checksum = swap16(checksum16((uint16_t *)buf->data, 20));  // 注意大小端转换
	memcpy(buf->data, &header, sizeof(ip_hdr_t));  // 将填充完的头部数据填到buf中
    arp_out(buf, ip, NET_PROTOCOL_IP);  // 将封装后的IP数据报发送到arp层
	return;
}

/**
 * @brief 处理一个要发送的数据包
 *        你首先需要检查需要发送的IP数据报是否大于以太网帧的最大包长（1500字节 - ip包头长度）。
 *        
 *        如果超过，则需要分片发送。 
 *        分片步骤：
 *        （1）调用buf_init()函数初始化buf，长度为以太网帧的最大包长（1500字节 - ip包头长度）
 *        （2）将数据报截断，每个截断后的包长度 = 以太网帧的最大包长，调用ip_fragment_out()函数发送出去
 *        （3）如果截断后最后的一个分片小于或等于以太网帧的最大包长，
 *             调用buf_init()函数初始化buf，长度为该分片大小，再调用ip_fragment_out()函数发送出去
 *             注意：最后一个分片的MF = 0
 *    
 *        如果没有超过以太网帧的最大包长，则直接调用调用ip_fragment_out()函数发送出去。
 * 
 * @param buf 要处理的包
 * @param ip 目标ip地址
 * @param protocol 上层协议
 */
void ip_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol)
{
	// 初始化标识
	static uint16_t ip_id = 0;

	if (buf->len <= ETHERNET_MTU-20)
	{// 发送的IP数据包的数据部分没有超过以太网帧的最大包长（1500字节 - ip包头长度）, 不用分片
		ip_fragment_out(buf, ip, protocol, ip_id, 0, 0);
		ip_id += 1;
		return;
	}

	uint16_t len_sum = 0;

	buf_t send_buf;
	while (buf->len > 1480)
	{
		buf_init(&send_buf, 1480);  // 初始化buf，长度为以太网帧的最大包长（1500字节 - ip包头长度）
		memcpy(send_buf.data, buf->data, 1480);
		ip_fragment_out(&send_buf, ip, protocol, ip_id, len_sum/IP_HDR_OFFSET_PER_BYTE, 1);
		buf_remove_header(buf, 1480);
		len_sum += 1480;
	}
	// 最后的一个分片小于或等于以太网帧的最大包长时
	buf_init(&send_buf, buf->len);
	memcpy(send_buf.data, buf->data, buf->len);
	ip_fragment_out(&send_buf, ip, protocol, ip_id, len_sum/IP_HDR_OFFSET_PER_BYTE, 0);  // 最后一个分片的MF = 0
	ip_id += 1;
	return;
}
