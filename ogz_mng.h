#ifndef __OGZ_MNG_H
#define __OGZ_MNG_H

#include <string>
#include <map>
#include <list>
#include "oracleprocess.h"
#include "worker_struct.h"

struct CityMapKey {
	uint32_t id;
	std::string systemID;
};

struct TXCityMap {
	uint32_t		cityID;
	uint32_t		txCityID;
	uint32_t		type;
	string			comm;
	uint32_t		parentID;
	std::string		districtLevel;
};

struct AreaCodeMap {
	uint32_t		cityID;
	std::string		areaCode;
	uint32_t    districtLevel;
	uint32_t	parentID;
};

typedef struct {
	uint32_t    id;
	string  name;
	uint32_t	ogzID;
}Attribute;

typedef struct {
	uint32_t	id;
	uint32_t	parentID;
	uint32_t	ogzID;
	std::string	name;
	std::string	systemID;
	uint32_t	hostTel;
	bool		isBind;
	uint32_t	viewSetID;
	uint32_t	viewID;
	std::string	viewSetName;
	std::string	viewName;
	uint32_t		leaveWordType;
	uint32_t	districtLevel;
	bool		isOgz;
	map<uint32_t, Attribute *>		attrMap;
	uint32_t	beginHour;
	uint32_t	beginMin;
	uint32_t	endHour;
	uint32_t	endMin;
	std::string	areaCode;
}City;

typedef struct {
	uint32_t	id;
	std::string		name;
	map<uint32_t, Attribute *>		attrMap;
}Ogz;

typedef struct {
	uint32_t	id;
	uint32_t	parentID;
	uint32_t	districtLevel;
}District;

class COgzMng
{
	public:
		COgzMng();
		~COgzMng();

		enum {
			DISTRICT_LEVEL_PROVINCE = 2,
			DISTRICT_LEVEL_CITY = 3,
		};


		int init(OracleProcess *conn);

		int insertCity(City *city);
		int insertOgz(Ogz *ogz);
		int insertAttribute(Attribute *attribute);
		int insertAttribute(CityMapKey cityKey, Attribute *attribute);
		int insertTXCity(TXCityMap * txCityMap);
		int insertAreaCode(AreaCodeMap * areaCodeMap);
		int insertDistrict(District * district);

		int removeCity(CityMapKey cityKey);
		int removeOgz(uint32_t ogzID);
		int removeAttribute(uint32_t attrID);
		int removeAttribute(CityMapKey cityKey, uint32_t attrID);

		int removeTXCity(uint32_t txCityID);
		int removeAreaCode(std::string areaCode);
		int removeDistrict(uint32_t districtID);

		std::list<City *> getProvinceList(string systemID);
		std::list<City *> getCityList(string systemID, uint32_t provinceID);
		City * getCity(CityMapKey cityKey);
		AreaCodeMap *  getCityIDByAreaCode(std::string areaCode);
		Ogz * getOgz(uint32_t ogzID);
		Attribute * getAttribute(uint32_t attrID);
		Attribute * getAttribute(CityMapKey cityKey, uint32_t attrID);

		TXCityMap * getTXCity(uint32_t txCityID);
		District	* getDistrict(uint32_t districtID);

	public:
		map<CityMapKey, list<City *> >	mProvinceListMap;
		map<CityMapKey, City *>		mCityMap;
		map<uint32_t, Ogz *>	mOgzMap;
		map<uint32_t, TXCityMap *>	mTXCityMap;
		map<uint32_t, Attribute *>	mAttrMap;

		map<std::string, AreaCodeMap * > mAreaCodeMap;

		map<uint32_t, District *>	mDistrictMap;

		OracleProcess			*mDBConn;
};

#endif


