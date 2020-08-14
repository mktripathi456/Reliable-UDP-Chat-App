#include <stdlib.h>
#include "CoreRelUDP.c"
int main(void)
{

	int sockfd = SocketPreparationAndBindUsingPort(5000);
	struct sockaddr_in other;
	SocketAddressFill(&other, "127.0.0.1", 8000);

	int ch = 0;
	int cnt = 0;
	char message[4096];
	while (1)
	{
		ch = 0;
		ch = getchar();
		if (ch == '\n' || cnt == 4096)
		{
			message[cnt++] = '\0';
			cnt = 0;
			int msgLen = strlen(message);
			if (msgLen > 0)
			{
				printf("-------Sending--------\n");
				int status_msg_sent = SendMessage(sockfd, &other, message, msgLen);
				if (status_msg_sent == -1)
					ExitProgram("Error: Receiver Offline or ConnectionWeak");
			}

			continue;
		}
		message[cnt++] = ch;
	}
	printf("----------Chat Ended----------\n");
	return 0;
}