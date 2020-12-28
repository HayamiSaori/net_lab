#include <iostream>
#include <string.h>
#include <fstream>
#include <arpa/inet.h>
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
    char *pbuf;
    char filename[MAX_NAME_LEN];
    char ch;
    ofstream upload_file;
    ifstream download_file;
    
    listen_fd = socket(AF_INET,SOCK_STREAM,0); 
    if(listen_fd == -1)
    {
        cout << "Failed to create listen socket" << endl;
        return LISTEN_ERR;
    }
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    // server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_aton(ID_ADDR,&server_addr.sin_addr);
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
    cout << "From pid : " << getpid() << endl;
    cout << "Initialize the server successfully.Now waiting for requests of clients..." << endl;
    while(true)
    {
        memset(buf,0,DATA_MAX_SIZE);
        memset(filename,0,MAX_NAME_LEN);
        connect_fd = accept(listen_fd,NULL,NULL);
        if(connect_fd == -1)
        {
            cout << "Failed to accept socket" << endl;
            continue;
        }
        buf_len = recv(connect_fd,buf,DATA_MAX_SIZE,0);
        i = 0;
        if(buf[0] == FLAG_CH)   // means that this request is download
        {
            for(i=1;i<strlen(buf);i++)
            {
                filename[i - 1] = buf[i];
            }
            filename[i - 1] = '\0';
            cout << "get a request which wants to download file \"" << filename << "\"" << endl;
            memset(buf,0,DATA_MAX_SIZE);
            download_file.open(filename,ios::in);
            i = 0;
            if(download_file)
            {
                while (download_file.get(ch))
                {
                    buf[i] = ch;
                    i++;
                }
                buf[i] = '\0';
            }
            else                
            {
                buf[0] = FLAG_CH;buf[1] = '\0';
            }
            buf_len = send(connect_fd,buf,DATA_MAX_SIZE,0);
            download_file.close();
        }
        else                    // means that this request is upload
        {
            while(buf[i] != FLAG_CH)    // cut the buf to divide filename and content
            {
                filename[i] = buf[i];
                i++;
            }
            filename[i] = '\0';
            cout << "get a request which wants to upload file \"" << filename << "\"" << endl;
            pbuf = buf + i + 1;
            buf[strlen(buf)] = '\0';
            upload_file.open(filename,ios::out);
            upload_file << pbuf;
            upload_file.close();   
        }
    }
    close(listen_fd);
    return 0;
}