#ifndef CONFIG_H
#define CONFIG_H

#define DRIVER_IF_NAME "enp0s5" //使用的物理网卡名称
#define DRIVER_IF_IP      \
    {                     \
        10, 211, 55, 88\
    } //自定义网卡ip地址
    // 前三位为网卡号
    // 最后一位为主机号
    // 虚拟机IP为10.211.55.8
    // 实验九中改为10.211.55.88
    // 192.168.163.103
#define DRIVER_IF_MAC                      \
    {                                      \
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66 \
    }                     //自定义网卡mac地址


#define ETHERNET_MTU 1500 //以太网最大传输单元

#define ARP_MAX_ENTRY 16       //arp表最大长度
#define ARP_TIMEOUT_SEC 60 * 5 //arp表过期时间
#define ARP_MIN_INTERVAL 1     //向相同地址发送arp请求的最小间隔

#define IP_DEFALUT_TTL 64 //IP默认TTL

#define UDP_MAX_HANDLER 16 //最多的UDP处理程序数

#endif