#include "icmp.h"
#include "ip.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief 处理一个收到的数据包
 *        你首先要检查buf长度是否小于icmp头部长度
 *        接着，查看该报文的ICMP类型是否为回显请求，
 *        如果是，则回送一个回显应答（ping应答），需要自行封装应答包。
 * 
 *        应答包封装如下：
 *        首先调用buf_init()函数初始化txbuf，然后封装报头和数据，
 *        数据部分可以拷贝来自接收到的回显请求报文中的数据。
 *        最后将封装好的ICMP报文发送到IP层。  
 * 
 * @param buf 要处理的数据包
 * @param src_ip 源ip地址
 */
void icmp_in(buf_t *buf, uint8_t *src_ip)
{
    if (buf->len < 8)
    {// 如果buf长度小于icmp头部长度（8字节），不处理
        return;
    }
    // 报文检查正确，继续
    icmp_hdr_t *icmp = (icmp_hdr_t *)buf->data;
    if ((icmp->type == ICMP_TYPE_ECHO_REQUEST) && (icmp->code == 0))
    {// 如果该报文的ICMP类型为回显请求
        // 检测校验和
        uint16_t hdr_checksum = swap16(icmp->checksum);  // 缓存头部校验和字段（注意大小端转换）
        icmp->checksum = 0;  // 将校验和字段清零
        if (hdr_checksum != checksum16((uint16_t *)buf->data, buf->len))  // 整个icmp数据报都需要检测
        {// 计算的结果与之前缓存的校验和不一致, 不处理
            return;
        }

        // 校验和正确，继续
        buf_init(&txbuf, buf->len);  // 调用buf_init()函数初始化txbuf
        memcpy(txbuf.data, buf->data, buf->len);  // 数据部分拷贝来自接收到的回显请求报文中的数据
        
        // 填写icmp头部
        icmp_hdr_t header;
        header.type = ICMP_TYPE_ECHO_REPLY;
        header.code = 0;
        header.checksum = swap16(0);
        header.id = icmp->id;
        header.seq = icmp->seq;

        // 添加icmp头部
        memcpy(txbuf.data, &header, sizeof(icmp_hdr_t));
        // 计算校验和
        header.checksum = swap16(checksum16((uint16_t *)txbuf.data, txbuf.len));
        memcpy(txbuf.data, &header, sizeof(icmp_hdr_t));

        // 将封装好的ICMP报文发送到IP层
        ip_out(&txbuf, src_ip, NET_PROTOCOL_ICMP);
        return;
    }
    

    
}

/**
 * @brief 发送icmp不可达
 *        你需要首先调用buf_init初始化buf，长度为ICMP头部 + IP头部 + 原始IP数据报中的前8字节 
 *        填写ICMP报头首部，类型值为目的不可达
 *        填写校验和
 *        将封装好的ICMP数据报发送到IP层。
 * 
 * @param recv_buf 收到的ip数据包
 * @param src_ip 源ip地址
 * @param code icmp code，协议不可达或端口不可达
 */
void icmp_unreachable(buf_t *recv_buf, uint8_t *src_ip, icmp_code_t code)
{
    buf_t buf;
    // 先初始化长度为IP头部 + 原始IP数据报中的前8字节的buf
    buf_init(&buf, sizeof(ip_hdr_t) + 8);
    // 将IP头部和原始IP数据报中的前8字节拷贝到buf中
    memcpy(buf.data, recv_buf->data, sizeof(ip_hdr_t) + 8);
    // 增加icmp头部缓存空间  
    buf_add_header(&buf, sizeof(icmp_hdr_t));
    icmp_hdr_t header;
    header.type = ICMP_TYPE_UNREACH;
    header.code = code;
    header.id = swap16(0);  // 必须为0
    header.seq = swap16(0);  // 必须为0
    // 填写校验和
    header.checksum = swap16(0);
    memcpy(buf.data, &header, sizeof(icmp_hdr_t));
    header.checksum = swap16(checksum16((uint16_t *)buf.data, buf.len));
    memcpy(buf.data, &header, sizeof(icmp_hdr_t));
    ip_out(&buf, src_ip, NET_PROTOCOL_ICMP);
    return;
}