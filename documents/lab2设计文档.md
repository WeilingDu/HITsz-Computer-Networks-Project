# 实验二设计文档

## 实验分析

### ICMP

1. 当MAC帧的类型字段=0x0800时，说明本帧的数据部分为IPv4类型。IPv4头部中的proto字段=1时，说明为ICMP报文格式。

   ```c
   ip_head = ethernet_head + 14;
   p = ip_head + 12;
   proto = (ip_head + 9)[0];
   ```

2. 32-39为ICMP头部

   * 32 ：Type字段，说明 ICMP 报文的作用及格式。

   * 33 ：Code字段，说明某种 ICMP 报文的类型。

     ![](https://tva1.sinaimg.cn/large/0081Kckwgy1gl9qlc8n08j316m0hc42u.jpg)

     根据Type和Code的值分析ICMP的消息类型。

   * 34-35 ：校验和

   * 36-37 ：标识符

     ```c
     int be = (p[0] << 8) + p[1];
     printf("Identifier(BE):%d(0x%02x%02x)\n",be,p[0],p[1]);
     int le = (p[1] << 8) + p[0];
     printf("Identifier(LE):%d(0x%02x%02x)\n",le,p[1],p[0]);
     ```

   * 38-39 ：序列号

     ```c
     be = (p[0] << 8) + p[1];
     printf("Sequence number(BE):%d(0x%02x%02x)\n",be,p[0],p[1]);
     le = (p[1] << 8) + p[0];
     printf("Sequence number(LE):%d(0x%02x%02x)\n\n",le,p[1],p[0]);
     ```

     

### ARP

1. 当MAC帧的类型字段=0x0806时，说明本帧的数据部分为ARP报文，然后对MAC帧进行解析。

   * 0-5字节为目的MAC地址。
   * 6-11字节为源MAC地址。
   * 12-13字节为帧类型，等于ARP的协议号0x0806。

2. 14-41字节为ARP报文，确定为ARP请求帧之后，对该部分报文做进一步解析。其中14-21为ARP报头。

   * 14-15 ：硬件类型。表示 ARP 报文可以在哪种类型的网络上传输，值为1时表示 为以太网地址。（在代码中体现为hardware_type指针）
   * 16-17 ：上层协议类型。表示硬件地址要映射的协议地址类型，映射 IP 地址时的 值为 0x0800。（在代码中体现为protocol_type指针）
   * 18 ：标识 MAC 地址长度，为6。
   * 19 ：标识 IP 地址长度，为4。
   * 20-21 ：操作类型，指定本次 ARP 报文类型。2标识 ARP 请求报文，2标识 ARP 应答报文。（在代码中体现为temp = protocol_type + 4）

3. 剩下的是源MAC和源IP与目的MAC和目的IP的解析

   * 22-27 ：源 MAC 地址，标识发送设备的硬件地址。
* 28-31 ：源 IP 地址，标识发送方设备的 IP 地址。
   * 32-37 ：目的 MAC 地址，表示接收方设备的硬件地址，在请求报文中该字段值全为0，表示任意地址。
* 38-41 ：目的 IP 地址，表示接受方的 IP 地址。

## 程序运行截图

<img src="https://tva1.sinaimg.cn/large/0081Kckwgy1gl9qvslv01j30e00f8mz2.jpg" style="zoom:50%;" />

<img src="https://tva1.sinaimg.cn/large/0081Kckwgy1gl9rcomfnbj30h60jugoh.jpg" style="zoom:50%;" />



