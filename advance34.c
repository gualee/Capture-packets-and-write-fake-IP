#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <netinet/ip.h> 

#define IFF_PROMISC 0x100
#define ETHER_ADDR_LEN  6
#define PortNumber 5555

struct pseudo_header{
	unsigned int source_address;
	unsigned int dest_address;
	unsigned char placeholder;
	unsigned char protocol;
	unsigned short tcp_length;
	struct tcphdr tcp;
};//for checksum calculation

unsigned short csum(unsigned short *ptr, int nbytes) {
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum = 0;
	while (nbytes>1) {
		sum += *ptr++;
		nbytes -= 2;
	}
	if (nbytes == 1){
		oddbyte = 0;
		*((u_char*)&oddbyte) = *(u_char*)ptr;
		sum += oddbyte;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum = sum + (sum >> 16);
	answer = (short)~sum;
	return(answer);
}

int main(int argc, char *argv[]){
	char datagram[4096], source_ip[32];
	struct iphdr *pip = (struct iphdr *) datagram;
	struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof(struct ip));
	struct sockaddr_in to;
	struct pseudo_header psh;

	//Establish socket
	int sock = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
	strcpy(source_ip, argv[1]);

	to.sin_family = AF_INET;
	to.sin_port = htons(4321);
	to.sin_addr.s_addr = inet_addr(argv[2]);

	memset(datagram, 0, 4096); 
	//IP Header
	pip->ihl = 5;
	pip->version = 4;
	pip->tos = 0;
	pip->tot_len = sizeof(struct ip) + sizeof(struct tcphdr);
	pip->id = htons(54321);
	pip->frag_off = 0;
	pip->ttl = 255;
	pip->protocol = IPPROTO_TCP;
	pip->check = 0;
	pip->saddr = inet_addr(source_ip);
	pip->daddr = to.sin_addr.s_addr;
	pip->check = csum((unsigned short *)datagram, pip->tot_len >> 1);

	//TCP Header
	tcph->source = htons(1234);
	tcph->dest = htons(80);
	tcph->seq = 0;
	tcph->ack_seq = 0;
	tcph->doff = 5;      
	tcph->fin = 0;
	tcph->syn = 1;
	tcph->rst = 0;
	tcph->psh = 0;
	tcph->ack = 0;
	tcph->urg = 0;
	tcph->window = htons(5840); 
	tcph->check = 0;
	tcph->urg_ptr = 0;
	
	//IP checksum
	psh.source_address = inet_addr(source_ip);
	psh.dest_address = to.sin_addr.s_addr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_TCP;
	psh.tcp_length = htons(20);

	memcpy(&psh.tcp, tcph, sizeof(struct tcphdr));

	tcph->check = csum((unsigned short*)&psh, sizeof(struct pseudo_header));

	//IP_HDRINCL to tell the kernel that headers are included in the packet
	int one = 1;
	const int *val = &one;
	if(setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0){
		printf("Error setting IP_HDRINCL. Error number : %d . Error message : %s \n", errno, strerror(errno));
		exit(0);
	}
	for(int i = 0; i < 1000; i++){
		int byte_sent = sendto(sock, datagram, pip->tot_len, 0, (struct sockaddr *) &to, sizeof(to));
		if(byte_sent < 0)
			printf("Sending error.\n");
	}//end for
	printf("Packet have Send \n");
	return 0;
}
