#include <assert.h>
#include <sstream>
#include "CoCycle.h"
#include "ogz_mng.h"
#include "code_converter.h"
#include "struct.h"

using namespace chaos;
using namespace std;

extern CoCycle *g_cycle;

COgzMng::COgzMng() {}

COgzMng::~COgzMng() {}

bool operator<(CityMapKey a, CityMapKey b)
{
	if(a.id < b.id)  return true;

	if(a.id == b.id) {
		if(a.systemID.compare(b.systemID) < 0)  return true;
		return false;
	}

	return false;
}

int COgzMng::init(OracleProcess *conn)
{
	CoCycle *cycle = g_cycle;

	if(conn == NULL)	return -1;

	mDBConn = conn;

	string sql = "";

	try
	{
		otl_stream osi;
		
		CityMapKey cityKey;
		stringstream ss ;
		ss << "select a.CITY_ID, a.PARENT_CITY_ID, a.CITY_NAME, c.HOST_TEL, a.HOST_TEL, a.ORGAN_ID,"
			<< "d.FLAG, d.ID, d.Name, a.LEAVE_WORD_TYPE, "
			<< "a.START_HOUR, a.END_HOUR, a.START_MINUTE, a.END_MINUTE, b.AREA_CODE"
			<< " from IM_SM_OPENDED_CITY a"
			<< " left join IM_SM_DISTRICT b on a.CITY_ID = b.ID"
			<< " left join IM_SM_HOST_TEL_CONFIGER c on a.HOST_TEL = c.ID"
			<< " left join IM_SM_VIEW_SET d on a.CITY_ID = d.PROVINCE_ID and d.FLAG=1"
			<< " where b.DISTRICT_LEVEL="
			<< DISTRICT_LEVEL_PROVINCE
			<< " and AVAIL=1";

		sql = ss.str();

		if(mDBConn->Select(sql, osi, 100) != 0) return -1;


		while( !osi.eof() )
		{

			City *city = cycle->mPool->newObj<City>();
			assert(city != NULL);

			osi >> city->id >> city->parentID >> city->name >> city->systemID >> city->hostTel;

			osi >> city->ogzID;
			
			if(osi.is_null()) {
				city->isOgz = false;
			} else {
				city->isOgz = true;
				
			}

			int flag = 0;
			
			osi >> flag ;
			if(osi.is_null() || flag == 0) {
				city->isBind = false;
				
			} else {
				city->isBind = true;
			}
			osi >> city->viewSetID >> city->viewSetName >> city->leaveWordType
				>> city->beginHour >> city->endHour >> city->beginMin >> city->endMin
				>> city->areaCode;
			city->viewID = 0;
			city->viewName = "";
			city->districtLevel = DISTRICT_LEVEL_PROVINCE;

			insertCity(city);
			if(city->isOgz) {	
				otl_stream osi3;

				ss.str("");
				ss << "select ID, NAME, ORGAN_ID"
					<< " from IM_SM_MANUL_SERVER_TYPE"
					<< " where enable=1 and OPENED_CITY_ID="   //ywy 2011-11-24 new (enable=1)
					<< city->id
					<< " and HOST_TEL="
					<< city->hostTel;

				sql = ss.str();
				if(mDBConn->Select(sql, osi3, 100) != 0) return -1;

				while( !osi3.eof() )
				{
					Attribute   * attr = cycle->mPool->newObj<Attribute>();
					assert(attr != NULL);

					osi3 >> attr->id >> attr->name >> attr->ogzID;
					
					cityKey.id = city->id;
					cityKey.systemID = city->systemID;
					insertAttribute(cityKey, attr);
				}
			}
		}
		
		otl_stream osi1;

		ss.str("");
		ss << "select ltrim(rtrim(a.CITY_NAME)), a.CITY_ID, a.PARENT_CITY_ID, a.HOST_TEL, a.ORGAN_ID, c.HOST_TEL,"
			<< "e.FLAG, d.ID, d.NAME, e.ID, e.NAME, a.LEAVE_WORD_TYPE, "
			<< "a.START_HOUR, a.END_HOUR, a.START_MINUTE, a.END_MINUTE, b.AREA_CODE"
			<< " from IM_SM_OPENDED_CITY a"
			<< " left join IM_SM_DISTRICT b on a.CITY_ID = b.ID"
			<< " left join IM_SM_HOST_TEL_CONFIGER c on a.HOST_TEL = c.ID"
			<< " left join IM_SM_VIEW e on a.CITY_ID = e.CITY_ID and e.FLAG=1"
			<< " left join IM_SM_VIEW_SET d on d.ID = e.VIEW_SET_ID"
			<< " where b.DISTRICT_LEVEL="
			<< DISTRICT_LEVEL_CITY
			<< " and AVAIL=1";

		sql = ss.str();
		if(mDBConn->Select(sql, osi1, 100) != 0) return -1;
		
		while( !osi1.eof() )
		{
			City *city = cycle->mPool->newObj<City>();
			assert(city != NULL);

			osi1 >> city->name >> city->id >> city->parentID >> city->hostTel >> city->ogzID >> city->systemID;

			int flag = 0;
			osi1 >> flag;
			
			if(osi1.is_null() || flag == 0) {
				city->isBind = false;
			} else {
				city->isBind = true;
			}
			
			osi1 >> city->viewSetID >> city->viewSetName >> city->viewID 
				>> city->viewName >> city->leaveWordType >> city->beginHour
				>> city->endHour >> city->beginMin >> city->endMin >> city->areaCode;

			insertCity(city);

			otl_stream osi2;

			ss.str("");
			ss << "select ID, NAME, ORGAN_ID"
				<< " from IM_SM_MANUL_SERVER_TYPE"
				<< " where enable=1 and OPENED_CITY_ID="      //ywy 2011-11-24 new (enable=1)
				<< city->id
				<< " and HOST_TEL="
				<< city->hostTel;

			sql = ss.str();
			if(mDBConn->Select(sql, osi2, 100) != 0) return -1;

			while( !osi2.eof() )
			{
				Attribute   * attr = cycle->mPool->newObj<Attribute>();
				assert(attr != NULL);

				osi2 >> attr->id >> attr->name >> attr->ogzID;
				cityKey.id = city->id;
				cityKey.systemID = city->systemID;

				insertAttribute(cityKey, attr);
			}

			city->isOgz = true;
			city->districtLevel = DISTRICT_LEVEL_CITY;
		}

		otl_stream	osiTXCityMap;

		ss.str("");
		ss << "select a.CITY_ID, a.IM_CITY_ID, a.TYPE, a.COMM, b.PARENT_ID, b.DISTRICT_LEVEL "
			<< " from IM_SM_DISTRICT_CONVERSION a"
			<< " left join IM_SM_DISTRICT b on a.IM_CITY_ID = b.ID";

		sql = ss.str();

		if(mDBConn->Select(sql, osiTXCityMap, 100) != 0) return -100;

		while( !osiTXCityMap.eof())
		{
			TXCityMap  *txCityMap = cycle->mPool->newObj<TXCityMap>();
			if(txCityMap == NULL)	return -101;
			
			osiTXCityMap >> txCityMap->txCityID >> txCityMap->cityID >> txCityMap->type 
				>> txCityMap->comm >> txCityMap->parentID >> txCityMap->districtLevel;

			insertTXCity(txCityMap);
		}

		otl_stream	osiAreaCodeMap;

		ss.str("");
		ss << "select a.ID, a.AREA_CODE,a.PARENT_ID, a.DISTRICT_LEVEL"
			<< " from IM_SM_DISTRICT a";

		sql = ss.str();

		if(mDBConn->Select(sql, osiAreaCodeMap, 100) != 0) return -100;

		string districtLevel;
		while( !osiAreaCodeMap.eof())
		{
			AreaCodeMap * areaCodeMap = cycle->mPool->newObj<AreaCodeMap>();
			if(areaCodeMap == NULL)	return -101;
			
			District * district = cycle->mPool->newObj<District>();
			if(district == NULL) return -102;

			osiAreaCodeMap >> areaCodeMap->cityID >> areaCodeMap->areaCode 
				>> areaCodeMap->parentID >> districtLevel;

			areaCodeMap->districtLevel = atoi(districtLevel.c_str());

			if(areaCodeMap->areaCode.length() > 0)
			{
				insertAreaCode(areaCodeMap);
			}

			district->id = areaCodeMap->cityID;
			district->parentID = areaCodeMap->parentID;
			district->districtLevel = areaCodeMap->districtLevel;

			insertDistrict(district);
		}
		otl_stream osiAttr;

		ss.str("");
		ss << "select ID, NAME, ORGAN_ID"
			<< " from IM_SM_MANUL_SERVER_TYPE";

		sql = ss.str();
		if(mDBConn->Select(sql, osiAttr, 100) != 0) return -1;

		while( !osiAttr.eof() )
		{
			Attribute   * attr = cycle->mPool->newObj<Attribute>();
			assert(attr != NULL);

			osiAttr >> attr->id >> attr->name >> attr->ogzID;
			insertAttribute(attr);
		}
	}
	catch(otl_exception& p)
	{ // intercept OTL exceptions

		cout<< "init view fail" << p.msg << p.stm_text<< p.var_info << endl;
		return -2;

	}

	return 0;
}

int COgzMng::insertCity(City *city)
{
	CityMapKey	key;
	key.id = city->id;
	key.systemID = city->systemID;
	mCityMap.insert(pair<CityMapKey, City *>(key, city));

	return 0;
}


int COgzMng::insertOgz(Ogz *ogz)
{
	mOgzMap.insert(pair<uint32_t, Ogz *>(ogz->id, ogz));
	return 0;
}

int COgzMng::insertAttribute(Attribute * attribute)
{
	mAttrMap.insert(pair<uint32_t, Attribute *>(attribute->id, attribute));
	return 0;
}

int COgzMng::insertAttribute(CityMapKey  cityKey, Attribute * attribute)
{
	City * city  = getCity(cityKey);

	if(city == NULL) return -1;

	city->attrMap.insert(pair<uint32_t, Attribute *>(attribute->id, attribute));
	return 0;
}

int COgzMng::insertTXCity(TXCityMap * txCityMap)
{
	mTXCityMap.insert(pair<uint32_t, TXCityMap *>(txCityMap->txCityID, txCityMap));
	return 0;
}

int COgzMng::insertAreaCode(AreaCodeMap * areaCodeMap)
{
//	string areaCode = "#" + areaCodeMap->areaCode;
	string areaCode = areaCodeMap->areaCode;
	mAreaCodeMap.insert(pair<string, AreaCodeMap *>(areaCode, areaCodeMap));
	return 0;
}

int COgzMng::insertDistrict(District * district)
{
	mDistrictMap.insert(pair<uint32_t, District *>(district->id, district));
	return 0;
}

int COgzMng::removeCity(CityMapKey  cityKey)
{
	map<CityMapKey, City *>::iterator iter;
	iter = mCityMap.find(cityKey);

	if(iter == mCityMap.end())	return -1;


	mCityMap.erase(iter);
	return 0;
}

int COgzMng::removeOgz(uint32_t ogzID)
{
	map<uint32_t, Ogz *>::iterator iter;
	iter = mOgzMap.find(ogzID);

	if(iter == mOgzMap.end()) return -1;

	mOgzMap.erase(iter);

	return 0;
}

int COgzMng::removeAttribute(uint32_t attrID)
{
	map<uint32_t, Attribute *>::iterator iter;

	iter = mAttrMap.find(attrID);

	if(iter == mAttrMap.end()) return -2;

	mAttrMap.erase(iter);

	return 0;
}

int COgzMng::removeAttribute(CityMapKey cityKey, uint32_t attrID)
{
	City *city = getCity(cityKey);

	if(city == NULL) return -1;

	map<uint32_t, Attribute *>::iterator iter;

	iter = city->attrMap.find(attrID);

	if(iter == city->attrMap.end()) return -2;

	city->attrMap.erase(iter);

	return 0;
}

int COgzMng::removeTXCity(uint32_t txCityID)
{
	map<uint32_t, TXCityMap *>::iterator iter;
	iter = mTXCityMap.find(txCityID);

	if(iter == mTXCityMap.end())	return -2;

	mTXCityMap.erase(iter);
	return 0;
}

int COgzMng::removeAreaCode(string areaCode)
{
	map<string, AreaCodeMap *>::iterator iter;
	iter = mAreaCodeMap.find(areaCode);
	if(iter == mAreaCodeMap.end()) return -2;

	mAreaCodeMap.erase(iter);
	return 0;
}

int COgzMng::removeDistrict(uint32_t districtID)
{
	map<uint32_t, District *>::iterator iter;
	iter = mDistrictMap.find(districtID);
	if(iter == mDistrictMap.end()) return -2;

	mDistrictMap.erase(iter);
	return 0;
}

City * COgzMng::getCity(CityMapKey	cityKey)
{
	map<CityMapKey, City *>::iterator iter;
	
	iter = mCityMap.find(cityKey);

	if(iter == mCityMap.end()) {
		District * district = getDistrict(cityKey.id);
		if(district == NULL) return NULL;
		if(district->districtLevel == COgzMng::DISTRICT_LEVEL_CITY) {
			cityKey.id = district->parentID;
			LOG(5, "find district:%d", cityKey.id);

			iter = mCityMap.find(cityKey);

			if(iter == mCityMap.end())	return NULL;
		} else return  NULL;

	}

	return iter->second;
}

AreaCodeMap * COgzMng::getCityIDByAreaCode(string areaCode) {
	map<string, AreaCodeMap *>::iterator iter;
	iter = mAreaCodeMap.find(areaCode);

	if(iter == mAreaCodeMap.end())	return NULL;

	return iter->second;
}

list<City *> COgzMng::getProvinceList(string systemID)
{
	list<City *>	provinceList;

	map<CityMapKey, City *>::iterator iter;

	City * city = NULL;
	for(iter = mCityMap.begin(); iter != mCityMap.end(); iter++)
	{
		city = iter->second;

		if(city->districtLevel == DISTRICT_LEVEL_PROVINCE 
				&& city->systemID.compare(systemID) == 0) {
			provinceList.push_back(city);
		}
	}

	return provinceList;
}

list<City *> COgzMng::getCityList(string systemID, uint32_t provinceID)
{
	list<City *>	cityList;

	map<CityMapKey, City *>::iterator iter;

	City * city = NULL;
	for(iter = mCityMap.begin(); iter != mCityMap.end(); iter++)
	{
		city = iter->second;

		if(city->districtLevel == DISTRICT_LEVEL_CITY 
				&& city->systemID.compare(systemID) == 0
				&& city->parentID == provinceID) {
			cityList.push_back(city);
		}
	}

	return cityList;
}

Ogz * COgzMng::getOgz(uint32_t ogzID)
{
	map<uint32_t, Ogz *>::iterator iter;
	iter = mOgzMap.find(ogzID);

	if(iter == mOgzMap.end()) return NULL;

	return iter->second;
}

Attribute * COgzMng::getAttribute(uint32_t attrID)
{
	
	map<uint32_t, Attribute *>::iterator iter;

	iter = mAttrMap.find(attrID);

	if(iter == mAttrMap.end()) return NULL;

	return iter->second;
}

Attribute * COgzMng::getAttribute(CityMapKey cityKey, uint32_t attrID)
{

	City * city = getCity(cityKey);

	if(city == NULL) return NULL;

	map<uint32_t, Attribute *>::iterator iter;

	iter = city->attrMap.find(attrID);

	if(iter == city->attrMap.end()) return NULL;

	return iter->second;
}

TXCityMap * COgzMng::getTXCity(uint32_t txCityID)
{
	map<uint32_t, TXCityMap *>::iterator iter;

	iter = mTXCityMap.find(txCityID);

	if(iter == mTXCityMap.end()) return NULL;

	return iter->second;
}

District * COgzMng::getDistrict(uint32_t districtID)
{
	map<uint32_t, District *>::iterator iter;

	iter = mDistrictMap.find(districtID);

	if(iter == mDistrictMap.end()) return NULL;
	return iter->second;
}
