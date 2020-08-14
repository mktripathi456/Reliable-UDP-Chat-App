#include <time.h>
#include "CoreRelUDP.c"
int main(void)
{

	time_t now;
	struct tm *local;
	struct sockaddr_in other;
	socklen_t otherLen = sizeof(other);
	int sockfd = SocketPreparationAndBindUsingPort(8000);

	char *message = (char *)malloc(sizeof(char) * 4096);
	char *message2 = (char *)malloc(sizeof(char) * 4096);

	int msgLen = 4096;
	while (1)
	{

		if (ReceiveMessage(sockfd, &other, otherLen, message, msgLen) < 0)
            ExitProgram("Error: ConnectionWeak");

		if (strcmp(message, message2) != 0)
		{

			time(&now);
			local = localtime(&now);
			printf("%02d:%02d:%02d > ", local->tm_hour, local->tm_min, local->tm_sec);

			printf("%s\n", message);
			strcpy(message2, message);
		}
	}

	return 0;
}