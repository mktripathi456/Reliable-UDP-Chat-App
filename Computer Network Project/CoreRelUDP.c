#include "ConstantsAndFuncDeclaration.h"

/*
*	Sends the message reliably and returns 1 if sent correctly.
*/

int SendMessage(int sockfd, struct sockaddr_in *other2, char *message, int msgLen) // ,char *dest, int port)
{

	struct sockaddr_in other = *(other2);

	char *msgending = message + msgLen;
	char *packet = (char *)malloc(sizeof(char) * MAXPKTSIZE);
	short int seqNum = 1;
	int pktLen;

	while (message <= msgending)
	{

		pktLen = CreatePacketFromMessage(packet, seqNum, message);
		
		message += pktLen - (sizeof(short int));

		if (DEBUGPRINT)
			printf("......Seq Number-%d.....\n", seqNum);

		if (CheckAckAndResend(sockfd, &other, packet, pktLen, TIMEOUT) < 0)
		{
			close(sockfd);
			return -1;
		}
		seqNum = (seqNum + 1) % (32767);
	}

	// send Null character if it hasn't been sent yet.
	if (msgLen % MAXMSG == 0)
	{
		seqNum = (seqNum + 1) % (32767);
		char nullMsg[] = "";
		pktLen = CreatePacketFromMessage(packet, seqNum, nullMsg);
		if (CheckAckAndResend(sockfd, &other, packet, pktLen, TIMEOUT) < 0)
		{
			close(sockfd);
			return -1;
		}
	}
	return 1;
}

int CheckAckAndResend(int sockfd, struct sockaddr_in *other, char *packet, int pktLen, int timeout)
{
	char *ack = (char *)malloc(sizeof(char) * 2);
	struct timeval now, then;
	int tries = 0;

	if (DEBUGPRINT)
		printf("....Sending...\n");

	while (tries != MAXTRIES)
	{
		if (DEBUGPRINT)
			if (tries > 0)
				printf("....Packet Resending...\n");

		SendPacketUsingRUDP(sockfd, packet, pktLen, (struct sockaddr *)other);
		tries++;

		gettimeofday(&now, NULL);
		gettimeofday(&then, NULL);
		then.tv_sec += (then.tv_usec + timeout) / 1000000;
		then.tv_usec = (then.tv_usec + timeout) % 1000000;

		while (Time_LessThan(&now, &then))
		{
			RecievePacketUsingRUDP(sockfd, ack, sizeof(char) * 2, Time_Difference(&now, &then), (struct sockaddr *)other);
			
			if (DEBUGPRINT)
				printf("....Current ACK Buffer(%d)...\n", *ack);
			
			if (IsExpected(ack, *(short int *)packet))
			{
				if (DEBUGPRINT)
					printf("....ACK Recieved-%d...\n", *ack);
				printf(" ✓\n");

				return 1;
			}
			gettimeofday(&now, NULL);
		}
	}
	return -1;
}


int ReceiveMessage(int sockfd, struct sockaddr_in *other2, socklen_t otherLen, char *message, int msgLen)
{
	char *msgEnding = message + msgLen;
	char *pointer = message;

	struct sockaddr_in other = *other2;

	char *packet = (char *)malloc(sizeof(char) * MAXPKTSIZE);
	short int pktLen;
	char *ack = (char *)malloc(sizeof(short int));
	short int seqNum = 1;

	while (pointer < msgEnding)
	{
		pktLen = recvfrom(sockfd, packet, MAXPKTSIZE, 0, (struct sockaddr *)&other, &otherLen);
	
		if (DEBUGPRINT)
			printf("......<UDTR>....\n");
	
		// If the received packet is the one we're expecting
		if (IsExpected(packet, seqNum))
		{
			strncpy(pointer, packet + sizeof(short int), MAXMSG);
	
			pointer += pktLen - sizeof(short int);
			*(short int *)ack = seqNum;
	
			if (DEBUGPRINT)
				printf(".......Recieved Seq Num-%d.....\n", seqNum);
	
			seqNum = (seqNum + 1) % (32767);
		}

		// Send ACK for the last received in-order packet.
		SendPacketUsingRUDP(sockfd, ack, sizeof(short int), (struct sockaddr *)&other);
		if (DEBUGPRINT)
			printf(".......Sent ACK-%d.....\n", *ack);

		if (packet[pktLen - 1] == '\0')
			break;
	}

	return strlen(message);
}

//	Sends a packet over an unreliable source.
int SendPacketUsingRUDP(int sockfd, char *message, int msgLen, struct sockaddr *other)
{
	if (DEBUGPRINT)
		printf("......<UDTS>....\n");
	socklen_t otherSize = (socklen_t)sizeof(*other);

	//Send N bytes of BUF on socket FD to peer at address ADDR (which is
	//ADDR_LEN bytes long). Returns the number sent, or -1 for errors.
	int len = sendto(sockfd, message, msgLen, 0, other, otherSize);
	return len;
}

//	waits to receive a packet from an unreliable source until timeout (in microseconds)
int RecievePacketUsingRUDP(int sockfd, char *ack, int maxLen, int timeout, struct sockaddr *other)
{

	if (DEBUGPRINT)
		printf("......<UDTR>....\n");
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = timeout;

	//SO_RCVTIMEO is an option to set a timeout value for input operations.
	//It accepts a struct timeval parameter with the number of seconds and microseconds used 
	//to limit waits for input operations to complete. In the current implementation, this timer
	// is restarted each time additional data are received by the protocol, and thus the limit is 
	//in effect an inactivity timer. 
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
	socklen_t otherSize = (socklen_t)sizeof(*other);

	//Read N bytes into BUF through socket FD.
	//If ADDR is not NULL, fill in *ADDR_LEN bytes of it with tha address of
	//the sender, and store the actual size of the address in *ADDR_LEN.
	//Returns the number of bytes read or -1 for errors.
	return recvfrom(sockfd, ack, maxLen, 0, other, &otherSize);
}

int CreatePacketFromMessage(char *packet, short int seqNum, char *message)
{
	*(short int *)packet = seqNum;
	// D <-- S , Atmost n
	strncpy(packet + sizeof(short int), message, MAXMSG);

	return strlen(packet + sizeof(short int)) + sizeof(short int) +
		   ((packet[MAXPKTSIZE - 1] == '\0') ? 1 : 0);

}

//	Is the received packet is the one expected
int IsExpected(char *buff, int seqNum)
{
	return *(short int *)buff == seqNum;
}

//	Prepare the socket and bind
int SocketPreparationAndBindUsingPort(int port)
{
	struct sockaddr_in this;
	//(SOCK_DGRAM) is a datagram-based protocol UDP

	//Some operating systems (eg. Linux kernel after 2.6.20) 
	//support a second protocol for SOCK_DGRAM, called UDP-Lite.
	// If supported by your system, it would be enabled by providing 
	//IPPROTO_UDPLITE as the third argument to the socket() call.
	//It is differentiated from normal UDP by allowing checksumming to
	// be applied to only a portion of the datagram. 
	//(Normally, UDP checksumming is an all-or-nothing effort.) 
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0)
		ExitProgram("Error");

	this.sin_family = AF_INET;
	
	//htons makes sure that numbers are stored in memory in network byte order,
	//  which is with the most significant byte first. 
	this.sin_port = htons(port);

	//converts unsigned integer hostlong from host byte order to network byte order.
	this.sin_addr.s_addr = htonl(INADDR_ANY);

	//bind() associates the socket with its local address,
	if (bind(sockfd, (struct sockaddr *)&this, sizeof(this)) == -1)
		ExitProgram("Error");

	return sockfd;
}

// Fill sockaddress
void SocketAddressFill(struct sockaddr_in *other, char *dest, int port)
{
	//IPV4 Address Family. Other like Unix Sockets (AF_UNIX),Bluetooth (AF_BLUETOOTH) 
	other->sin_family = AF_INET;
	
	//many devices store numbers in little-endian format,
	// meaning that the least significant byte comes first.

	other->sin_port = htons(port);
	
	//// int inet_aton(const char *cp, struct in_addr *addr)
	//converts the specified string, in the Internet standard dot notation,
	// to a network address, and stores the address in the structure provided.
	
	if (inet_aton(dest, &other->sin_addr) == 0) 
		ExitProgram("Invalid address");
}

//	Crash the program after printing the error
void ExitProgram(char *s)
{
	perror(s);
	exit(1);
}

//	Is timestamp 'now' less than 'then'?
int Time_LessThan(struct timeval *now, struct timeval *then)
{
	if (now->tv_sec < then->tv_sec)
		return 1;
	else if (now->tv_sec == then->tv_sec && now->tv_usec < then->tv_usec)
		return 1;
	return 0;
}

//	How many microseconds diff
int Time_Difference(struct timeval *now, struct timeval *then)
{
	int diff = 0;
	diff += (then->tv_sec - now->tv_sec) * 1000000;
	diff += (then->tv_usec - now->tv_usec);
	return diff;
}
