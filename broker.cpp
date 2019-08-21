#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <winsock2.h>
#include <process.h>
#include "broker.h"

using namespace std;

#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32.lib")


SOCKET listen_sock, clnt_sock[10], data_sock[10];
int control_port = 36007;
int data_port;

MessageBroker broker;


typedef struct {
	string name;
	string sub_ip;
	string sub_port;
	string pub_ip;
	string pub_port;
}t;


void recvThread(void *arg);
// void connectToSub(void *);
void connectToSub(t);
void matchTopic(void);





int main() {
    int count = 0;
	WSADATA wsaData;
	HANDLE th_recv[10];
	data_port = control_port;

	struct sockaddr_in addr;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSA Start Failed\n");
		return 0;
	}

    addr.sin_family = AF_INET;
	addr.sin_port = htons(control_port); /* 36007 */
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Socket Open Failed\n");
		exit(1);
	}

	if (bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Bind Failed\n");
		exit(2);
	}

	if (listen(listen_sock, 5) == -1) {
		printf("Listen Failed\n");
		exit(3);
	}

    cout<<"broker"<<endl;

	while (1) {
		// accept and create client thread
		if ((clnt_sock[count] = accept(listen_sock, NULL, NULL)) == -1) {
			printf("Accept Failed\n");
			continue;
		}
		else {
			if (count < 10) {
				int idx = count;
				th_recv[count] = (HANDLE)_beginthread(recvThread, 0, &idx);
				count++;
			}
		}
	}
    closesocket(listen_sock);
	WSACleanup();
}


void recvThread(void *arg) {
    char buff[1024];
	char msg[1024];
	int connected = 1;
	int len;
	int transfermode = 0; // 0 ascii , 1 binary


	int idx = *((int*)arg);
		
	printf("User #%d connected\n", idx);

	// send 220 message
	memset(msg, 0, sizeof(msg));
	sprintf(msg, "220 Server ready\r\n");
	len = send(clnt_sock[idx], msg, strlen(msg), 0);

    while(connected) {
        memset(buff, 0, sizeof(buff));
		if (recv(clnt_sock[idx], buff, sizeof(buff), 0) > 0) {
            if (strncmp(buff, "PUB ", 4) == 0) {
                char * ptr = strtok(buff," ");
                char * name = strtok(NULL," ");
                char * port = strtok(NULL," ");
                char * ip = strtok(NULL," ");
                if(broker.registerTopic("pub",name,port,ip)) {
                    memset(msg, 0, sizeof(msg));
	                sprintf(msg, "230 register done\r\n");
					matchTopic(); /* 토픽 매칭 */
                } else {
                    memset(msg, 0, sizeof(msg));
	                sprintf(msg, "430 register failed\r\n");
                }
                len = send(clnt_sock[idx], msg, strlen(msg), 0);
				printf("User %d: %s\n", idx, msg);
            } else if (strncmp(buff, "SUB ", 4) == 0) {
                char * ptr = strtok(buff," ");
                char * name = strtok(NULL," ");
                char * port = strtok(NULL," ");
                char * ip = strtok(NULL," ");
                if(broker.registerTopic("sub",name,port,ip)) {
                    memset(msg, 0, sizeof(msg));
	                sprintf(msg, "230 register done\r\n");
					matchTopic(); /* 토픽 매칭 */
                } else {
                    memset(msg, 0, sizeof(msg));
	                sprintf(msg, "430 register failed\r\n");
                }
                len = send(clnt_sock[idx], msg, strlen(msg), 0);
				printf("User %d: %s\n", idx, msg);
            } else if (strncmp(buff,"TEST",4) == 0) {
				broker.testFunction();
			    // _beginthread(connectToSub,0,0);
			}
        }
    }
}

void connectToSub(t args) {

	string name = args.name;
	string pub_ip = args.pub_ip;
	string pub_port = args.pub_port;
	string sub_ip = args.sub_ip;
	string sub_port = args.sub_port;
	
	// printf("name: %s\npub_ip: %s\npub_port: %s\nsub_ip: %s\nsub_port: %s",
	// name.c_str(),pub_ip.c_str(),pub_port.c_str(),sub_ip.c_str(),sub_port.c_str());

	char msg[1024];
	SOCKET sock_main;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(sub_port.c_str()));
	addr.sin_addr.S_un.S_addr = inet_addr(sub_ip.c_str());

	if ((sock_main = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Socket Open Failed\n");
		exit(1);
	}

	if (connect(sock_main, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Connect Failed\n");
		exit(4);
	}

	
	memset(msg, 0, sizeof(msg));
	sprintf(msg, "240 Topic Matched %s %s %s"
		,name.c_str(),pub_port.c_str(),pub_ip.c_str());
	send(sock_main, msg, strlen(msg), 0);

	return;
}

void matchTopic() {

	broker.updateMatchList();
    map<string,Match>::iterator iter = broker.matchList.begin();
	for(iter=broker.matchList.begin(); iter!=broker.matchList.end(); iter++) {
		list<Owner>::iterator iter2 = iter->second.subList.begin();
		for(iter2; iter2!=iter->second.subList.end(); iter2++) {
			if(iter2->matched == false) {
				iter2->setMatched();
				HANDLE handle;
				t arg;
				arg.name = iter->first;
				arg.sub_ip = iter2->getip();
				arg.sub_port = iter2->getport();
				arg.pub_ip = iter->second.pub_ip;
				arg.pub_port = iter->second.pub_port;
				connectToSub(arg);
			}
		}
	}
	return;
}