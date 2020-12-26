#include <iostream>
#include <string.h>
#include <fstream>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../defs.h"
using namespace std;

int main(void)
{
    int i,buf_len,listen_fd,connect_fd;
    struct sockaddr_in server_addr;
    char buf[DATA_MAX_SIZE];
    char div[2] = "$";
    char *pbuf;
    char filename[NAME_MAX_LEN];

    ofstream message;
    listen_fd = socket(AF_INET,SOCK_STREAM,0); 
    if(listen_fd == -1)
    {
        cout << "Failed to create listen socket" << endl;
        return LISTEN_ERR;
    }
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(LISTEN_PORT);
    if(bind(listen_fd,(struct sockaddr *)(&server_addr),sizeof(server_addr)) == -1)
    {
        cout << "Failed to bind listen socket" << endl;
        return BIND_ERR;
    }
    if(listen(listen_fd,MAX_REQ_NUM) == -1)
    {
        cout << "Faild to listen port" << endl;
        return LISTEN_ERR;
    }
    cout << "Initialize the server successfully.Now waiting for requests of clients..." << endl;
    while(true)
    {
        memset(buf,0,DATA_MAX_SIZE);
        connect_fd = accept(listen_fd,NULL,NULL);
        if(connect_fd == -1)
        {
            cout << "Failed to accept socket" << endl;
            continue;
        }
        buf_len = recv(connect_fd,buf,DATA_MAX_SIZE,0);
        cout << buf << endl;
        i=0;
        while(buf[i] != '$')
        {
            filename[i] = buf[i];
            i++;
        }
        filename[i] = '\0';
        pbuf = buf + i + 1;
        message.open(filename,ios::out);
        message << pbuf;
        message.close();
    }
    close(listen_fd);
    return 0;
}