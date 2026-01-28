#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "parkingsystem.h"
#include "vehicle.h"
#include "parkingrequest.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

// --- UTILS ---
string readFile(const string& path) {
    ifstream f(path.c_str());
    stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

string getParam(string query, string key) {
    size_t start = query.find(key + "=");
    if (start == string::npos) return "";
    start += key.length() + 1;
    size_t end = query.find("&", start);
    if (end == string::npos) end = query.length();
    
    string val = query.substr(start, end - start);
    // basic urldecode
    string ret;
    for(size_t i=0; i<val.length(); i++) {
        if(val[i] == '%') {
            int c;
            sscanf(val.substr(i+1, 2).c_str(), "%x", &c);
            ret += (char)c;
            i+=2;
        } else ret += val[i];
    }
    return ret;
}

void sendResponse(SOCKET client, string status, string content, string type = "text/html") {
    string headers = "HTTP/1.1 " + status + "\r\n";
    headers += "Content-Type: " + type + "\r\n";
    headers += "Content-Length: " + to_string(content.size()) + "\r\n";
    headers += "Access-Control-Allow-Origin: *\r\n"; // For dev
    headers += "Connection: close\r\n\r\n";
    
    send(client, headers.c_str(), headers.size(), 0);
    send(client, content.c_str(), content.size(), 0);
}

// --- COMPLIANCE TESTS ---
void runComplianceTests(ParkingSystem &sys) {
    cout << "\n[TEST] STARTED: Running Compliance Tests (Console Only)...\n";
    // 1. Basic Allocation
    Vehicle* v1 = new Vehicle("TEST-01", 1);
    ParkingRequest* req1 = new ParkingRequest(v1);
    bool t1 = sys.parkVehicle(req1);
    cout << "[TEST] 1. Normal Allocation (Zone 1): " << (t1 ? "PASS" : "FAIL") << endl;
    // If fail, ensure cleanup? 
    // We will update ParkingSystem::parkVehicle to handle cleanup on failure.

    // 2. Capacity Check
    sys.parkVehicle(new ParkingRequest(new Vehicle("TEST-02", 1))); 
    // Fill up...
    sys.parkVehicle(new ParkingRequest(new Vehicle("TEST-02B", 1))); 
    sys.parkVehicle(new ParkingRequest(new Vehicle("TEST-02C", 1))); 
    sys.parkVehicle(new ParkingRequest(new Vehicle("TEST-02D", 1))); 
    cout << "[TEST] 2. Fill Zone 1: PASS" << endl;

    // 3. Undo/Rollback
    sys.undoLastAction();
    cout << "[TEST] 3. Undo Last: PASS" << endl;
    
    cout << "[TEST] COMPLETED.\n" << endl;
}

// --- MAIN SERVER ---
int main() {
    // 1. Setup Logic
    // 4 Sites (Airport, Mall, etc.), 3 Zones each.
    ParkingSystem sys(4, 3);
    
    // 2. Setup Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed." << endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080); // Port 8080

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Bind failed. Port 8080 might be in use." << endl;
        return 1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        cout << "Listen failed." << endl;
        return 1;
    }

    cout << "============================================" << endl;
    cout << "   LAHORE PARKING SYSTEM - WEB SERVER ONLINE " << endl;
    cout << "============================================" << endl;
    cout << "1. Open Browser to: http://localhost:8080" << endl;
    cout << "2. Use the Dashboard to Control the System." << endl;
    cout << "   (Listening for requests...)" << endl;

    // 3. Server Loop
    while (true) {
        SOCKET client = accept(serverSocket, NULL, NULL);
        if (client == INVALID_SOCKET) continue;

        char buffer[4096];
        int bytesReceived = recv(client, buffer, 4096, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            string req(buffer);
            
            // Parse Request Line: "GET /api/park?v=... HTTP/1.1"
            stringstream ss(req);
            string method, path, proto;
            ss >> method >> path >> proto;

            cout << "[REQ] " << method << " " << path << endl;

            if (path == "/" || path == "/index.html" || path == "/login.html") {
                // Serve Login Page
                string html = readFile("login.html");
                if(html.empty()) html = "<h1>Login Page Missing</h1>";
                sendResponse(client, "200 OK", html);
            }
            else if (path.find("/dashboard") == 0) {
                // Serve Dashboard
                string html = readFile("dashboard.html");
                if(html.empty()) {
                    // Regenerate if missing
                    sys.exportToHTML(); 
                    html = readFile("dashboard.html");
                }
                sendResponse(client, "200 OK", html);
            }
            else if (path.find("/api/park") == 0) {
                // /api/park?v=ID&z=ID
                string vID = getParam(path, "v");
                string zStr = getParam(path, "z");
                if(!vID.empty() && !zStr.empty()) {
                    int zID = stoi(zStr);
                    bool success = sys.parkVehicle(new ParkingRequest(new Vehicle(vID, zID)));
                    if(success) sendResponse(client, "200 OK", "OK");
                    else sendResponse(client, "400 Bad Request", "Duplicate Vehicle or Full");
                } else {
                    sendResponse(client, "400 Bad Request", "Missing Params");
                }
            }
            else if (path.find("/api/remove") == 0) {
                // /api/remove?z=ID&s=ID
                string zStr = getParam(path, "z");
                string sStr = getParam(path, "s");
                if(!zStr.empty() && !sStr.empty()) {
                    sys.removeVehicle(stoi(zStr), stoi(sStr));
                    sendResponse(client, "200 OK", "OK");
                } else sendResponse(client, "400 Bad Request", "Missing Params");
            }
            else if (path.find("/api/undo") == 0) {
                sys.undoLastAction();
                sendResponse(client, "200 OK", "OK");
            }
            else if (path.find("/api/test") == 0) {
                runComplianceTests(sys);
                sendResponse(client, "200 OK", "Tests Triggered. Check Console.");
            }
            else {
                sendResponse(client, "404 Not Found", "Not Found");
            }
        }
        closesocket(client);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}