#include <stdio.h>
#include <string.h>
#include <time.h>
#include "net.h"
#include "udp.h"

void handler(udp_entry_t *entry, uint8_t *src_ip, uint16_t src_port, buf_t *buf)
{
    printf("recv udp packet from %s:%d len=%d\n", iptos(src_ip), src_port, buf->len);
    for (int i = 0; i < buf->len; i++)
        putchar(buf->data[i]);
    putchar('\n');
    uint16_t len = 1800;
    //uint16_t len = 1000;
    uint8_t data[len];

    uint16_t dest_port = 60001;
    for (int i = 0; i < len; i++)
        data[i] = i;
    udp_send(data, len, 60000, src_ip, dest_port); //发送udp包
}
int main(int argc, char const *argv[])
{

    net_init();               //初始化协议栈
    udp_open(60000, handler); //注册端口的udp监听回调

    while (1)
    {
        net_poll(); //一次主循环
    }

    return 0;
}
