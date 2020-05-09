#pragma once
#include <string>

using namespace std;

struct CellID
{
	string imsi;
	string imeiSV;
	string dist;
};

struct Subscriber
{
	string imsi;
	string imeiSV;
	string assistData;
	string bsName;
	string subName;
};

struct Geolocation
{
	string imsi;
	string imeiSV;
	string latitude;
	string longtitude;
	string date;
};

struct BS
{
	string name;
	string mcc;
	string mnc;
	string lac;
	string cellid;
	string lat;
	string lon;
	string alt;
	string accuracy;
	string antenna;
	string ip;
	string port;
};

struct TA
{
	string imsi;
	string imeiSV;
	string servingBSName;
	string servingBSTA;
	string servingBSLev;
	BS neighbourBS1;
	BS neighbourBS2;
	BS neighbourBS3;
	BS neighbourBS4;
	BS neighbourBS5;
	BS neighbourBS6;
};