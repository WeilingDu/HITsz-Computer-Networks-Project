# 实验六设计文档

## 1. icmp_in 函数

函数功能：处理一个收到的数据包buf

1. 报文检查。

   * 报文长度：如果该数据包的长度小于icmp头部长度（8字节），则不处理。
   * 报文类型：报文是否为回显请求（type为8，code为0），若不是，则不处理。
   * 头部校验和字段：先缓存头部校验和，然后将该数据的校验和字段清零，再调用checksum16计算校验和**（注意：这里的计算长度为整个数据包的长度**），看是否和之前缓存的头部校验和相等。若不相等，则不处理。

   以上检查均通过之后，转2。

2. 回送一个回显应答（ping应答）

   * 调用buf_init()函数初始化txbuf，初始化长度为收到的数据包buf的长度。将buf的数据拷贝到txbuf中（因为数据部分可以拷贝来自接收到的回显请求报文中的数据，之后再替换icmp头部）。

   * 填写icmp头部。type为0，code为0，校验和字段为0，标识和序列号和收到的回显请求报头的值相同。然后调用checksum16计算校验和（这里的计算长度为整个数据包的长度），得到的结果填入校验和字段。

     填写完头部之后复制到txbuf的的前8个字节。

   * 调用ip_out()将封装好的ICMP报文发送到IP层，传入参数上层协议为icmp协议。

```c
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
```



## 2. icmp_unreachable 函数

函数功能：发送icmp不可达报文。

1. 调用buf_init()函数初始化一个buf，初始化长度为IP头部 + 原始IP数据报中的前8字节的buf。然后将IP头部和原始IP数据报中的前8字节拷贝到buf中。

   ```c
   		buf_t buf;
       // 先初始化长度为IP头部 + 原始IP数据报中的前8字节的buf
       buf_init(&buf, sizeof(ip_hdr_t) + 8);
       // 将IP头部和原始IP数据报中的前8字节拷贝到buf中
       memcpy(buf.data, recv_buf->data, sizeof(ip_hdr_t) + 8);
   ```

2. 填写ICMP报头首部。type为3表示目的不可达；code为传入参数；**标识和序列号必须为0。**

   将校验和字段填写为0。然后调用buf_add_header()对buf增加icmp头部缓存空间。将填写完的ICMP报头首部赋值到buf中。然后调用checksum16计算校验和（这里的计算长度为整个数据包的长度），将得到的结果填入ICMP报头的校验和字段。

   ```c
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
   ```

3. 调用ip_out()将封装好的ICMP报文发送到IP层，传入参数上层协议为icmp协议。

   ```c
   	ip_out(&buf, src_ip, NET_PROTOCOL_ICMP);
   ```

## 3. 运行截图

![](https://tva1.sinaimg.cn/large/0081Kckwgy1glqy5llpnbj31eo0t044y.jpg)

