#include "ip.h"
#include "arp.h"
#include "icmp.h"
#include "udp.h"
#include <string.h>
// #include <config.h>
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
    // TODO 
    ip_hdr_t *ip_head = (ip_hdr_t *) buf -> data;
    uint16_t checksum_old = swap16(ip_head -> hdr_checksum);
    uint16_t total_len = swap16(ip_head -> total_len);
    uint16_t *p = buf -> data;
    uint8_t head_len = (ip_head -> hdr_len) * 4;
    uint8_t version = ip_head -> version;

    // inspection
    ip_head -> hdr_checksum = 0;
    if(
        version == IP_VERSION_4
        && head_len <= 60
        && head_len >= 20
        && total_len <= 65535
        && checksum_old == checksum16(p,head_len)
        && memcmp(ip_head->dest_ip,net_if_ip,4) == 0
    ){
        switch (ip_head -> protocol)
        {
        case NET_PROTOCOL_UDP:
            buf_remove_header(buf,head_len);
            udp_in(buf,ip_head -> src_ip);
            break;
        case NET_PROTOCOL_ICMP:
            buf_remove_header(buf,head_len);
            icmp_in(buf,ip_head->src_ip);
            break;
        default:
            ip_head -> hdr_checksum = swap16(checksum_old);
            icmp_unreachable(buf,ip_head->src_ip,ICMP_CODE_PROTOCOL_UNREACH);
            break;
        }
    }
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
    // TODO
    int i;
    uint16_t *p;
    buf_add_header(buf,20);
    p = buf -> data;
    ip_hdr_t *ip_head = (ip_hdr_t *) buf -> data;
    ip_head -> version = IP_VERSION_4;
    ip_head -> hdr_len = 20 / 4;
    ip_head -> tos = 0;
    if(mf == 1)
    {
        ip_head -> total_len = swap16(ETHERNET_MTU);
    }
    else
    {
        ip_head -> total_len = swap16(buf->len);
    }
    ip_head -> id = swap16(id);
    ip_head -> flags_fragment = swap16((mf << 13) + (offset / 8));
    ip_head -> ttl = 64;
    ip_head -> protocol = protocol;
    for(i=0;i<4;i++)
    {
        (ip_head -> src_ip)[i] = net_if_ip[i];
        (ip_head -> dest_ip)[i] = ip[i];
    }
    ip_head -> hdr_checksum = 0;
    ip_head -> hdr_checksum = swap16(checksum16(p,20));
    arp_out(buf,ip,NET_PROTOCOL_IP);
}

/**
 * @brief 处理一个要发送的数据包
 *        你首先需要检查需要发送的IP数据报是否大于以太网帧的最大包长（1500字节 - ip包头长度）。
 *        
 *        如果超过，则需要分片发送。 
 *        分片步骤：
 *        （1）调用buf_init()函数初始化buf，长度为以太网帧的最大包长（1500字节 - ip包头头长度）
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
int IP_ID = 0;
void ip_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol)
{
    // TODO 
    uint16_t offset,cur_id;
    offset = 0;
    cur_id = IP_ID;
    while ((buf -> len) - 20 > ETHERNET_MTU)
    {
        buf_init(&txbuf,ETHERNET_MTU - 20);
        memcpy(txbuf.data,buf->data,buf->len);
        ip_fragment_out(&txbuf,ip,protocol,cur_id,offset,1);
        buf_remove_header(buf,ETHERNET_MTU - 20);
        offset = offset + ETHERNET_MTU - 20;
    }
    buf_init(&txbuf,buf->len);
    txbuf.data = buf -> data;
    ip_fragment_out(&txbuf,ip,protocol,cur_id,offset,0);
    IP_ID++;
}