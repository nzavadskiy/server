#pragma comment (lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "8080"
#define SETTINGS_FILE "settings.json"
#define WIN32_LEAN_AND_MEAN
#undef UNICODE

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <random>
#include <list>
#include <iostream>
#include <json.hpp>
#include <iomanip>
#include <string>
#include "jsonConverter.cpp"
#include "server.h"
#include <fstream>
#include <sstream>

using json = nlohmann::json;
using namespace std;

void to_json(json& j, const Subscriber& p)
{
	j = json{
		{"imeiSV", p.imeiSV}, {"imsi", p.imsi},
		{"assistData", p.assistData}, { "bsName", p.bsName }, { "subName", p.subName }
	};
}

void from_json(const json& j, Subscriber& p)
{
	j.at("imeiSV").get_to(p.imeiSV);
	j.at("imsi").get_to(p.imsi);
	j.at("assistData").get_to(p.assistData);
	j.at("bsName").get_to(p.bsName);
	j.at("subName").get_to(p.subName);
}

void to_json(json& j, const Geolocation& p)
{
	j = json{ {"imeiSV", p.imeiSV}, {"imsi", p.imsi} , {"latitude", p.latitude}, {"longtitude", p.longtitude}, {"date", p.date} };
}

void from_json(const json& j, Geolocation& p)
{
	j.at("imeiSV").get_to(p.imeiSV);
	j.at("imsi").get_to(p.imsi);
	j.at("latitude").get_to(p.latitude);
	j.at("longtitude").get_to(p.longtitude);
	j.at("date").get_to(p.date);
}

void to_json(json& j, const TA& p)
{
	j = json{
		{"imeiSV", p.imeiSV}, {"imsi", p.imsi} ,
		{"ta", p.TA}, {"lev1", p.lev1}, {"lev2", p.lev2},
		{"lev3", p.lev3}, {"lev4", p.lev4}, {"lev5", p.lev5},
		{"lev6", p.lev6}, {"lev7", p.lev7}
	};
}

void from_json(const json& j, TA& p)
{
	j.at("imeiSV").get_to(p.imeiSV);
	j.at("imsi").get_to(p.imsi);
	j.at("ta").get_to(p.TA);
	j.at("lev1").get_to(p.lev1);
	j.at("lev2").get_to(p.lev2);
	j.at("lev3").get_to(p.lev3);
	j.at("lev4").get_to(p.lev4);
	j.at("lev5").get_to(p.lev5);
	j.at("lev6").get_to(p.lev6);
	j.at("lev7").get_to(p.lev7);
}

void to_json(json& j, const CellID& p)
{
	j = json{
		{"imsi", p.imsi}, {"imeiSV", p.imeiSV}, {"dist", p.dist}
	};
}

void from_json(const json& j, CellID& p)
{
	j.at("imsi").get_to(p.imsi);
	j.at("imeiSV").get_to(p.imeiSV);
	j.at("dist").get_to(p.dist);
}

void to_json(json& j, const BS& p)
{
	j = json{
		{"name", p.name}, {"mcc", p.mcc}, {"mnc", p.mnc}, {"lac", p.lac}, {"cellid", p.cellid},
		{"lat", p.lat} , {"lon", p.lon} , {"antenna", p.antenna},
		{"ip", p.ip} , {"port", p.port}
	};
}

void from_json(const json& j, BS& p)
{
	j.at("name").get_to(p.name);
	j.at("mcc").get_to(p.mcc);
	j.at("mnc").get_to(p.mnc);
	j.at("lac").get_to(p.lac);
	j.at("cellid").get_to(p.cellid);
	j.at("lat").get_to(p.lat);
	j.at("lon").get_to(p.lon);
	j.at("antenna").get_to(p.antenna);
	j.at("ip").get_to(p.ip);
	j.at("port").get_to(p.port);
}

string GetFileString(string fileName)
{
	try
	{
		std::ifstream f(fileName);
		std::stringstream ss;
		ss << f.rdbuf();
		return ss.str();
	}
	catch (exception e)
	{
		cout << e.what() << endl;
		return "";
	}
}

class Client
{
public:
	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char buffer[1024] = { 0 };

	string Start(string ip, int port)
	{
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			return "Socket creation error";
		}

		memset(&serv_addr, '0', sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(port);

		// Convert IPv4 and IPv6 addresses from text to binary form
		if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0)
		{
			return "Invalid address/ Address not supported ";
		}

		if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			return "\nConnection Failed";
		}

		return "";
	}
};

class Server
{
public:
	static SOCKET ClientSocket;
	static char recvbuf[DEFAULT_BUFLEN];
	static Client client;
	static int recvbuflen;
	static SOCKET ListenSocket;
	static string localIP;
	static string remoteIP;
	static int localPort;
	static int remotePort;

public:
	
	static void GetIpPort(string jString)
	{
		json j = json::parse(jString);
		localIP = j["localIP"].get<string>();
		remoteIP = j["remoteIP"].get<string>();
		localPort = j["localPort"].get<int>();
		remotePort = j["remotePort"].get<int>();
	}

	static int SetSocket()
	{
		GetIpPort(GetFileString(SETTINGS_FILE));

		WSADATA wsaData;
		int iResult;
		struct addrinfo *result = NULL;
		struct addrinfo hints;

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
		{
			printf("WSAStartup failed with error: %d\n", iResult);
			return iResult;
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE; 

		// Resolve the server address and port
		iResult = getaddrinfo(localIP.c_str(), to_string(localPort).c_str(), &hints, &result);
		if (iResult != 0)
		{
			WSACleanup();
			return iResult;
		}

		// Create a SOCKET for connecting to server
		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ListenSocket == INVALID_SOCKET)
		{
			freeaddrinfo(result);
			WSACleanup();
			return WSAGetLastError();
		}

		const sockaddr* addr = result->ai_addr;
		int len = result->ai_addrlen;
		// Setup the TCP listening socket
		iResult = bind(ListenSocket, addr, len);
		if (iResult == SOCKET_ERROR)
		{
			freeaddrinfo(result);
			closesocket(ListenSocket);
			WSACleanup();
			return WSAGetLastError();
		}

		freeaddrinfo(result);
		return 0;
	}
	static DWORD WINAPI ListenAndReceive()
	{
		while (true)
		{
			BS request;
			request.ip = localIP;
			request.port = to_string(localPort);
			json j = request;
			string r = "0" + j.dump();
			client.Start(remoteIP, remotePort);
			send(client.sock, r.c_str(), strlen(r.c_str()), 0);
			strcpy_s(recvbuf, "");
			int iResult = listen(ListenSocket, SOMAXCONN);
			if (iResult == SOCKET_ERROR)
			{
				closesocket(ListenSocket);
				WSACleanup();
				return WSAGetLastError();
			}
			recv(client.sock, recvbuf, recvbuflen, 0);
			// Accept a client socket
			ClientSocket = accept(ListenSocket, NULL, NULL);
			if (ClientSocket == INVALID_SOCKET) {
				closesocket(ListenSocket);
				WSACleanup();
				return WSAGetLastError();
			}
			// заглушки на местоположение
			list<string> positions;
			positions.push_back("4211ffffd33650b6413cc69c6a94a801c8883cb0d110");
			positions.push_back("e211ffffd53220b6413cc63c6a93b0030c8440b8ad10");
			positions.push_back("4211ffff4f1230b6413cc60c6a945401fc7420d49110");
			positions.push_back("2211ffffb3873db64139649869e2bc0060bcb8f9fd10");
			positions.push_back("2211ffffcafb64b6413cc7206a945a0000e49d006510");
			positions.push_back("2211ffffcf7560b6413cc6146a96100000f895044910");
			positions.push_back("2211ffffd32c1cb6413cc7c46a95100000c8a90c7110");
			positions.push_back("2211ffffd53220b6413cc72c6a954c000108a8c13d10");
			positions.push_back("2211ffff0e1960b6413cc4c06a963c00049874ccb910");

			int iSendResult;
			for (int i = 0; i < 3; i++)
			{
				BS bs;
				bs.name = "bs" + to_string(i + 1);
				bs.lac = "1002";
				bs.mcc = "250";
				bs.mnc = "02";
				bs.lat = "111";
				bs.lon = "222";
				bs.ip = localIP;
				bs.port = to_string(localPort);
				bs.cellid = to_string(i + 1);
				json j = bs;
				string r = "11" + j.dump(); 
				client.Start(remoteIP, remotePort);
				send(client.sock, r.c_str(), strlen(r.c_str()), 0);
				strcpy_s(recvbuf, "");
				recv(client.sock, recvbuf, recvbuflen, 0);
			}
			do {
				strcpy_s(recvbuf, "");
				iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
				string recvb = (string)recvbuf;
				if (iResult > 0)
				{
					if (recvbuf[0] == '1') // запрос на передачу rrlp-сообщения
					{
						// получение абонента от { до }
						string a = recvb.substr(1, recvb.find_last_of('}'));
						a.shrink_to_fit(); // чтоб рботало
						json j_ = json::parse(a);
						Subscriber sub = j_.get<Subscriber>();	
						iSendResult = send(ClientSocket, "90", strlen("90"), 0);
						// заглушка на обработку сообщений
						string r = "";
						if (sub.assistData == "ms-based gps")
						{
							if (positions.size() != 0)
							{
								sub.assistData = positions.back();
								positions.pop_back();
								json j = sub;
								 r = "32" + j.dump();
							}
							else
							{
								sub.assistData = "e211ffffd53220b6413cc63c6a93b0030c8440b8ad10";
								json j = sub;
								r = "33" + j.dump();
							}							
							client.Start(remoteIP, remotePort);
							send(client.sock, r.c_str(), strlen(r.c_str()), 0);
							strcpy_s(recvbuf, "");
							recv(client.sock, recvbuf, recvbuflen, 0);
						}
						if (sub.assistData == "ms-based e-otd")
						{
							sub.assistData = "22041c";
							json j = sub;
							string r = "33" + j.dump();
							client.Start(remoteIP, remotePort);
							send(client.sock, r.c_str(), strlen(r.c_str()), 0);
							strcpy_s(recvbuf, "");
							recv(client.sock, recvbuf, recvbuflen, 0);
						}
						if (sub.assistData == "ms-assisted gps")
						{
							sub.assistData = "22082202BD09C9CF3F37873E002D26F21738BB0066B4D2AE58770000";
							json j = sub;
							string r = "33" + j.dump();
							client.Start(remoteIP, remotePort);
							send(client.sock, r.c_str(), strlen(r.c_str()), 0);
							strcpy_s(recvbuf, "");
							recv(client.sock, recvbuf, recvbuflen, 0);
						}
						if (sub.assistData == "ms-assisted e-otd")
						{
							sub.assistData = "22041c";
							json j = sub;
							string r = "33" + j.dump();
							client.Start(remoteIP, remotePort);
							send(client.sock, r.c_str(), strlen(r.c_str()), 0);
							strcpy_s(recvbuf, "");
							recv(client.sock, recvbuf, recvbuflen, 0);
						}						
					}					
					if (recvb[0] == '2') //местоположение по ТА
					{
						// получение абонента
						string a = recvb.substr(1, recvb.find_last_of('}'));
						a.shrink_to_fit();
						json j_ = json::parse(a);
						Subscriber sub = j_.get<Subscriber>();  // got subscriber
						iSendResult = send(ClientSocket, "90", strlen("90"), 0);
						uniform_int_distribution<int> distribution_ta(0, 63);
						uniform_int_distribution<int> distribution_lev(20, 140);
						std::random_device rd;
						default_random_engine generator(rd());
						TA ta;
						ta.TA = to_string(distribution_ta(generator));
						ta.lev1 = to_string(distribution_lev(generator));
						ta.lev2 = to_string(distribution_lev(generator));
						ta.lev3 = to_string(distribution_lev(generator));
						ta.lev4 = to_string(distribution_lev(generator));
						ta.lev5 = to_string(distribution_lev(generator));
						ta.lev6 = to_string(distribution_lev(generator));
						ta.lev7 = to_string(distribution_lev(generator));
						ta.imsi = sub.imsi;
						ta.imeiSV = sub.imeiSV;
						// отправка данных
						json j = ta;
						string r = "31" + j.dump();
						client.Start(remoteIP, remotePort);
						send(client.sock, r.c_str(), strlen(r.c_str()), 0);
						strcpy_s(recvbuf, "");
						recv(client.sock, recvbuf, recvbuflen, 0);
					}
					if (recvb[0] == '3') //местоположение по CELL ID + TA
					{
						//получение данных
						string a = recvb.substr(1, recvb.find_last_of('}'));
						a.shrink_to_fit();
						json j_ = json::parse(a);
						Subscriber sub = j_.get<Subscriber>();  // got subscriber
						iSendResult = send(ClientSocket, "90", strlen("90"), 0);
						//заглушка						
						CellID cellid;
						cellid.imsi = sub.imsi;
						cellid.imeiSV = sub.imeiSV;
						cellid.dist = "1";
						json j = cellid;
						string r = "34" + j.dump();
						client.Start(remoteIP, remotePort);
						send(client.sock, r.c_str(), strlen(r.c_str()), 0);
						strcpy_s(recvbuf, "");
						recv(client.sock, recvbuf, recvbuflen, 0);
					}
					else
					{
						//iSendResult = send(ClientSocket, "-1", strlen("-1"), 0);
					}
					if (iSendResult == SOCKET_ERROR)
					{
						closesocket(ClientSocket);
						WSACleanup();
						return WSAGetLastError();
					}
				}
				else if (iResult == 0)
				{
					break;
				}
				else
				{
					closesocket(ClientSocket);
				}
			} while (iResult > 0);
		}
		ShutDown();
		return 0;
	}

	static int ShutDown()
	{
		int iResult;
		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(ClientSocket);
			WSACleanup();
			return WSAGetLastError();
		}
		closesocket(ClientSocket);
		WSACleanup();
	}
};

SOCKET Server::ClientSocket = INVALID_SOCKET;
int Server::recvbuflen = DEFAULT_BUFLEN;
SOCKET Server::ListenSocket = INVALID_SOCKET;
char Server::recvbuf[DEFAULT_BUFLEN];
Client Server::client;
string Server::localIP;
string Server::remoteIP;
int Server::localPort;
int Server::remotePort;

int __cdecl main(void)
{
	cout << "Server is working ..." << endl;

	Server::SetSocket();
	DWORD thID;
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Server::ListenAndReceive, NULL, NULL, &thID);
	string p;

	while (true)
	{
		strcpy_s(Server::recvbuf, "");
		int t;
		cout << "Enter command\n";
		cout << "1. Add BS\n";
		cout << "2. Remove BS\n";
		cout << "3. Alter BS\n";
		cout << "4. Add subscriber\n";
		cout << "5. Remove subscriber\n";
		cout << "0. Shut down\n";
		cin >> t;
		BS bs;
		Subscriber sub;
		json j;
		string r;
		switch (t)
		{
		case 1:			
			cout << "name:";
			cin >> bs.name;
			cout << endl;
			cout << "mcc:";
			cin >> bs.mcc;
			cout << endl; 
			cout << "mnc:";
			cin >> bs.mnc;
			cout << endl; 
			cout << "lac:";
			cin >> bs.lac;
			cout << endl; 
			cout << "cellid:";
			cin >> bs.cellid;
			cout << endl; 
			cout << "lat:";
			cin >> bs.lat;
			cout << endl; 
			cout << "lon:";
			cin >> bs.lon;
			cout << endl;
			bs.ip = "127.0.0.1";
			bs.port = DEFAULT_PORT;

			j = bs;
			r = "11" + j.dump(); 
			//Server::client.Start(8081);
			send(Server::client.sock, r.c_str(), strlen(r.c_str()), 0);
			strcpy_s(Server::recvbuf, "");
			recv(Server::client.sock, Server::recvbuf, Server::recvbuflen, 0);
			break;
		case 2:
			cout << "name:";
			cin >> bs.name;
			cout << endl;

			j = bs;
			r = "12" + j.dump();
			//Server::client.Start(8081);
			send(Server::client.sock, r.c_str(), strlen(r.c_str()), 0);
			strcpy_s(Server::recvbuf, "");
			recv(Server::client.sock, Server::recvbuf, Server::recvbuflen, 0);
			break;
		case 3:
			cout << "name:";
			cin >> bs.name;
			cout << endl;
			cout << "mcc:";
			cin >> bs.mcc;
			cout << endl;

			j = bs;
			r = "13" + j.dump();
			//Server::client.Start(8081);
			send(Server::client.sock, r.c_str(), strlen(r.c_str()), 0);
			strcpy_s(Server::recvbuf, "");
			recv(Server::client.sock, Server::recvbuf, Server::recvbuflen, 0);
			break;
		case 4:			
			cout << "imsi:";
			cin >> sub.imsi;
			cout << endl;
			cout << "imeiSV:";
			cin >> sub.imeiSV;
			cout << endl;
			cout << "bsName:";
			cin >> sub.bsName;
			cout << endl;
			cout << "subName:";
			cin >> sub.subName;
			cout << endl;
			sub.assistData = "1111";

			j = sub;
			r = "21" + j.dump();
			//Server::client.Start(8081);
			send(Server::client.sock, r.c_str(), strlen(r.c_str()), 0);
			strcpy_s(Server::recvbuf, "");
			recv(Server::client.sock, Server::recvbuf, Server::recvbuflen, 0);
			break;
		case 5:
			cout << "imsi:";
			cin >> sub.imsi;
			cout << endl;
			cout << "imeiSV:";
			cin >> sub.imeiSV;
			cout << endl;

			j = sub;
			r = "22" + j.dump();
			//Server::client.Start(8081);
			send(Server::client.sock, r.c_str(), strlen(r.c_str()), 0);
			strcpy_s(Server::recvbuf, "");
			recv(Server::client.sock, Server::recvbuf, Server::recvbuflen, 0);
			break;
		case 0:
			bs.ip = "127.0.0.1";
			bs.port = DEFAULT_PORT;
			j = bs;
			r = "4" + j.dump();
			//Server::client.Start(8081);
			send(Server::client.sock, r.c_str(), strlen(r.c_str()), 0);
			strcpy_s(Server::recvbuf, "");
			recv(Server::client.sock, Server::recvbuf, Server::recvbuflen, 0);
			return 0;
		default:
			break;
		}
	}

	return 0;
}