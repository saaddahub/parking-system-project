#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "parkingsystem.h"
#include "vehicle.h"
#include "parkingrequest.h"

#pragma comment (lib, "Ws2_32.lib")

using namespace std;

#define PORT "8080"
#define DEFAULT_BUFLEN 4096

// --- PARKING SYSTEM INSTANCE ---
ParkingSystem sys(3, 2);

// --- HTTP HELPER FUNCTIONS ---
string getQueryParam(string url, string param) {
    size_t start = url.find(param + "=");
    if (start == string::npos) return "";
    start += param.length() + 1;
    size_t end = url.find("&", start);
    if (end == string::npos) end = url.length();
    return url.substr(start, end - start);
}

void handleRequest(SOCKET clientSocket) {
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iResult = recv(clientSocket, recvbuf, recvbuflen, 0);

    if (iResult > 0) {
        string request(recvbuf, iResult);
        
        // Extract First Line (Method resource HTTP/1.1)
        stringstream ss(request);
        string method, url, protocol;
        ss >> method >> url >> protocol;

        cout << "[REQ] " << method << " " << url << endl;

        // --- ROUTING ---
        if (url.find("/park?") == 0) {
            string vID = getQueryParam(url, "v");
            string zID_str = getQueryParam(url, "z");
            
            if (!vID.empty() && !zID_str.empty()) {
                int zID = atoi(zID_str.c_str());
                Vehicle *v = new Vehicle(vID, zID);
                ParkingRequest *req = new ParkingRequest(v);
                sys.parkVehicle(req);
            }
        }
        else if (url.find("/remove?") == 0) {
            string zID_str = getQueryParam(url, "z");
            string sID_str = getQueryParam(url, "s");

            if (!zID_str.empty() && !sID_str.empty()) {
                sys.removeVehicle(atoi(zID_str.c_str()), atoi(sID_str.c_str()));
            }
        }
        else if (url.find("/undo") == 0) {
            sys.undoLastAction();
        }

        // --- RESPONSE (Always the Dashboard) ---
        string html = sys.getHTML();
        string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + to_string(html.length()) + "\r\nConnection: close\r\n\r\n" + html;

        send(clientSocket, response.c_str(), response.length(), 0);
    }

    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL;
    struct addrinfo hints;

    // 1. Initialize Winsock
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

    // 2. Resolve Server Address and Port
    iResult = getaddrinfo(NULL, PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // 3. Create Socket
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // 4. Bind
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    // 5. Listen
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    cout << "========================================" << endl;
    cout << "   NEON PARKING SERVER ONLINE @ 8080    " << endl;
    cout << "========================================" << endl;
    system("start http://localhost:8080"); // Auto-open Browser

    // 6. Main Loop
    while(true) {
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        handleRequest(ClientSocket);
    }

    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}
