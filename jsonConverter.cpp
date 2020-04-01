//#include <json.hpp>
//#include "server.h"
//
//using json = nlohmann::json;
//
//void to_json(json& j, const Subscriber& p)
//{
//	j = json{
//		{"imeiSV", p.imeiSV}, {"imsi", p.imsi},
//		{"assistData", p.assistData}, { "bsName", p.bsName }, { "subName", p.subName }
//	};
//}
//
//void from_json(const json& j, Subscriber& p)
//{
//	j.at("imeiSV").get_to(p.imeiSV);
//	j.at("imsi").get_to(p.imsi);
//	j.at("assistData").get_to(p.assistData);
//	j.at("bsName").get_to(p.bsName);
//	j.at("subName").get_to(p.subName);
//}
//
//void to_json(json& j, const Geolocation& p)
//{
//	j = json{ {"imeiSV", p.imeiSV}, {"imsi", p.imsi} , {"latitude", p.latitude}, {"longtitude", p.longtitude}, {"date", p.date} };
//}
//
//void from_json(const json& j, Geolocation& p)
//{
//	j.at("imeiSV").get_to(p.imeiSV);
//	j.at("imsi").get_to(p.imsi);
//	j.at("latitude").get_to(p.latitude);
//	j.at("longtitude").get_to(p.longtitude);
//	j.at("date").get_to(p.date);
//}
//
//void to_json(json& j, const TA& p)
//{
//	j = json{
//		{"imeiSV", p.imeiSV}, {"imsi", p.imsi} ,
//		{"ta", p.TA}, {"lev1", p.lev1}, {"lev2", p.lev2},
//		{"lev3", p.lev3}, {"lev4", p.lev4}, {"lev5", p.lev5},
//		{"lev6", p.lev6}, {"lev7", p.lev7}
//	};
//}
//
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
//
//void to_json(json& j, const CellID& p)
//{
//	j = json{
//		{"mcc", p.mcc}, {"mnc", p.mnc}, {"lac", p.lac},
//		{"cellid", p.cellid}, {"ip", p.ip}, {"port", p.port}
//	};
//}
//
//void from_json(const json& j, CellID& p)
//{
//	j.at("mcc").get_to(p.mcc);
//	j.at("mnc").get_to(p.mnc);
//	j.at("lac").get_to(p.lac);
//	j.at("cellid").get_to(p.cellid);
//	j.at("ip").get_to(p.ip);
//	j.at("port").get_to(p.port);
//}
//
//void to_json(json& j, const BS& p)
//{
//	j = json{
//		{"name", p.name}, {"mcc", p.mcc}, {"mnc", p.mnc}, {"lac", p.lac}, {"cellid", p.cellid},
//		{"lat", p.lat} , {"lon", p.lon} , {"antenna", p.antenna},
//		{"ip", p.ip} , {"port", p.port}
//	};
//}
//
//void from_json(const json& j, BS& p)
//{
//	j.at("name").get_to(p.name);
//	j.at("mcc").get_to(p.mcc);
//	j.at("mnc").get_to(p.mnc);
//	j.at("lac").get_to(p.lac);
//	j.at("cellid").get_to(p.cellid);
//	j.at("lat").get_to(p.lat);
//	j.at("lon").get_to(p.lon);
//	j.at("antenna").get_to(p.antenna);
//	j.at("ip").get_to(p.ip);
//	j.at("port").get_to(p.port);
//}