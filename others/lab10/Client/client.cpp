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
    string filecontent="";
    
    char filename[MAX_NAME_LEN];
    char ch;
    struct sockaddr_in server_addr;
    ifstream upload_file;
    ofstream download_file;
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
    inet_pton(AF_INET,ID_ADDR,&server_addr.sin_addr);
    if(connect(fd,(struct sockaddr *)(&server_addr),sizeof(server_addr)) == -1)
    {
        cout << "Connect error!" << endl;
        return CONNECT_ERR;
    }
    while(true)
    {
        memset(filename,0,MAX_NAME_LEN);
        memset(buf,0,DATA_MAX_SIZE);
        cout << "Please input the command.D|d for download and U|u for upload." << endl;
        cin >> ch;
        cout << "Please input the filename" << endl;
        cin >> filename;
        if(ch == 'D' || ch == 'd')
        {
            buf[0] = FLAG_CH;
            strcat(buf,filename);
            buf_len = send(fd,buf,DATA_MAX_SIZE,0);
            memset(buf,0,DATA_MAX_SIZE);
            buf_len = recv(fd,buf,DATA_MAX_SIZE,0);
            if(buf[0] == FLAG_CH)
            {
                cout << "No such file name \"" << filename << "\" in server." << endl; 
            }
            else
            {
                download_file.open(filename,ios::out);
                download_file << buf;
                download_file.close();
            }
        }
        else if(ch == 'U' || ch == 'u')
        {
            upload_file.open(filename,ios::in);
            i = 0;
            while(upload_file.get(ch))
            {
                filecontent += ch;
                i++;
            }
            for(i=0;i<strlen(filename);i++)
            {
                buf[i] = filename[i];
            }
            buf[i] = FLAG_CH;
            buf_len = strlen(filename) + filecontent.size() + 1;
            for(i=strlen(filename)+1;i<buf_len;i++)
            {
                buf[i] = filecontent[i - strlen(filename) - 1];
            }
            upload_file.close();
            send(fd,buf,strlen(buf),0);        
        }
        else
        {
            cout << "Error! Please try again." << endl;
        }
    }
    close(fd);
    return 0;
}