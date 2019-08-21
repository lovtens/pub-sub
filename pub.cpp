#include <iostream>
#include <stdio.h>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <winsock2.h>
#include <process.h>
#include "pub.h"
#include <direct.h>


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
int control_port = 37000; /*listen port */
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
void publishTopic(char *);


Publisher pub;

int main() {

	char tmp[40] = { 0 };


    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSA Start Failed\n");
		return 0;
	}
	cout<<"port : ";
	cin>>control_port;
    sprintf(ip_address,"127.0.0.1");
    cout<<"** publisher client **"<<endl;
	cin.clear();
    _beginthread(listenServer,0,0);

    ConnectToServer();

    while(1) {
        memset(command, 0, sizeof(command));
        fgets(command, 1000, stdin);
        command[strlen(command) - 1] = '\0';
        memset(tmp, 0, sizeof(tmp));

        if (strncmp(command,"PUB ", 4) == 0) {
            char * tmp2 = strtok(command," ");
            tmp2 = strtok(NULL," ");
            sprintf(tmp, "PUB %s %s %s",tmp2,to_string(control_port).c_str(),ip_address);
            _beginthread(recvThread,0,0);
        }
		else if (strncmp(command,"publish ", 8) == 0) {
			publishTopic(command);
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
	addr_listen.sin_port = htons(control_port); /* 37000 */
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
	int len;
	int idx = *((int*)arg);

	// printf("User #%d connected\n", idx);

	while(connected) {
        memset(buff, 0, sizeof(buff));
		if (recv(clnt_sock[idx], buff, sizeof(buff), 0) > 0) {
			if(strncmp(buff,"REQ ",4) ==0) {
                char * ptr = strtok(buff," ");
				char * name = strtok(NULL," ");
                char * port = strtok(NULL," ");
                char * ip = strtok(NULL," ");	
				// printf("name: %s\nport: %s\nip: %s",name,port,ip);
				pub.addTopic(name,port,ip);
				memset(msg, 0, sizeof(msg));
				sprintf(msg, "ACK %s",name);
				len = send(clnt_sock[idx], msg, strlen(msg), 0);
			}
		}
	}

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


void publishTopic(char * command) {


	SOCKET sock;
	list<Sub> subList;
	struct sockaddr_in addr;
	int count;
	int filesize;
	char msg[1024];
	char buff[1024];
	char tmpfile[1024];
	char curDir[1024];
	char * tmp2 = strtok(command," ");
	tmp2 = strtok(NULL," ");
	
	if(!pub.isInTopicList(tmp2)) {
		/* 토픽 없으면 리턴 */
		printf("no topic found\n");
		return;
	} else {
		subList = pub.getSubList(tmp2);
	}

	_getcwd(curDir, 1024);
	memset(tmpfile, 0, sizeof(tmpfile));
	strcpy(tmpfile, curDir);
	strcat(tmpfile, "\\");
	strncat(tmpfile, tmp2, strlen(tmp2));
	
	FILE *fp = fopen(tmpfile, "rb");
	if(fp == NULL){
		printf("no files found\n");
		return;
	}

	fseek(fp,0,SEEK_END);
	filesize = ftell(fp);
	// cout<<"filesize: "<<filesize<<endl;
	rewind(fp);

	memset(buff, 0, sizeof(buff));
	if((count = fread(buff,1,filesize,fp))>0) {
		list<Sub>::iterator iter = subList.begin();
		for(iter; iter!=subList.end(); iter++) {
			addr.sin_family = AF_INET;
			addr.sin_port = htons(atoi(iter->port.c_str()));
			addr.sin_addr.S_un.S_addr = inet_addr(iter->ip.c_str());
			if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
				printf("Socket Open Failed\n");
				exit(1);
			}
			if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
				printf("Connect Failed\n");
				exit(4);
			}
			memset(msg, 0, sizeof(msg));
			sprintf(msg, "REQ %s",tmp2);
			send(sock, msg, strlen(msg), 0);
			memset(msg, 0, sizeof(msg));
			if (recv(sock, (char*)msg, sizeof(msg), 0) > 0) {
				if (strncmp(msg, "ACK", 3) == 0){
					send(sock, buff, count, 0);
				}
			} else {
				printf("Disconnected\n");
				break;
			}
			closesocket(sock);
		}
	}
	fclose(fp);
	cout<<"published "<<tmp2<<endl;
	return;
}







