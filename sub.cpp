#include <iostream>
#include <stdio.h>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <winsock2.h>
#include <process.h>
#include "sub.h"

using namespace std;
using std::string;

#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32.lib")


SOCKET sock_main;
SOCKET listen_sock, clnt_sock[10], data_sock[10];
WSADATA wsaData;
struct sockaddr_in addr, addr_data;
struct sockaddr_in addr_listen;
int port = 36007;
int control_port = 38000; /*listen port */
int data_port;
char ip_address[16];
char buff[1024];
char command[100];
int remote_ip[4];
int remote_port[2];

void ConnectToServer();
void recvThread(void *);
void listenThread(void *);
void listenServer(void *arg);
void connectToPub(Topic);


Subscriber sub;

int main() {

	char tmp[40] = { 0 };

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSA Start Failed\n");
		return 0;
	}
	
	cout<<"port : ";
	cin>>control_port;
    sprintf(ip_address,"127.0.0.1");
    cout<<"** subscriber client **"<<endl;
	cin.clear();
	_beginthread(listenServer,0,0);

    ConnectToServer();

    while(1) {
        memset(command, 0, sizeof(command));
        fgets(command, 1000, stdin);
        command[strlen(command) - 1] = '\0';
        memset(tmp, 0, sizeof(tmp));

        if (strncmp(command,"SUB ", 4) == 0) {
            char * tmp2 = strtok(command," ");
            tmp2 = strtok(NULL," ");
            sprintf(tmp, "SUB %s %s %s",tmp2,to_string(control_port).c_str(),ip_address);
            _beginthread(recvThread,0,0);
        }
        send(sock_main,tmp,strlen(tmp),0);
        memset(buff,0,sizeof(buff));
    }

}


void listenServer(void *) {
    int count = 0;
	data_port = control_port;
	HANDLE th_recv[10];

    addr_listen.sin_family = AF_INET;
	addr_listen.sin_port = htons(control_port); /* 38000 */
	addr_listen.sin_addr.S_un.S_addr = INADDR_ANY;

    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Socket Open Failed\n");
		exit(1);
	}

	if (bind(listen_sock, (struct sockaddr*)&addr_listen, sizeof(addr_listen)) == -1) {
		printf("Bind Failed\n");
		exit(2);
	}

	if (listen(listen_sock, 5) == -1) {
		printf("Listen Failed\n");
		exit(3);
	}

    while (1) {
		if ((clnt_sock[count] = accept(listen_sock, NULL, NULL)) == -1) {
			printf("Accept Failed\n");
			continue;
		}
		else {
			if (count < 10) {
				int idx = count;
				th_recv[count] = (HANDLE)_beginthread(listenThread, 0, &idx);
				count++;
			}
		}
	}
    closesocket(listen_sock);
}

void connectToPub(Topic topic) {
	string pub_port = topic.port;
	string pub_ip = topic.ip;
	string name = topic.name;
	char msg[1024];
	char buff[1024];
	SOCKET sock;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(pub_port.c_str()));
	addr.sin_addr.S_un.S_addr = inet_addr(pub_ip.c_str());

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Socket Open Failed\n");
		exit(1);
	}

	if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Connect Failed\n");
		exit(4);
	}
	memset(msg, 0, sizeof(msg));
	sprintf(msg, "REQ %s %s %s",
			name.c_str(),to_string(control_port).c_str(),ip_address);
	send(sock, msg, strlen(msg), 0);
	
	while(1) {
		memset(buff, 0, sizeof(buff));
		if (recv(sock, (char*)buff, sizeof(buff), 0) > 0) {
			// printf("%s", buff);
			if (strncmp(buff, "ACK ", 4) == 0) {
				char * ptr = strtok(buff," ");
				char * name = strtok(NULL," ");
				printf("subscribed %s\n",name);
                return;
            }
		} else {
			printf("Disconnected\n");
			closesocket(sock);
			WSACleanup();
			exit(5);
		}
	}
	return;

}

void ConnectToServer() {

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = inet_addr(ip_address);

	if ((sock_main = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Socket Open Failed\n");
		exit(1);
	}
	if (connect(sock_main, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Connect Failed\n");
		exit(4);
	}
    while (1) {
		memset(buff, 0, sizeof(buff));
		if (recv(sock_main, (char*)buff, sizeof(buff), 0) > 0) {
			// printf("%s", buff);
			if (strncmp(buff, "220", 3) == 0) {
                cout<<"broker server connected"<<endl;
                return;
			} else if (strncmp(buff, "230",3) == 0) {
                cout<<"topic register done"<<endl;
                return;
            } else if (strncmp(buff, "430",3) == 0) {
                cout<<"topic register failed"<<endl;
                return;
            }
		} else {
			printf("Disconnected\n");
			closesocket(sock_main);
			WSACleanup();
			exit(5);
		}
	}
}


void listenThread(void *arg) {
	char buff[1024];
	char msg[1024];
	int connected = 1;
	bool filemode = false;
	int count;
	int len;
	int idx = *((int*)arg);
	string filename;
	FILE *fp;
	// printf("User #%d connected\n", idx);

	while(connected) {
        memset(buff, 0, sizeof(buff));
		if ((count = recv(clnt_sock[idx], buff, sizeof(buff), 0)) > 0) {
			if(filemode) {
				fp = fopen(filename.c_str(), "wb+");
				fwrite(buff, sizeof(char), count, fp);
				fclose(fp);
				filemode = false;
				cout<<filename<<" created"<<endl;
				break;
			}
			if(strncmp(buff,"240 ",4) ==0) {
                char * ptr = strtok(buff," ");
				ptr = strtok(NULL," ");
				ptr = strtok(NULL," ");
				char * name = strtok(NULL," ");
                char * port = strtok(NULL," ");
                char * ip = strtok(NULL," ");
				
				// printf("name: %s\nport: %s\nip: %s",name,port,ip);
				Topic topic(name,port,ip);
				sub.addTopic(topic);
				connectToPub(topic);
			}
			else if (strncmp(buff,"REQ ",4) == 0) {
                char * ptr = strtok(buff," ");
				filename = strtok(NULL," ");
				memset(msg, 0, sizeof(msg));
				sprintf(msg, "ACK");
				len = send(clnt_sock[idx], msg, strlen(msg), 0);
				filemode = true;
			}
		}
	}
	closesocket(clnt_sock[idx]);
}



void recvThread(void *) {

    while (1) {
		memset(buff, 0, sizeof(buff));
		if (recv(sock_main, (char*)buff, sizeof(buff), 0) > 0) {
			// printf("%s", buff);
            if (strncmp(buff, "230",3) == 0) {
                cout<<"topic register done"<<endl;
                return;
            } else if (strncmp(buff, "430",3) == 0) {
                cout<<"topic register failed"<<endl;
                return;
            }
		} else {
			printf("Disconnected\n");
			closesocket(sock_main);
			WSACleanup();
			exit(5);
		}
	}
}

