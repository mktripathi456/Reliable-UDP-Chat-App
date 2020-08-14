#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <math.h>

#define MAXTRIES 2147483640    // Max number of attempts to transmit  packet
#define MAXPKTSIZE 52 // Max size of application message packet  that needs to be transferred.
#define MAXMSG 50     // Max size of application message
#define TIMEOUT 900000 // Timeout

short DEBUGPRINT = 0;


int SendMessage(int sockfd, struct sockaddr_in *other2, char *message, int msgLen);


int ReceiveMessage(int sockfd, struct sockaddr_in *other2, socklen_t otherLen, char *message, int msgLen);

void ExitProgram(char *s);

int SocketPreparationAndBindUsingPort(int port);

int IsExpected(char *, int);

int SendPacketUsingRUDP(int sockfd, char *message, int msgLen, struct sockaddr *other);

int RecievePacketUsingRUDP(int sockfd, char *ack, int maxLen, int timeout, struct sockaddr *other);

int CreatePacketFromMessage(char *packet, short int seqNum, char *message);

int CheckAckAndResend(int sockfd, struct sockaddr_in *other, char *packet, int pktLen, int timeout);

int Time_LessThan(struct timeval *now, struct timeval *then);

int Time_Difference(struct timeval *now, struct timeval *then);

void SocketAddressFill(struct sockaddr_in *other, char *dest, int port);
