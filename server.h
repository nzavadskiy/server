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

struct BS
{
	string name;
	string mcc;
	string mnc;
	string lac;
	string cellid;
	string lat;
	string lon;
	string antenna;
	string ip;
	string port;
};
