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
int ICMP_ID = 1;
void icmp_in(buf_t *buf, uint8_t *src_ip)
{
    // TODO
    icmp_hdr_t *icmp_head = (icmp_hdr_t *) buf -> data;
    icmp_hdr_t *ans_head;
    uint16_t cur_id = ICMP_ID;
    uint16_t *p;
    if(
        icmp_head->type == ICMP_TYPE_ECHO_REQUEST 
        && icmp_head->code == 0
        && buf->len >= 20
    )
    {
        
        buf_init(&txbuf,buf->len);
        memcpy(txbuf.data+8,buf->data+8,buf->len);
        ans_head = (icmp_hdr_t *) txbuf.data;
        ans_head -> type = ICMP_TYPE_ECHO_REPLY;
        ans_head -> code = 0;
        ans_head -> id = swap16(cur_id);
        ans_head -> seq = swap16(cur_id);
        p = txbuf.data;
        ans_head -> checksum = 0;
        ans_head -> checksum = swap16(checksum16(p,txbuf.len));
        ip_out(&txbuf,src_ip,NET_PROTOCOL_ICMP);
        ICMP_ID++;
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
    // TODO
    buf_init(&txbuf,8+20+8);
    memcpy(txbuf.data+8,recv_buf->data,20+8);
    uint16_t *p = txbuf.data;
    icmp_hdr_t *icmp_head = (icmp_hdr_t *) txbuf.data;
    
    icmp_head -> type = ICMP_TYPE_UNREACH;
    icmp_head -> code = code;
    icmp_head -> id = 0;
    icmp_head -> seq = 0;
    icmp_head -> checksum = 0;
    icmp_head -> checksum = swap16(checksum16(p,8+20+8));
    ip_out(&txbuf,src_ip,NET_PROTOCOL_ICMP);
}