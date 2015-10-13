#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 1024
//#define DEFAULT_PORT "27015"

void printNewLineString(char charbuf[])
{
	int i = 0;
	while (charbuf[i] != '\n')
	{
		printf("%c", charbuf[i]);
		i++;
	}
	printf("\n");
}

void resetBuffer(char buf[], int length)
{
	int i = 0;
	for (i = 0; i < length; i++)
	{
		buf[i] = 0;
	}
}

int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN] = {0};
	char sendbuf[DEFAULT_BUFLEN] = {0};
	int recvbuflen = DEFAULT_BUFLEN;
	UINT16 networkbytes;
	char welcomeMessage[8] = "Welcome\n";
	char successMessage[8] = "Success\n";
	char failureMessage[8] = "Failure\n";
	char endMessage[20] = "it is over\0";

	int foundEntry = -1;
	int passedCheck = FALSE;

	//below is bad pratice but it works for storing IDS Names and Passwords
	char *IDArray[] = { "12345", "555", "2" };
	char *NameArray[] = { "Tim", "Michael", "Ed" };
	char *PasswordArry[] = {"password", "yes", "no"};

	int i = 0;

	while (TRUE)
	{
		resetBuffer(recvbuf, DEFAULT_BUFLEN);
		resetBuffer(sendbuf, DEFAULT_BUFLEN);
		//memset(recvbuf, '\0', DEFAULT_BUFLEN);
		//memset(send, '\0', DEFAULT_BUFLEN);
		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			printf("WSAStartup failed with error: %d\n", iResult);
			return 1;
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		iResult = getaddrinfo(NULL, argv[1], &hints, &result);
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			WSACleanup();
			continue;
		}

		// Create a SOCKET for connecting to server
		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ListenSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			continue;
		}

		// Setup the TCP listening socket
		iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			printf("bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			closesocket(ListenSocket);
			WSACleanup();
			continue;
		}

		freeaddrinfo(result);

		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			continue;
		}

		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			continue;
		}

		// No longer need server socket
		closesocket(ListenSocket);


		//send Client welcome message
		iSendResult = send(ClientSocket, welcomeMessage, 8, 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			continue;
		}

		//get id number and name
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			continue;
		}
		printf("Recieved ID number:");
		printNewLineString(recvbuf);

		//set to invalid value
		foundEntry = -1;
		passedCheck = 0;
		//checks ID vs ID array
		for (i = 0; i < 3; i++)
		{
			//check length so we know we don't over flow array
			if ((iResult -1) == strlen(IDArray[i]))
			{
				//compay recived id to ID Array element minus to \n
				if (memcmp(recvbuf, IDArray[i], (iResult - 1)) == 0)
				{
					foundEntry = i;
				}
			}
		}

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			continue;
		}

		printf("Recieved Name number:");
		printNewLineString(recvbuf);

		//checks name vs Name array
		if ((foundEntry != -1) && ((iResult - 1) == strlen(NameArray[foundEntry])))
		{
			//compay recived name to name Array element minus to \n
			if (memcmp(recvbuf, NameArray[foundEntry], (iResult - 1)) == 0)
			{
				passedCheck = TRUE;
			}
		}

		//if correct ID and Name send success otherwise send failure
		if (passedCheck == TRUE)
		{
			printf("found ID and name\n");
			iSendResult = send(ClientSocket, successMessage, 8, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				continue;
			}
		}
		else
		{
			printf("did not find ID and name\n");
			iSendResult = send(ClientSocket, failureMessage, 8, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				continue;
			}
			closesocket(ClientSocket);
			WSACleanup();
			continue;
		}

		//get password length and password
		printf("getting password lenght\n");
		iResult = recv(ClientSocket, &networkbytes, sizeof(UINT16), 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			continue;
		}

		networkbytes = ntohs(networkbytes);
		printf("length recv is %hd\n", networkbytes);
		printf("getting password\n");
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			continue;
		}
		recvbuf[iResult] = 0;
		printf("password recv is: %s\n", recvbuf);

		passedCheck = FALSE;

		//checks password
		if ((foundEntry != -1) && (iResult == strlen(PasswordArry[foundEntry])))
		{
			//compay recived name to passwprd Array element minus to \n
			if (memcmp(recvbuf, PasswordArry[foundEntry], (iResult - 1)) == 0)
			{
				passedCheck = TRUE;
			}
		}

		//create out end message
		if (passedCheck == TRUE)
		{
			strcat(sendbuf, "Condratulations ");
			strcat(sendbuf, NameArray[foundEntry]);
			strcat(sendbuf, "; you've just revealed the password for ");
			strcat(sendbuf, IDArray[foundEntry]);
			strcat(sendbuf, " to the world");
		}
		else
		{
			strcat(sendbuf, "Password incorrect.");
		}

		networkbytes = htons(strlen(sendbuf));
		printf("sending length %hd", strlen(sendbuf));
		iSendResult = send(ClientSocket, &networkbytes, sizeof(UINT16), 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			continue;
		}
		iSendResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			continue;
		}
		closesocket(ClientSocket);
		WSACleanup();
	}
	

	return 0;
}
