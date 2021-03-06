//one bs

#pragma comment (lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "8080"
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

using json = nlohmann::json;
using namespace std;

struct Subscriber
{
	string imsi;
	string imeiSV;
	string assistData;
};

struct Geolocation
{
	string imsi;
	string imeiSV;
	string latitude;
	string longtitude;
	string date;
};

struct TA
{
	string imsi;
	string imeiSV;
	string TA;
	string lev1;
	string lev2;
	string lev3;
	string lev4;
	string lev5;
	string lev6;
	string lev7;
};

struct CellID
{
	string imsi;
	string imeiSV;
	string mcc;
	string mnc;
	string lac;
	string cellid;
};

void to_json(json& j, const Subscriber& p)
{
	j = json{ {"imeiSV", p.imeiSV}, {"imsi", p.imsi}, {"assistData", p.assistData} };
}

void from_json(const json& j, Subscriber& p)
{
	j.at("imeiSV").get_to(p.imeiSV);
	j.at("imsi").get_to(p.imsi);
	j.at("assistData").get_to(p.assistData);
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
	j = json{ {"imeiSV", p.imeiSV}, {"imsi", p.imsi} , {"ta", p.TA}, {"lev1", p.lev1}, {"lev2", p.lev2}, {"lev3", p.lev3}, {"lev4", p.lev4}, {"lev5", p.lev5}, {"lev6", p.lev6}, {"lev7", p.lev7} };
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
	j = json{ {"imeiSV", p.imeiSV}, {"imsi", p.imsi} , {"mcc", p.mcc}, {"mnc", p.mnc} , {"lac", p.lac}, {"cellid", p.cellid}};
}

void from_json(const json& j, CellID& p)
{
	j.at("imeiSV").get_to(p.imeiSV);
	j.at("imsi").get_to(p.imsi);
	j.at("mcc").get_to(p.mcc);
	j.at("mnc").get_to(p.mnc);
	j.at("lac").get_to(p.lac);
	j.at("cellid").get_to(p.cellid);
}

class Client
{
public:
	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char buffer[1024] = { 0 };

	string Start(int port)
	{
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			return "Socket creation error";
		}

		memset(&serv_addr, '0', sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(port);

		// Convert IPv4 and IPv6 addresses from text to binary form 
		if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
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
	SOCKET ClientSocket = INVALID_SOCKET;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	SOCKET ListenSocket = INVALID_SOCKET;
	Client client;

public:
	
	int SetSocket()
	{
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
		iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
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

	int ListenAndReceive()
	{
		while (true)
		{
			int iResult = listen(ListenSocket, SOMAXCONN);
			if (iResult == SOCKET_ERROR)
			{
				closesocket(ListenSocket);
				WSACleanup();
				return WSAGetLastError();
			}

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
			do {
				strcpy_s(recvbuf, "");
				iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
				string recvb = (string)recvbuf;
				if (iResult > 0)
				{
					if (recvb[0] == '1') //запрос на получение списка абонентов
					{
						int port = atoi(recvb.substr(1).c_str());
						iSendResult = send(ClientSocket, "0", strlen("0"), 0);
						client.Start(8081);
						send(client.sock, "0", strlen("0"), 0);
						recv(client.sock, recvbuf, recvbuflen, 0);
						//заглушка со списком абонентов
						for (int i = 0; i < 3; i++)
						{
							string imsi = "25001";
							for (int j = 0; j < 10; j++)
							{
								uniform_int_distribution<int> distribution(0, 9);
								std::random_device rd;
								default_random_engine generator(rd());
								imsi += to_string(distribution(generator));
							}
							imsi = "250013250525243";
							string imeisv = "";
							for (int j = 0; j < 16; j++)
							{
								uniform_int_distribution<int> distribution(0, 9);
								std::random_device rd;
								default_random_engine generator(rd());
								imeisv += to_string(distribution(generator));
							}
							Subscriber s;
							if (i % 3 == 0)
								s = { imsi, imeisv, "0011" };
							if (i % 3 == 1)
								s = { imsi, imeisv, "0010" };
							if (i % 3 == 2)
								s = { imsi, imeisv, "1111" };
							json j = s;
							string r = "2" + j.dump();
							for (int i = 0; i < 5; i++)
							{
								client.Start(port);
								send(client.sock, r.c_str(), strlen(r.c_str()), 0);
								recv(client.sock, recvbuf, recvbuflen, 0);
							}
						}
						// отправка абонента
						// 2 - в содержимом - classmark
						// 3 или 7 - в содержимом - rrlp
						/*Subscriber s; // забить параметрами
						json j = s;
						string r = "2" + j.dump();
						client.Start(port);
						send(client.sock, r.c_str(), strlen(r.c_str()), 0);
						recv(client.sock, recvbuf, recvbuflen, 0);*/
					}
					else if (recvbuf[0] == '2') // запрос на передачу rrlp-сообщения
					{
						// получение абонента от { до }
						string a = recvb.substr(1, recvb.find_last_of('}'));
						a.shrink_to_fit(); // чтоб рботало
						json j_ = json::parse(a);
						Subscriber sub = j_.get<Subscriber>();
						iSendResult = send(ClientSocket, "0", strlen("0"), 0);
						// заглушка на обработку сообщений
						if (sub.assistData == "ms-based gps")
						{
							if (positions.size() != 0)
							{
								sub.assistData = positions.back();
								positions.pop_back();
							}
							else
							{
								sub.assistData = "e211ffffd53220b6413cc63c6a93b0030c8440b8ad10";
							}
							json j = sub;
							string r = "3" + j.dump();
							client.Start(8081);
							send(client.sock, r.c_str(), strlen(r.c_str()), 0);
							strcpy_s(recvbuf, "");
							recv(client.sock, recvbuf, recvbuflen, 0);
						}
						if (sub.assistData == "ms-based e-otd")
						{
							sub.assistData = "22041c";
							json j = sub;
							string r = "7" + j.dump();
							client.Start(8081);
							send(client.sock, r.c_str(), strlen(r.c_str()), 0);
							strcpy_s(recvbuf, "");
							recv(client.sock, recvbuf, recvbuflen, 0);
						}
						if (sub.assistData == "ms-assisted gps")
						{
							sub.assistData = "22082202BD09C9CF3F37873E002D26F21738BB0066B4D2AE58770000";
							json j = sub;
							string r = "7" + j.dump();
							client.Start(8081);
							send(client.sock, r.c_str(), strlen(r.c_str()), 0);
							strcpy_s(recvbuf, "");
							recv(client.sock, recvbuf, recvbuflen, 0);
						}
						if (sub.assistData == "ms-assisted e-otd")
						{
							sub.assistData = "22041c";
							json j = sub;
							string r = "7" + j.dump();
							client.Start(8081);
							send(client.sock, r.c_str(), strlen(r.c_str()), 0);
							strcpy_s(recvbuf, "");
							recv(client.sock, recvbuf, recvbuflen, 0);
						}
						// получение rrlp-ответа
						// разобрать сообщение и выяснить ms-assisted или ms-based
						// если ms-assisted - 7, если ms-based - 3
						// перенаправить ответ
						/*json j = sub;
						string r = "7" + j.dump(); // для ms-assisted
						string r = "3" + j.dump(); // для ms-based
						client.Start(8081);
						send(client.sock, r.c_str(), strlen(r.c_str()), 0);
						strcpy_s(recvbuf, "");
						recv(client.sock, recvbuf, recvbuflen, 0);
					}*/
					// не нужно
					/*if (recvb[0] == '3')
					{
						string a = recvb.substr(1, recvb.find_last_of('}'));
						a.shrink_to_fit();
						json j_ = json::parse(a);
						cout << j_ << endl;
						Subscriber sub = j_.get<Subscriber>();
						iSendResult = send(ClientSocket, "0", strlen("0"), 0);*/
				}
				if (recvb[0] == '5') //местоположение по ТА
				{
					// получение абонента
					string a = recvb.substr(1, recvb.find_last_of('}'));
					a.shrink_to_fit();
					json j_ = json::parse(a);
					Subscriber sub = j_.get<Subscriber>();  // got subscriber
					iSendResult = send(ClientSocket, "0", strlen("0"), 0);
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
					string r = "5" + j.dump();
					client.Start(8081);
					send(client.sock, r.c_str(), strlen(r.c_str()), 0);
					strcpy_s(recvbuf, "");
					recv(client.sock, recvbuf, recvbuflen, 0);
				}
				if (recvb[0] == '6') //местоположение по CELL ID
				{
					//получение данных
					string a = recvb.substr(1, recvb.find_last_of('}'));
					a.shrink_to_fit();
					json j_ = json::parse(a);
					cout << j_ << endl;
					Subscriber sub = j_.get<Subscriber>();  // got subscriber
					iSendResult = send(ClientSocket, "0", strlen("0"), 0);
					//заглушка
					uniform_int_distribution<int> distribution(0, 9);
					std::random_device rd;
					default_random_engine generator(rd());
					CellID c;
					//string lac = "";
					//for (int i = 0; i < 4; i++)
					//{
					//	c.lac += to_string(distribution(generator));
					//}
					//string cellid = "";
					//for (int i = 0; i < 4; i++)
					//{
					//	c.cellid += to_string(distribution(generator));
					//}
					// отправка данных
					c.lac = "1002";
					c.cellid = "1";
					c.imsi = sub.imsi;
					c.imeiSV = sub.imeiSV;
					c.mcc = "250";
					c.mnc = "02";
					json j = c;
					string r = "6" + j.dump();
					client.Start(8081);
					send(client.sock, r.c_str(), strlen(r.c_str()), 0);
					strcpy_s(recvbuf, "");
					recv(client.sock, recvbuf, recvbuflen, 0);
				}
				// удаление абонента
				// делается отдельно и не привязывается к запросу клиента
					/*json j__;
					j__.push_back(sub);
					string r_ = "4" + j__.dump();
					client.Start(8081);
					send(client.sock, r_.c_str(), strlen(r_.c_str()), 0);
					strcpy_s(recvbuf, "");
					recv(client.sock, recvbuf, recvbuflen, 0);*/
				else
				{
					//iSendResult = send(ClientSocket, "-1", strlen("-1"), 0);
				}
				if (iSendResult == SOCKET_ERROR)
				{
					closesocket(ClientSocket);
					WSACleanup();
					//return WSAGetLastError();
					SetSocket();
					ListenAndReceive();
				}
			}
			else if (iResult == 0)
			{
				break;
			}
			else
			{
				closesocket(ClientSocket);
				WSACleanup();
				//break;
				SetSocket();
				ListenAndReceive();
			}
		
			} while (iResult > 0);
		}
	}

	int ShutDown()
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

int __cdecl main(void)
{
	cout << "Server is working ..." << endl;
	Server server;
	server.SetSocket();
	server.ListenAndReceive();
	server.ShutDown();
	return 0;
}