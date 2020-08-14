#include <stdlib.h>
#include "CoreRelUDP.c"
int main(int argc, char **argv)
{

    char filename[15];
    printf("Enter the filename to be opened \n");
    scanf("%s", filename);

    FILE *fptr;
    char ch;
    int size = 0;

    fptr = fopen(filename, "r");
    if (fptr == NULL)
    {
        printf("Cannot open file \n");
        exit(0);
    }
    fseek(fptr, 0, 2);
    size = ftell(fptr);
    printf("The size of given file is : %d\n", size);
    fclose(fptr);

    fptr = fopen(filename, "r");
    if (fptr == NULL)
    {
        printf("Cannot open file \n");
        exit(0);
    }

    int sockfd = SocketPreparationAndBindUsingPort(5000);
    struct sockaddr_in other;
    SocketAddressFill(&other, "127.0.0.1", 8000);

    int cnt = 0, msgLen = 0;
    int MsgSize = 4096;
    char message[MsgSize];
    while ((ch = fgetc(fptr)) != EOF)
    {
        if (cnt % 100 == 0)
        {
            if (DEBUGPRINT)
                printf("CharsPacked-%d.\n", cnt);
        }

        if (cnt == MsgSize - 2)
        {
            message[cnt++] = '\0';
            cnt = 0;
            msgLen = strlen(message);
            if (msgLen > 1)
            {
                printf("-------Sending Block--------\n");
                int status_msg_sent = SendMessage(sockfd, &other, message, msgLen);
                if (status_msg_sent == -1)
                {
                    printf("Error: Reciever Offline.\n");
                    break;
                }
            }
        }
        message[cnt++] = ch;
    }
    message[cnt++] = '\0';
    msgLen = strlen(message);
    if (msgLen > 1)
    {
        printf("-------Sending Block--------\n");
        int status_msg_sent = SendMessage(sockfd, &other, message, msgLen);
        if (status_msg_sent == -1)
            ExitProgram("Error: Receiver Offline or ConnectionWeak");
    }

    printf("-------FilesSent--------\n");

    fclose(fptr);
    return 0;
}