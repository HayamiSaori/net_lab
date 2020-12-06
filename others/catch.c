#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#define BUFFER_MAX 2048
int main(int argc,char* argv[])
{
    int fd,proto,eth_type,str_len;
    int icmp_type,arp_type;
    char buffer[BUFFER_MAX];
    char *ethernet_head;
    char *ip_head;
    char *arp_head;
    unsigned char *p;

    if((fd=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL))) < 0)
    {
        printf("Error to create raw socket\n");
        return 0;
    }

    while (1)
    {
        str_len = recvfrom(fd,buffer,BUFFER_MAX,0,NULL,NULL);
        if(str_len < 42)
        {
            printf("Error to recieve message\n");
            return 0;
        }

        ethernet_head = buffer;
        p = ethernet_head;
        printf("-------------------------------------------\n");
        printf("Src MAC address: %.2x:%02x:%02x:%02x:%02x:%02x\n",
                p[6],p[7],p[8],p[9],p[10],p[11]);
        printf("Dst MAC address: %.2x:%02x:%02x:%02x:%02x:%02x\n",
                p[0],p[1],p[2],p[3],p[4],p[5]);
        ip_head = ethernet_head + 14;

        eth_type = ((ethernet_head + 12)[0] << 8)
                    + (ethernet_head + 12)[1];
        // printf("eth_type:0x%x\n",eth_type);
        if(eth_type == ETH_P_IP)
        {
            p = ip_head + 12;
            printf("Src IP: %d.%d.%d.%d\n",p[0],p[1],p[2],p[3]);
            printf("Dst IP: %d.%d.%d.%d\n",p[4],p[5],p[6],p[7]);
            proto = (ip_head + 9)[0];
            p = ip_head + 12;
            switch (proto)
            {
            case IPPROTO_ICMP:
                printf("protocol:icmp\t");
                p = ip_head + 20;
                icmp_type = (p[0] << 8) + p[1];
                // printf("type code:0x%x\n",icmp_type);
                if(icmp_type == 0x0800)
                    printf("type:request\n");
                else if(icmp_type == 0)
                    printf("type:answer\n");
                else
                    printf("type:timeout\n");
                break;
            case IPPROTO_TCP:
                printf("protocol:tcp\n");
                p = ip_head + 20;
                printf("Src port:%d\n",(p[0] << 8) + p[1]);
                printf("Dst port:%d\n",(p[2] << 8) + p[3]);
                break;
            case IPPROTO_UDP:
                printf("protocol:udp\n");
                break;
            default:
                break;
            }
        }
        else if(eth_type == ETH_P_ARP)
        {
            arp_head = ethernet_head + 14;
            printf("protocol:ARP\t");
            arp_type = ((arp_head + 6)[0] << 8) + (arp_head + 6)[1];
            if(arp_type == 1)
                printf("type:ARP request\n");
            else if(arp_type == 2)
                printf("type:ARP answer\n");
            else if(arp_type == 3)
                printf("type:RARP request\n");
            else if(arp_type == 4)
                printf("type:RARP answer\n");
            else
                printf("type:unknown\n");
            p = arp_head + 8;
            printf("Src ARP MAC address: %.2x:%02x:%02x:%02x:%02x:%02x\n",
                    p[0],p[1],p[2],p[3],p[4],p[5]);
            printf("Src IP address:%d.%d.%d.%d\n",p[6],p[7],p[8],p[9]);
            printf("Dst ARP MAC address: %.2x:%02x:%02x:%02x:%02x:%02x\n",
                    p[10],p[11],p[12],p[13],p[14],p[15]);
            printf("Dst IP address:%d.%d.%d.%d\n",p[16],p[17],p[18],p[19]);
        }
    }
    return 0;
}