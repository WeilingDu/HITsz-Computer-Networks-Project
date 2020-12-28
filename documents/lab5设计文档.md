# 实验五设计文档

## 1. ip_in 函数

函数功能：处理一个收到的数据包

1. 报头检查。若版本号为ipv4，且总长度不大于65535，且首部长度等于20，那么数据包完整。转2。

   ```c
   	ip_hdr_t *ip = (ip_hdr_t *)buf->data;
   	// 报头检查
   	if (ip->version != IP_VERSION_4
   		|| ip->total_len > swap16(65535)
   		|| (ip->hdr_len * IP_HDR_LEN_PER_BYTE) != 20
   		)
   	{
   		return;
   	}
   ```

2. 检验头部校验和。注意要缓存头部校验和字段（注意大小端转换）。然后将数据包中的校验和字段清0，然后调用checksum16函数来对该数据包的**前20字节**计算校验和。若计算的结果与之前缓存的校验和不一致, 不处理该数据包；若一致，则检验该数据包的目的ip是否是本机ip。若不是，不处理该数据包；若是，则转3。

   ```c
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
   ```

   注意：这里要把缓存的校验和字段赋值回去，因为若是该程序不支持的协议，会调用icmp_unreachable()回送一个ICMP协议不可达的报文，此时需要把ip报头也发送回去。

3. 检查IP报头的协议字段。如果是ICMP协议，则去掉IP头部，发送给ICMP协议层处理；如果是UDP协议，则去掉IP头部，发送给UDP协议层处理

   ```c
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
   ```

## 2. checksum() 校验和算法

函数功能：计算16位校验和。

传入参数：要计算的数据包的数据起始地址，计算长度（单位为字节）

将数据看成16位为单位的数字组成，依次进行二进制求和。每一次求和后要将加法过程中产生的进位（最高位的进位）加到低 16 位：即为将高 16 位与低 16 位相加，然后将该次加法最高位产生的进位加到低 16 位。最后将上述的和取反，即得到校验和。

注意：

* 求和时应将最高位的进位保存，所以加法应采用 32 位加法，即保存累加和的数据类型应该是32bit
* 二进制求和时，要将数据包的数据进行大小端转换。

```c
uint16_t checksum16(uint16_t *buf, int len)
{
    uint32_t cksum = 0;  // 加法应采用 32 位加法
    
    // 16 位为单位数字相加
    while(len > 1){
        cksum += (((*buf)&0xFF) << 8) | (((*buf) >> 8) & 0xFF);  // 注意大小端转换！
        buf++;  // 指针前移16位
        len -= sizeof(uint16_t);  // 减去2字节

        // 高位有进位，进位到低位
        cksum = (cksum >> 16) + (cksum & 0xffff);  // 将高 16 位与低 16 位相加
        cksum += (cksum >> 16);  // 把该次加法最高位产生的进位加到低 16 位
    }

    // 如果长度是奇数
    if (len != 0)
    {
        cksum += *((uint8_t *)buf);
        // 高位有进位，进位到低位
        cksum = (cksum >> 16) + (cksum & 0xffff);  // 将高 16 位与低 16 位相加
        cksum += (cksum >> 16);  // 把该次加法最高位产生的进位加到低 16 位
    }
    
    // 将上述的和取反
    return ~(uint16_t)cksum;
        
}
```

## 3. ip_out 函数

函数功能：IP 数据报输出处理。

**实现该函数的分片部分有两个关键点：**

* 分片的标识字段：在该函数中设置静态局部变量初始化id为0，之后每次发送一个数据报就将该值加一（同一个分片的id值是一样的）。

  ```c
  	// 初始化标识
  	static uint16_t ip_id = 0;
  ```

* 分片的位偏移字段：将某个数据报分片之前，设置已传输的数据值为0，当发送分片时，片偏移字段为（已传输的数据值 / ip分片偏移长度单位）

1. 检查需要发送的数据长度是否大于以太网帧的最大包长（1500-20=1480字节）。如果超过，则需要分片发送，转2；如果没有超过，不用分片发送，转3。

2. 分片步骤

   * 若要发送的数据包buf的长度大于1480，则调用buf_init()函数初始化rxbuf，长度为1480字节。将要发送的数据报的1480字节部分复制到rxbuf中，然后调用ip_fragment_out()发送该数据，mf标志为1。并截断数据报buf的前1480字节。
   * 最后的一个分片小于或等于以太网帧的最大包长时。调用buf_init()函数初始化rxbuf，长度为剩下未分片的数据长度。调用ip_fragment_out()发送该数据，mf标志为0。

   ```c
   	uint16_t len_sum = 0;
   
   	while (buf->len > 1480)
   	{
   		buf_init(&rxbuf, 1480);  // 初始化txbuf，长度为以太网帧的最大包长（1500字节 - ip包头长度）
   		memcpy(rxbuf.data, buf->data, 1480);
   		ip_fragment_out(&rxbuf, ip, protocol, ip_id, len_sum/IP_HDR_OFFSET_PER_BYTE, 1);
   		buf_remove_header(buf, 1480);
   		len_sum += 1480;
   	}
   	// 最后的一个分片小于或等于以太网帧的最大包长时
   	buf_init(&rxbuf, buf->len);
   	memcpy(rxbuf.data, buf->data, buf->len);
   	ip_fragment_out(&rxbuf, ip, protocol, ip_id, len_sum/IP_HDR_OFFSET_PER_BYTE, 0);  // 最后一个分片的MF = 0
   	ip_id += 1;
   	return;
   ```

3. 调用ip_fragment_out()发送该数据包buf，传入的片偏移参数和mf参数均未0。

   ```c
   	if (buf->len <= ETHERNET_MTU-20)
   	{// 发送的IP数据包的数据部分没有超过以太网帧的最大包长（1500字节 - ip包头长度）, 不用分片
   		ip_fragment_out(buf, ip, protocol, ip_id, 0, 0);
   		ip_id += 1;
   		return;
   	}
   ```

## 4. ip_fragment_out 函数

函数功能：处理一个要发送的分片。

1. 填写IP数据报头部字段。版本号为IPv4；首都长度为5（因为P包头长度为20字节，该字段的单位为4个字节）；服务类型为0；总长度为要发送的分片的长度+ip报头的长度）；存活时间为IP默认TTL；标识符为传入参数id；协议字段为传入参数上层协议；源 IP 地址为本机ip地址；目的 IP 地址为传入参数ip地址。

   **重点在于标志与分段字段flags_fragment的填写和首部校验和hdr_checksum的填写。**

   * **flags_fragment**（第1个bit忽略，第2个bit为0表示允许分片，第3个bit为mf，后13个bit为片偏移offset）

     如果传入参数mf为1，说明有更多片段，那么flags_fragment字段为swap16(0x2000 | offset)。

     如果传入参数mf为0，说明该片段已经是最后一个分片，那么flags_fragment字段为swap16(offset)。

   * **hdr_checksum**

     先将该字段填0。调用buf_add_header()给buf增加IP数据报头部缓存空间，然后把报头填到要发送的数据buf中。调用checksum16函数来对该数据包的**前20字节**计算校验和，并填到头部的校验和字段中。

2. 调用arp_out()将封装后的IP数据报发送到arp层，传入参数上层协议为ip协议。

   ```c
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
   ```

## 5. 运行截图

* IP 包收发

  ![](https://tva1.sinaimg.cn/large/0081Kckwgy1glqxbliwqkj31di0rc7an.jpg)

* IP 分片

  ![](https://tva1.sinaimg.cn/large/0081Kckwgy1glqxc61zpgj31fg050dh3.jpg)