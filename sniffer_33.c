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
#include <arpa/inet.h>
#include <net/if.h>
#define Server_Address "140.120.14.113"

struct ether_header{
    unsigned char h_dest[ETH_ALEN];
    unsigned char h_source[ETH_ALEN];
	unsigned short ether_type;
};

int main(int argc, char *argv[]){
	struct ether_header *peth;
	struct iphdr *pip;
	struct tcphdr *ptcp;
	char *ptemp;
	char buffer[ETH_FRAME_LEN];
	struct ifreq ifr;
	struct sockaddr_in source, dest;
	int tcp_counter = 0;

	//Establish socket
	int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0)
		printf("Error creating socket\n");
	else
		printf("Socket creating successfully\n");

	/* Get Interface flag */
	strcpy(ifr.ifr_name, "enp1s0");
	if( ioctl(sock, SIOCGIFFLAGS, &ifr) == -1 ){
		perror("ioctl");
		exit(1);
	}
	/* Set promiscuous mode */
	ifr.ifr_flags |= IFF_PROMISC;
	if( ioctl(sock, SIOCGIFFLAGS, &ifr) == -1 ){
		perror("ioctl");
		exit(3);
	}
	
	while(1){	
		//Receive packets
		int receive = recvfrom(sock, buffer, ETH_FRAME_LEN, 0, NULL, NULL);
		if (receive < 0){
			printf("Receive Failed\n");
			break;
		}
		ptemp = buffer;
		peth = (struct ether_header *)ptemp;

		//Check IP Packet
		if(ntohs(peth->ether_type) == 0x0800){
			ptemp += sizeof(struct ether_header);
			pip = (struct iphdr*)ptemp;

			memset(&source, 0, sizeof(source));
		    source.sin_addr.s_addr = pip->saddr;
     
		    memset(&dest, 0, sizeof(dest));
		    dest.sin_addr.s_addr = pip->daddr;

			struct in_addr temp_src, temp_des;
			temp_src.s_addr = pip->saddr;
			temp_des.s_addr = pip->daddr;

			//Assign inputIP in String
			char IP[20];
			strcpy(IP, Server_Address);

			//Compare Host IP & Packet destination
			if ((IPPROTO_TCP == pip->protocol) && (strcmp(inet_ntoa(temp_des), IP) != 0) && (strcmp(inet_ntoa(temp_src), IP) != 0)){
				ptcp = (struct tcphdr*)ptemp;
				tcp_counter++;

				printf("\n=======Packet %d=======\n", tcp_counter);
				printf("Source MAC Address:%.2X:%.2X:%.2X:%.2X:%.2X:%.2X \n"
				, peth->h_source[0] , peth->h_source[1] , peth->h_source[2] , peth->h_source[3] , peth->h_source[4] , peth->h_source[5]);
				printf("Destination MAC Address:%.2X:%.2X:%.2X:%.2X:%.2X:%.2X \n"
				, peth->h_dest[0] , peth->h_dest[1] , peth->h_dest[2] , peth->h_dest[3] , peth->h_dest[4] , peth->h_dest[5] );
				printf("IP -> protocol: TCP\n");
				printf("IP -> src_ip:%s\n", inet_ntoa(temp_src));
				printf("IP -> dst_ip:%s\n", inet_ntoa(temp_des));
				printf("Src_port: %u\n", ntohs(ptcp->th_sport));
				printf("Dst_port: %u\n", ntohs(ptcp->th_dport));
			}//end if
		}//end if
    	if(tcp_counter == 10)
			break;
	}//end while
	close(sock);
	ifr.ifr_flags &= ~IFF_PROMISC;	
	return 0;
}




