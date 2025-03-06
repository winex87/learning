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
    
    char recv_buf[4096];
    if (connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Connection failed" << endl;
    }
    else {
        while (true) {
            string msg;  
            if (!(cin >> msg)) {
                cout << "Bad input, please retry." << endl;
                cin.clear();
                continue;
            }

            if (msg == "q")
                break;

            send(sock_fd, msg.c_str(), msg.size(), 0);


            // int recv_size = recv(sock_fd, recv_buf, 4096, 0);
            // if (recv_size) {
            //     cout << recv_size << endl;
            //     for (int i = 0; i < recv_size; ++i)
            //         cout << recv_buf[i];
            //     cout << endl;
            // }
        }

    }
    close(sock_fd);
    return 0;
}

