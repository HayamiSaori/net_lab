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
    // TODO
    uint8_t *eth_head = buf -> data;
    uint16_t protocol = ((eth_head + 12)[0] << 8) + (eth_head + 12)[1];
    if(protocol == NET_PROTOCOL_IP)
    {
        buf_remove_header(buf,14);
        ip_in(buf);
    }
    else if(protocol == NET_PROTOCOL_ARP)
    {
        buf_remove_header(buf,14);
        arp_in(buf);
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
    // TODO
    buf_add_header(buf,14);
    uint8_t *eth_head = buf -> data;
    uint8_t i;
    for(i=0;i<6;i++)
    {
        eth_head[i] = mac[i];
        eth_head[i + 6] = net_if_mac[i];
    }
    switch (protocol)
    {
    case NET_PROTOCOL_ICMP:
    case NET_PROTOCOL_TCP:
    case NET_PROTOCOL_UDP:
    case NET_PROTOCOL_IP:
        printf("\n---IP---\n");
        eth_head[12] = 0x08;
        eth_head[13] = 0x00;
        break;
    case NET_PROTOCOL_ARP:
        printf("\n---ARP---\n");
        eth_head[12] = 0x08;
        eth_head[13] = 0x06;
        break;
    default:
        eth_head[12] = protocol >> 8 & 0xff;
        eth_head[13] = protocol & 0xff;
        break;
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
