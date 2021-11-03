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
    bool sockMode = false;
    int custom_port = PORT;

    int createConnection()
    {
        int sock = 0;
        struct sockaddr_in serv_addr;
        Log::info("Attempting to connect to xowpy on port %d", custom_port);
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            Log::error("Socket creation error");
            return -1;
        }
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(custom_port);
        
        if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
        {
            Log::error("Invalid address/ Address not supported");
            return -1;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            Log::error("Connection Failed");
            return -1;
        }
        act_sock = sock;
        return sock;
    }

    int sendMessage(std::string message)
    {
        if (sockMode && act_sock < 0 && createConnection() < 0) 
            return 1;
        char const * msg = message.c_str();
        send(act_sock , msg , strlen(msg) , 0 );
        Log::info("message sent %s", msg);
        return 0;
    }
}