#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>

#define PORTNUM 2001

int myTcpPort;
char myFilePath[50];
int hashing_function(char *value);
void *myThread1(void *arg);
void *myThread2(void *arg);
void *myThread3(void *arg);

int myPredecccor;
int mySuccessor;
int preNodeID;
int nextNodeID;
struct sockaddr_in sender;
struct sockaddr_in reciever;
int socketForAll;

struct myPacket{
	int info;
	int suc;
	int pre;
	int initiator;
};

struct hash_data{
	int key;
	char fileName[20];
	char filePath[40];
};

struct packetInfo{
	int packetInitiator;
	int key;
	int info;
	char fileName[20];
	int freePort;
};

int MyNodeID;
struct hash_data hash_table[10];

int sendingFile(struct packetInfo from);
int recieveFile(struct sockaddr_in from, struct packetInfo recievedPacket);

void *myThread1(void *arg){
	int ch = 0;
	int mySocket = *((int *)arg);
	struct packetInfo recievedPacket;
	socklen_t socksize;
	int fileKey = 0;
	int nodeFileKey = 0;
	
	while(1){
		printf("Listeneing to packets \n");
		fflush(stdout);
		recvfrom(mySocket, &recievedPacket, sizeof(recievedPacket), 0, (struct sockaddr *)&sender, &socksize);
		printf("Packet recieved %d \n", recievedPacket.key);
		fflush(stdout);
		
		if(recievedPacket.packetInitiator == nextNodeID){
			continue;
		}
		
		if(recievedPacket.info == 500){
			printf("File is not present in the Network (DHT) \n");
			continue;
		}
		
		if (recievedPacket.info == 600){
			
		}
		
		if(recievedPacket.info == 100){
			printf("Wow My file is comming \n");
			fflush(stdout);
			
			while(1){
				ch = recieveFile(sender, recievedPacket);
				printf("ch: %d", ch);
				fflush(stdout);
				if(ch == 1){
					break;
				}
			}
			continue;
		}
		
		if(recievedPacket.info == 200){
			printf("Uploading File \n");
			fflush(stdout);
			
			sendingFile(recievedPacket);
			continue;
			
		}
		
		if((recievedPacket.key<MyNodeID && recievedPacket.key>preNodeID) || recievedPacket.key == MyNodeID){
			if(recievedPacket.info == 300){
				
				fileKey = hashing_function(recievedPacket.fileName);
				nodeFileKey = fileKey%10;
				
				if (hash_table[nodeFileKey].key == nodeFileKey){
					printf("Here is your File \n");
					fflush(stdout);
					printf("sending File.... Wait \n");
					fflush(stdout);
					sender.sin_port = htons(recievedPacket.packetInitiator);
					recievedPacket.info = 100;
					sendto(mySocket, &recievedPacket, sizeof(recievedPacket), 0, (struct sockaddr *)&sender, sizeof(sender));
					sendingFile(recievedPacket);
					continue;
				} else {
					sender.sin_port = htons(recievedPacket.packetInitiator);
					recievedPacket.info = 500;
					sendto(mySocket, &recievedPacket, sizeof(recievedPacket), 0, (struct sockaddr *)&sender, sizeof(sender));
					continue;
				}
			}
			
			if(recievedPacket.info == 400){
				printf("Here is the Node where your file save \n");
				fflush(stdout);
				fileKey = hashing_function(recievedPacket.fileName);
				fileKey = fileKey%10;
				hash_table[fileKey].key = fileKey;
				strcpy(hash_table[fileKey].fileName, recievedPacket.fileName);
				strcpy(hash_table[fileKey].filePath, myFilePath);
				strcat(hash_table[fileKey].filePath, recievedPacket.fileName);
				printf("Recieving File PLease Wait.... Wait \n");
				fflush(stdout);
				sender.sin_port = htons(recievedPacket.packetInitiator);
				recievedPacket.info = 200;
				sendto(mySocket, &recievedPacket, sizeof(recievedPacket), 0, (struct sockaddr *)&sender, sizeof(sender));
				
				while(1){
					ch = recieveFile(sender, recievedPacket);
					printf("ch: %d", ch);
					fflush(stdout);
					if(ch == 1){
						break;
					}
				}
			continue;
			}
			
		} else {
			printf("Your file is not here");
			fflush(stdout);
			sender.sin_family = AF_INET;
			sender.sin_port = htons(mySuccessor);
			sender.sin_addr.s_addr = inet_addr("127.0.0.1");
			sendto(mySocket, &recievedPacket, sizeof(recievedPacket), 0, (struct sockaddr *)&sender, sizeof(sender));
		}
	}
}

void *myThread2(void *arg){
	int mySocket = *((int *)arg);
	int decision = 0;
	socklen_t socksize;
	struct packetInfo sendingPacket;
	struct packetInfo recievingPacket;
	struct sockaddr_in recieverInfo;
	struct sockaddr_in senderInfo;
	printf("Choose One of Them:\nEnter 1 for Downloading a file\nEnter 2 for uploading the File\n?\n");
	fflush(stdout);
	scanf(" %d", &decision);
	fflush(stdin);
	
	if (decision == 1){
		recieverInfo.sin_family = AF_INET;
		recieverInfo.sin_port = htons(mySuccessor);
		recieverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
		printf("Enter the name of the file you want to downlaod: ");
		scanf(" %s", sendingPacket.fileName);
		fflush(stdin);
		sendingPacket.key = hashing_function(sendingPacket.fileName);
		sendingPacket.info = 300;
		sendingPacket.freePort = myTcpPort;
		myTcpPort++;
		sendingPacket.packetInitiator = PORTNUM;
		sendto(mySocket, &sendingPacket, sizeof(sendingPacket), 0, (struct sockaddr *)&recieverInfo, sizeof(recieverInfo));
	}
	
	if(decision == 2){
		recieverInfo.sin_family = AF_INET;
		recieverInfo.sin_port = htons(mySuccessor);
		recieverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
		printf("Enter the file name you want to upload on the DHT Node: ");
		fflush(stdout);
		scanf(" %s", sendingPacket.fileName);
		fflush(stdin);
		sendingPacket.key = hashing_function(sendingPacket.fileName);
		sendingPacket.info = 400;
		sendingPacket.freePort = myTcpPort;
		myTcpPort++;
		sendingPacket.packetInitiator = PORTNUM;
		sendto(mySocket, &sendingPacket, sizeof(sendingPacket), 0, (struct sockaddr *)&recieverInfo, sizeof(recieverInfo));
	}
}

int recieveFile(struct sockaddr_in from, struct packetInfo recievedPacket){
	socklen_t socksize;
	int tcpSocket;
	int recvBytes = 0;
	struct sockaddr_in tcpINFO;
	char filePath[50];
	strcpy(filePath, myFilePath);
	strcat(filePath, recievedPacket.fileName);
	sleep(1);
	tcpINFO.sin_family = AF_INET;
	tcpINFO.sin_port = htons(recievedPacket.freePort);
	tcpINFO.sin_addr.s_addr = inet_addr("127.0.0.1");
	tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	bind(tcpSocket, (struct sockaddr *)&tcpINFO, sizeof(struct sockaddr));
	int ch = connect(tcpSocket, (struct sockaddr *)&tcpINFO, sizeof(tcpINFO));
	
	if (ch == -1){
		close(tcpSocket);
		perror("client: connect");
		return -1;
	}
	int FileFD = open(filePath, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	char fileBuff[1023];
	
	while(1){
		recvBytes = recv(tcpSocket, fileBuff, sizeof(fileBuff), 0);
		write(FileFD, fileBuff, recvBytes);
		if (recvBytes == 0){
			close(tcpSocket);
			close(FileFD);
			return 1;
		}
	}
	
	return 1;
}

int sendingFile(struct packetInfo from){
	
	char filePath[50];
	strcpy(filePath, myFilePath);
	strcat(filePath, from.fileName);
	socklen_t socksize;
	int tcpSocket;
	struct sockaddr_in tcpINFO;
	tcpINFO.sin_family = AF_INET;
	tcpINFO.sin_port = htons(from.freePort);
	tcpINFO.sin_addr.s_addr = htonl(INADDR_ANY);
	tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	bind(tcpSocket, (struct sockaddr *)&tcpINFO, sizeof(struct sockaddr));
	listen(tcpSocket, 10);
	int conFD = accept(tcpSocket, (struct sockaddr *)&tcpINFO, &socksize);
	int FileFD = open(filePath, O_RDONLY);
	char fileBuff[1023];
	int recvBytes = 0;
	
	while(1){
		recvBytes = read(FileFD, fileBuff, sizeof(fileBuff));
		send(conFD, fileBuff, recvBytes, 0);
		printf("---recv\n");
		if(recvBytes == 0){
			close(FileFD);
			close(conFD);
			close(tcpSocket);
			return 0;
		}
	}
	return 0;
}

int main()
{
	/* Setting up the Socket for Communication */
	strcpy(myFilePath, "FILE_DIR_PATH");
	struct sockaddr_in dest, serv;
	int mySocket;
	char buff[100];
	MyNodeID = 10;
	myPredecccor = 2000;
	mySuccessor = 2002;
	preNodeID = 0;
	nextNodeID = 20;
	socklen_t socksize;
	serv.sin_family = AF_INET;
	serv.sin_port = htons(PORTNUM);
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	mySocket = socket(AF_INET, SOCK_DGRAM, 0);
	bind(mySocket, (struct sockaddr *)&serv, sizeof(serv));
	myTcpPort = 2234;
	/* End of Stablishing a connection */
	
	/* Initializing Threads */
	pthread_t tid1, tid2;
	pthread_create(&tid1, NULL, myThread1, &mySocket);
	pthread_create(&tid2, NULL, myThread2, &mySocket);
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	/* End of Creating Thread */
	
	/* Initializing the packet */
	struct myPacket card;
	card.info = 2003;
	/* End of initializing Packet */
	
	/* Ending Session */
	close(mySocket);
	return 0;
}

int hashing_function(char *value){
	int i = 0, key = 0;
	for(i=0; value[i]!='\0'; i++){
		if ((int)(value[i]) == 10) { break; }
		key += (int)(value[i]);
	}
	return (key%60);
}
