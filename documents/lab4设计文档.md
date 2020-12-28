# 实验四设计文档

## 1. arp_req 函数

该函数功能：发送一个ARP请求报文。

当某一个主机要发送报文时，若在ARP缓存中找不到对方主机的硬件地址，则需要发送ARP 请求报文（目的是为了获得对方主机的硬件地址）。

1. 调用buf_init对txbuf进行初始化，初始化的长度为ARP报文的长度。

   ```C
   	buf_t *buf;
   	buf = &txbuf;
   	buf_init(buf, sizeof(arp_pkt_t));  // 调用buf_init对txbuf进行初始化
   ```

2. 填写ARP报头。代码中有提供已经初始化的ARP报文，但初始化的ARP报文没有填写报文类型opcode和接收方ip地址target_ip。所以要将该报文的opcode设置为ARP_REQUEST，以及将target_ip设置为该函数的传入参数ip。

   ```c
   	// 填写ARP报头
   	arp_pkt_t packet;
   	packet = arp_init_pkt;
   	packet.opcode = swap16(ARP_REQUEST);  // 将ARP的opcode设置为ARP_REQUEST，注意大小端转换
   	// 填写ARP的目标ip
   	memcpy(packet.target_ip, target_ip, NET_IP_LEN);
   	memcpy(buf->data, &packet, sizeof(arp_pkt_t));
   ```

3. 调用ethernet_out()将ARP数据报发送到ethernet层。

   传入参数：对于ARP 请求帧，因为它是一个广播帧，所以要填上广播 MAC 地址（FF-FF-FF-FF-FF-FF），其目标主机是网络上的所有主机。上层协议为ARP协议。

   ```c
   	// 对于ARP请求帧，要填上广播MAC地址FF-FF-FF-FF-FF-FF
   	uint8_t dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
   	ethernet_out(buf, dest_mac, NET_PROTOCOL_ARP);
   ```

## 2. arp_out 函数

该函数功能：处理一个要发送的数据包。

1. 调用arp_lookup()根据传入参数目标ip地址来查找ARP表中是否有该ip地址对应的mac地址。

   * 若函数返回值不是NULL，说明可以找到该ip地址对应的mac地址，那么直接调用ethernet_out()将该数据报发送给ethernet层。传入参数：目的mac地址为在arp表中找到的mac地址，上层协议为arp_out()的传入参数protocol。
   * 若函数返回值为NULL，说明没有找到对应的MAC地址，转2

   ```c
   	// 根据IP地址来查找ARP表
   	uint8_t *dest_mac = arp_lookup(ip);  
   	
   	if (dest_mac != NULL)
   	{// 如果能找到该IP地址对应的MAC地址
   		ethernet_out(buf, dest_mac, protocol);  // 数据报直接发送给ethernet层
   		return;
   	}
   ```

2. 要调用arp_req()发送ARP请求报文，填入目标ip地址。并且，需要把该数据报缓存到arp_buf，等到arp_in()能收到ARP request报文的应答报文时，再将arp_buf中的数据报发送出去。

   缓存时，将该数据报复制到arp_buf的buf字段中；将valid字段设置为1；将protocol字段设置为传入参数protocol；将ip字段设置为传入参数ip。

   ```c
   	// 如果没有找到对应的MAC地址
   	// 将来自IP层的数据包缓存到arp_buf中
   	memcpy(&(arp_buf.buf), buf, sizeof(buf_t));
   	arp_buf.valid = 1;
   	arp_buf.protocol = protocol;
   	memcpy(arp_buf.ip, ip, NET_IP_LEN);
   	// 发送ARP request报文
   	arp_req(ip);
   	return;
   ```

## 3. arp_in 函数

函数功能：处理一个收到的数据包

1. 检查数据包前面的ARP报头部分。若硬件类型为以太网，协议类型为IP协议，硬件地址长度为6，协议地址长度为4，操作类型为请求或应答，则报头完整。

   ```c
   	arp_pkt_t *arp = (arp_pkt_t *)buf->data;
   	int opcode = swap16(arp->opcode);
   	// 报文不完整
   	if (arp->hw_type != swap16(ARP_HW_ETHER)
   		|| arp->pro_type != swap16(NET_PROTOCOL_IP)
   		|| arp->hw_len != NET_MAC_LEN
   		|| arp->pro_len != NET_IP_LEN
   		|| (opcode != ARP_REQUEST && opcode != ARP_REPLY)
   ```

2. 当收到一个数据包之后，调用arp_update更新ARP表项，传入参数为该arp报文的源ip地址和源mac地址，以及有效状态。

   ```c
   	arp_update(arp->sender_ip, arp->sender_mac, ARP_VALID);
   ```

3. 查看arp_buf的valid字段是否为1

   * 若为1，说明ARP分组队列里面有待发送的数据包，转4
   * 若不为1，说明ARP分组队列里面没有待发送的数据包。然后判断接收到的报文是否为request请求报文，并且，该请求报文的目的IP正好是本机的IP地址，若是，则回应应答报文，转5

4. 调用arp_lookup()根据arp_buf的目的ip地址字段找到对应的MAC地址，如果能找到，则调用ethernet_out()将该数据报发送给ethernet层。传入参数：目的mac地址为在arp表中找到的mac地址，上层协议为arp_buf中的protocol字段。

   **注意：这里发送完数据报之后，要把arp_buf中的valid字段置为无效。**

   ```c
   	if (arp_buf.valid == 1)
   	{// ARP分组队列里面有待发送的数据包
   		// 根据IP地址来查找ARP表
   		uint8_t *dest_mac = arp_lookup(arp_buf.ip);  
   		if (dest_mac != NULL)
   		{// 如果能找到该IP地址对应的MAC地址
   			ethernet_out(&(arp_buf.buf), dest_mac, arp_buf.protocol);  // 数据报直接发送给ethernet层
   			arp_buf.valid = 0;  // 注意，要将arp_buf置为无效
   			return;
   		}
   	}
   ```

5. 调用buf_init初始化一个buf，buf长度为arp报文长度。然后填写报文，将代码中初始化的ARP报文的opcode设置为ARP_REPLY，将target_ip设置为该数据包的sender_ip，将target_mac设置为该数据包的sender_mac(因为将要发送的数据报为对arp请求报文的回应)。

   然后调用ethernet_out()将该数据报发送给ethernet层。**注意：这里protocol传入参数为NET_PROTOCOL_ARP**

   ```c
   	// ARP分组队列里面没有待发送的数据包
   	if ((opcode == ARP_REQUEST) && (!memcmp(net_if_ip, arp->target_ip, NET_IP_LEN))) 
   	{// 该报文是请求本机MAC地址的ARP请求报文
   		// 调用buf_init初始化一个buf
   		buf_init(&rxbuf, sizeof(arp_pkt_t)); 
   		// 填写ARP报头
   		arp_pkt_t packet;
   		packet = arp_init_pkt;
   		packet.opcode = swap16(ARP_REPLY);
   		memcpy(packet.target_ip, arp->sender_ip, NET_IP_LEN);
   		memcpy(packet.target_mac, arp->sender_mac, NET_MAC_LEN);
   		memcpy(rxbuf.data, &packet, sizeof(arp_pkt_t));
   		ethernet_out(&rxbuf, arp->sender_mac, NET_PROTOCOL_ARP);
   	}
   ```

## 4. arp_update 函数

函数功能：更新ARP表

**该函数实现的关键在于表项的超时时间戳timeout字段的理解**：time_t是unix系统中距离1970年某个固定时间的秒数。可以通过c语言中的time(NULL)获得当前的系统时间，通过当前的系统时间减去该表项的timeout字段得到该表项已经存在了多长时间，若该值大于所允许的arp表过期时间，则说明该表项无效。当插入某个表项是，将timeout字段设置为系统时间。

1. 检测ARP表中所有的ARP表项是否有超时，如果有超时，则将该表项的状态改为无效。

   ```c
   	// 依次轮询检测ARP表中所有的ARP表项是否有超时
       for (int i = 0; i < ARP_MAX_ENTRY; i++)
   	{
   		if ((time(NULL) - arp_table[i].timeout) > ARP_TIMEOUT_SEC)
   		{// 该ARP表项已超时
   			arp_table[i].state = ARP_INVALID;  // 将该表项的状态改为无效
   		}
   	}
   ```

2. 检测ARP表是否有无效的表项，若有则将传入参数新的IP、MAC信息插入到表中，并记录超时时间，更改表项的状态为有效。若无，转3。

   ```c
   	// 检测ARP表是否有无效的表项
   	for (int i = 0; i < ARP_MAX_ENTRY; i++)
   	{
   		if (arp_table[i].state == ARP_INVALID)
   		{// ARP表有无效的表项
   			memcpy(arp_table[i].ip, ip, NET_IP_LEN);
   			memcpy(arp_table[i].mac, mac, NET_MAC_LEN);
   			arp_table[i].timeout = time(NULL);
   			arp_table[i].state = ARP_VALID;  // 将该表项的状态改为有效
   			return;
   		}
   	}
   ```

3. 遍历ARP表找到无效的表项中timeout最大的表项（因为前面已经将超时的表项设置为无效），记录下标。然后更新该表项。

   ```c
   	time_t timeout_max = 0;
   	int index;
   	// 找到超时时间最长的一条表项, 表项下标为index
   	for (int i = 0; i < ARP_MAX_ENTRY; i++)
   	{
   		if (arp_table[i].state == ARP_INVALID)
   		{
   			if (arp_table[i].timeout > timeout_max)
   			{
   				timeout_max = arp_table[i].timeout;
   				index = i;
   			}
   		}
   	}
   	memcpy(arp_table[index].ip, ip, NET_IP_LEN);
   	memcpy(arp_table[index].mac, mac, NET_MAC_LEN);
   	arp_table[index].timeout = time(NULL);
   	arp_table[index].state = ARP_VALID;  // 将该表项的状态改为有效
   	return;
   ```

## 5. 运行截图

![](https://tva1.sinaimg.cn/large/0081Kckwly1glquhz98sjj31di0rc0yv.jpg)



