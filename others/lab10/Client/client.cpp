#include <iostream>
#include <string.h>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../defs.h"
using namespace std;
int main(void) 
{
    int fd,buf_len,i;
    char buf[DATA_MAX_SIZE];
    // char filename[NAME_MAX_LEN];
    string filename,filecontent="";
    char localhost[] = "127.0.0.1";
    char ch;
    struct sockaddr_in server_addr;
    ifstream sendfile;
    fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd == -1)
    {
        cout << "Failed to create socket" << endl;
        return SOCKET_ERR;
    }
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(LISTEN_PORT);
    inet_pton(AF_INET,localhost,&server_addr.sin_addr);
    if(connect(fd,(struct sockaddr *)(&server_addr),sizeof(server_addr)) == -1)
    {
        cout << "Connect error!" << endl;
        return CONNECT_ERR;
    }
    cout << "Which file do you want to send? Please input the filename" << endl;
    cin >> filename;
    sendfile.open(filename,ios::in);
    // sendfile >> filecontent;
    i = 0;
    while(sendfile.get(ch))
    {
        filecontent += ch;
        i++;
    }
    
    // cout << filecontent << endl;
    for(i=0;i<filename.size();i++)
    {
        buf[i] = filename[i];
    }
    buf[i] = '$';
    
    buf_len = filename.size() + filecontent.size() + 1;
    for(i=filename.size()+1;i<buf_len;i++)
    {
        buf[i] = filecontent[i - filename.size() - 1];
    }
    // cout << buf << endl;
    sendfile.close();
    send(fd,buf,strlen(buf),0);
    close(fd);
    return 0;
}