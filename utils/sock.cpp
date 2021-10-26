#define PORT 50007
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "sock.h"
#include "log.h"

namespace Socks
{
    /*
        socket fuck
    */

   int act_sock = -1;

    int createConnection()
    {
        int sock = 0;
        struct sockaddr_in serv_addr;
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            return -1;
        }
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        
        if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
        {
            printf("\nInvalid address/ Address not supported \n");
            return -1;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\nConnection Failed \n");
            return -1;
        }
        act_sock = sock;
        return sock;
    }

    int sendMessage(std::string message)
    {
        if (act_sock < 0) return -1;
        char const * msg = message.c_str();
        send(act_sock , msg , strlen(msg) , 0 );
        Log::info("message sent %s", msg);
        return 0;
    }
}