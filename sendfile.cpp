#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <cassert>

using namespace std;

#define BUFF_SIZE 4096
int main(int argc, const char* argv[])
{
    // assert(argc < 3);

    const char* serv_ip = argv[1];
    int serv_port = atoi(argv[2]);
    
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    inet_pton(AF_INET, serv_ip, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(serv_port);
    int sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock_fd >= 0);
    
    if (connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Connection failed" << endl;
    }
    else {
        char buff[BUFF_SIZE];
        while (true) {
            int recv_size = recv(sock_fd, buff, BUFF_SIZE, 0);
            if (recv_size) {
                cout << recv_size << endl;
                for (int i = 0; i < recv_size; i++)
                    cout << buff[i]; 
                break;
            }
            // string msg;  
            // if (!(cin >> msg)) {
            //     cout << "Bad input, please retry." << endl;
            //     cin.clear();
            //     continue;
            // }
            //
            // send(sock_fd, msg.c_str(), msg.size(), 0);
            //
            // if (msg == "q")
            //     break;

        }

    }
    close(sock_fd);
    return 0;
}

