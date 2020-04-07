#pragma comment (lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "8080"
#define ASCII_OFFSET 48
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

void to_json(json& j, const TA& p)
{ 
	j = json{
		{"imsi", p.imsi}, 
		{"imeiSV", p.imeiSV},
		{"servingBSName", p.servingBSName}, 
		{"servingBSTA", p.servingBSTA}, 
		{"servingBSLev", p.servingBSLev},
		{"neighbours",{
			{
				{"mcc", p.neighbourBS1.mcc}, {"mnc", p.neighbourBS1.mnc}, {"lac", p.neighbourBS1.lac},
				{"cellid", p.neighbourBS1.cellid}, {"antenna", p.neighbourBS1.antenna}
			},
			{
				{"mcc", p.neighbourBS2.mcc}, {"mnc", p.neighbourBS2.mnc}, {"lac", p.neighbourBS2.lac},
				{"cellid", p.neighbourBS2.cellid}, {"antenna", p.neighbourBS2.antenna}
			},
			{
				{"mcc", p.neighbourBS3.mcc}, {"mnc", p.neighbourBS3.mnc}, {"lac", p.neighbourBS3.lac},
				{"cellid", p.neighbourBS3.cellid}, {"antenna", p.neighbourBS3.antenna}
			},
			{
				{"mcc", p.neighbourBS4.mcc}, {"mnc", p.neighbourBS4.mnc}, {"lac", p.neighbourBS4.lac},
				{"cellid", p.neighbourBS4.cellid}, {"antenna", p.neighbourBS4.antenna}
			},
			{
				{"mcc", p.neighbourBS5.mcc}, {"mnc", p.neighbourBS5.mnc}, {"lac", p.neighbourBS5.lac},
				{"cellid", p.neighbourBS5.cellid}, {"antenna", p.neighbourBS5.antenna}
			},
			{
				{"mcc", p.neighbourBS6.mcc}, {"mnc", p.neighbourBS6.mnc}, {"lac", p.neighbourBS6.lac},
				{"cellid", p.neighbourBS6.cellid}, {"antenna", p.neighbourBS6.antenna}
			}
			}
		}
	};
}

//void from_json(const json& j, TA& p)
//{
//	j.at("imeiSV").get_to(p.imeiSV);
//	j.at("imsi").get_to(p.imsi);
//	j.at("ta").get_to(p.TA);
//	j.at("lev1").get_to(p.lev1);
//	j.at("lev2").get_to(p.lev2);
//	j.at("lev3").get_to(p.lev3);
//	j.at("lev4").get_to(p.lev4);
//	j.at("lev5").get_to(p.lev5);
//	j.at("lev6").get_to(p.lev6);
//	j.at("lev7").get_to(p.lev7);
//}

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
	string remoteIP = "";
	int remotePort = NULL;

	string Start()
	{
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			return "Socket creation error";
		}

		memset(&serv_addr, '0', sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(remotePort);

		// Convert IPv4 and IPv6 addresses from text to binary form
		if (inet_pton(AF_INET, remoteIP.c_str(), &serv_addr.sin_addr) <= 0)
		{
			return "Invalid address/ Address not supported ";
		}

		if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			return "\nConnection Failed";
		}

		return "";
	}

	bool SendReceiveMessage(string message)
	{
		if (remoteIP != "" && remotePort != NULL)
		{
			Start();
			send(sock, message.c_str(), strlen(message.c_str()), 0);
			char recvbuf[DEFAULT_BUFLEN];
			recv(sock, recvbuf, DEFAULT_BUFLEN, 0);
			string recvb = ((string)recvbuf).substr(0, 2);
			if (recvb == "90")
				return true;
			else if (recvb == "91")
				return false;
		}
		else return false;
	}
};

class Server
{
public:
	static SOCKET ClientSocket;
	static Client client;
	static SOCKET ListenSocket;
	static string localIP;
	static int localPort;

public:
	
	static void GetIpPort(string jString)
	{
		json j = json::parse(jString);
		localIP = j["localIP"].get<string>();
		client.remoteIP = j["remoteIP"].get<string>();
		localPort = j["localPort"].get<int>();
		client.remotePort = j["remotePort"].get<int>();
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
			try
			{
				BS request;
				request.ip = localIP;
				request.port = to_string(localPort);
				json j = request;
				string r = "0" + j.dump();
				client.Start();
				send(client.sock, r.c_str(), strlen(r.c_str()), 0);
				int iResult = listen(ListenSocket, SOMAXCONN);
				if (iResult == SOCKET_ERROR)
				{
					closesocket(ListenSocket);
					WSACleanup();
					return WSAGetLastError();
				}
				char initRecvBuf[DEFAULT_BUFLEN];
				recv(client.sock, initRecvBuf, DEFAULT_BUFLEN, 0);
				// Accept a client socket
				ClientSocket = accept(ListenSocket, NULL, NULL);
				if (ClientSocket == INVALID_SOCKET) {
					closesocket(ListenSocket);
					WSACleanup();
					return WSAGetLastError();
				}
				printf("Сервер запущен\n");
			}
			catch (exception e)
			{
				cout << e.what() << endl;
			}

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
				client.SendReceiveMessage(r);
			}
			int sendRes;
			int recvRes;
			do {
				char msgRecvBuf[DEFAULT_BUFLEN];
				strcpy_s(msgRecvBuf, "");
				printf("Ожидание сообщений\n");
				recvRes = recv(ClientSocket, msgRecvBuf, DEFAULT_BUFLEN, 0);
				string recvb = (string)msgRecvBuf;
				if (recvRes > 0)
				{
					int msgID = (int)msgRecvBuf[0] - ASCII_OFFSET;
					switch (msgID)
					{
						case 0:
						{
							closesocket(ClientSocket);
							WSACleanup();
							return WSAGetLastError();
						}
						case 1: // запрос на передачу rrlp-сообщения
						{
							// получение абонента от { до }
							string a = recvb.substr(1, recvb.find_last_of('}'));
							a.shrink_to_fit(); // чтоб работало
							json j_ = json::parse(a);
							Subscriber sub = j_.get<Subscriber>();
							sendRes = send(ClientSocket, "90", strlen("90"), 0);
							// заглушка на обработку сообщений
							string r = "";
							if (sub.assistData == "ms-based gps")
							{													
								sub.assistData = "e211ffffd53220b6413cc63c6a93b0030c8440b8ad10";
								json j = sub;
								r = "32" + j.dump();								
								client.SendReceiveMessage(r);
							}
							if (sub.assistData == "ms-based e-otd")
							{
								sub.assistData = "22041c";
								json j = sub;
								string r = "33" + j.dump();
								client.SendReceiveMessage(r);
							}
							if (sub.assistData == "ms-assisted gps")
							{
								sub.assistData = "22082202BD09C9CF3F37873E002D26F21738BB0066B4D2AE58770000";
								json j = sub;
								string r = "33" + j.dump();
								client.SendReceiveMessage(r);
							}
							if (sub.assistData == "ms-assisted e-otd")
							{
								sub.assistData = "22041c";
								json j = sub;
								string r = "33" + j.dump();
								client.SendReceiveMessage(r);
							}
							break;
						}
						case 2: //местоположение по ТА и уровням сигнала
						{
							// получение абонента
							string a = recvb.substr(1, recvb.find_last_of('}'));
							a.shrink_to_fit();
							json j_ = json::parse(a);
							Subscriber sub = j_.get<Subscriber>();  // got subscriber
							sendRes = send(ClientSocket, "90", strlen("90"), 0);							
							TA ta;
							ta.imsi = sub.imsi;
							ta.imeiSV = sub.imeiSV;
							ta.neighbourBS1.mcc = "1";
							ta.neighbourBS1.mnc = "1";
							// отправка данных
							json j = ta;
							string r = j.dump();
							r = "31" + r;
							client.SendReceiveMessage(r);
							break;
						}
						case 3: //местоположение по CELL ID + TA
						{
							//получение данных
							string a = recvb.substr(1, recvb.find_last_of('}'));
							a.shrink_to_fit();
							json j_ = json::parse(a);
							Subscriber sub = j_.get<Subscriber>();  // got subscriber
							sendRes = send(ClientSocket, "90", strlen("90"), 0);
							//заглушка						
							CellID cellid;
							cellid.imsi = sub.imsi;
							cellid.imeiSV = sub.imeiSV;
							cellid.dist = "1";
							json j = cellid;
							string r = "34" + j.dump();
							client.SendReceiveMessage(r);
							break;
						}
						case 4: // ошибка в добавлении БС: уже есть такое имя
						{
							string a = recvb.substr(1, recvb.find_last_of('}'));
							a.shrink_to_fit();
							json j_ = json::parse(a);
							BS bs = j_.get<BS>();
							cout << bs.name << endl;
							sendRes = send(ClientSocket, "90", strlen("90"), 0);
							break;
						}
						case 5: // ошибка в добавлении пользователя
						{
							string a = recvb.substr(2, recvb.find_last_of('}') - 1);
							a.shrink_to_fit();
							json j_ = json::parse(a);
							Subscriber sub = j_.get<Subscriber>();
							int itemID = (int)msgRecvBuf[1] - ASCII_OFFSET;
							switch (itemID) 
							{
								case 1:
								{
									cout << "Не существует БС с таким именем " << sub.bsName << endl;
									break;
								}
								case 2:
								{
									cout << "Уже есть пользователь с такими идентификаторами " << sub.imsi << " " << sub.imeiSV << endl;
									break;
								}

							}
							sendRes = send(ClientSocket, "90", strlen("90"), 0);
						}
						default:
						{
							//iSendResult = send(ClientSocket, "-1", strlen("-1"), 0);
						}
					}
					if (sendRes == SOCKET_ERROR)
					{
						closesocket(ClientSocket);
						WSACleanup();
						return WSAGetLastError();
					}
				}
				else if (recvRes == 0)
				{
					break;
				}
				else
				{
					closesocket(ClientSocket);
				}
			} while (recvRes > 0);
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
SOCKET Server::ListenSocket = INVALID_SOCKET;
Client Server::client;
string Server::localIP;
int Server::localPort;

int __cdecl main(void)
{
	setlocale(LC_ALL, "Russian");
	cout << "Отправка запроса ..." << endl;
	Server::SetSocket();
	DWORD thID;
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Server::ListenAndReceive, NULL, NULL, &thID);
	string p;

	while (true)
	{
		int t;
		printf("Enter command\n1. Add BS\n2. Remove BS\n3. Alter BS\n4. Add subscriber\n5. Remove subscriber\n0. Shut down\n");
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
			Server::client.SendReceiveMessage(r);
			break;
		case 2:
			cout << "name:";
			cin >> bs.name;
			cout << endl;

			j = bs;
			r = "12" + j.dump();
			Server::client.SendReceiveMessage(r);
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
			Server::client.SendReceiveMessage(r);
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
			Server::client.SendReceiveMessage(r);
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
			Server::client.SendReceiveMessage(r);
			break;
		case 0:
			bs.ip = Server::localIP;
			bs.port = to_string(Server::localPort);
			j = bs;
			r = "4" + j.dump();
			Server::client.SendReceiveMessage(r);
			return 0;
		default:
			break;
		}
	}

	return 0;
}