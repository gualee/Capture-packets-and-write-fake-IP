#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <net/if.h>

struct ether_header{
	unsigned char ether_dhost[ETH_ALEN];
	unsigned char ether_shost[ETH_ALEN];
	unsigned short ether_type;
};

int main(int argc, char *argv[]){
	
	struct ether_header *peth;
	struct iphdr *pip;
	struct tcphdr *ptcp;
	struct udphdr *pudp;
	struct icmphdr *picmp;
	struct igmphdr *pigmp;
	char *ptemp;
	char buffer[ETH_FRAME_LEN];
	int ip_counter = 0, arp_counter = 0, rarp_counter = 0, tcp_counter = 0, udp_counter = 0, icmp_counter = 0, igmp_counter = 0;
    
	//Establish socket
    int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0)
		  printf("Error creating socket\n");
    else
		  printf("Socket creating successfully\n");

	//Get interface flag
	struct ifreq ifr;
	strcpy(ifr.ifr_name,"enp1s0");
	if(ioctl(sock,SIOCGIFFLAGS,&ifr) == -1){
		perror("ioctl");
		exit(1);
	}
	
	//Set promiscuous mode
	ifr.ifr_flags |= IFF_PROMISC;
	if(ioctl(sock, SIOCSIFFLAGS, &ifr) == -1){
		perror("ioctl");
		exit(3);
	}
	
	while(1){
		//Receive packets
		int byte_recv = recvfrom(sock, buffer, ETH_FRAME_LEN, 0, NULL, NULL);
		if (byte_recv < 0){
			printf("Receive Failed\n");
			break;
		}
		ptemp = buffer;
		peth = (struct ether_header *)ptemp;
		
		if(ip_counter + arp_counter + rarp_counter + tcp_counter + udp_counter + icmp_counter + igmp_counter == 100) 
			break;

		switch(ntohs(peth -> ether_type)){
			case 0x0800:
				ip_counter++;
				break;
			case 0x0806:
				arp_counter++;
				break;
			case 0x8035:
				rarp_counter++;
				break;
		}

		ptemp += sizeof(struct ether_header);
		pip = (struct iphdr*)ptemp;
		switch(pip -> protocol){
			case IPPROTO_TCP:
			ptcp = (struct tcphdr*)ptemp;					
			tcp_counter++;
			break;

			case IPPROTO_UDP:
			pudp = (struct udphdr*)ptemp;
			udp_counter++;
			break;

            case IPPROTO_ICMP:
            picmp = (struct icmphdr*)ptemp;
            icmp_counter++;
            break;

            case IPPROTO_IGMP:
            pigmp = (struct igmphdr*)ptemp;
            igmp_counter++;
            break;
           
		}//end switch
	}//end while
	
	printf("------statics------\n");
	printf("IP      :%d\n", ip_counter);
	printf("ARP	    :%d\n", arp_counter);
	printf("RARP    :%d\n", rarp_counter);
	printf("TCP	    :%d\n", tcp_counter);
	printf("UDP	    :%d\n", udp_counter);
	printf("ICMP    :%d\n", icmp_counter);
	printf("IGMP    :%d\n", igmp_counter);
    printf("------finish------\n");

	close(sock);
	ifr.ifr_flags &= ~IFF_PROMISC;
	return 0;
}

