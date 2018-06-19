#ifndef __AGAIL_STORAGE_H__
#define __AGAIL_STORAGE_H__

#include "EntityConnectionPool.h"

namespace AGAIL {
    class Storage;
}

class AGAIL::Storage {
    public:
	Storage (Json::Value conf);

	void disconnect ();

	// Functions that operate on the entities database
	int createEntity (std::string entity_id, std::string entity_type, std::string owner, Json::Value& data);
	int readEntity (std::string entity_id, std::string entity_type, Json::Value& data);
	int updateEntity (std::string entity_id, std::string entity_type, Json::Value& data);
	int deleteEntity (std::string entity_id, std::string entity_type);

        // Functions that operate on the groups database
	int createGroup (std::string group_name, std::string owner);
        int readGroup (std::string group_name, std::string owner, Json::Value& data);
        //int updateGroup (std::string group_name, std::string owner, Json::Value data);
        int deleteGroup (std::string group_name, std::string owner);

        // Functions that operate on both tables
	int addEntityToGroup (std::string group_name, std::string owner, std::string entity_id, std::string entity_type);
        int removeEntityFromGroup (std::string group_name, std::string owner, std::string entity_id, std::string entity_type);

	// Functions that list entities and groups
	int listEntitiesByAttributeValueAndType (Json::Value attribute_constraints, std::string entity_type, Json::Value& entities);
        int listEntitiesByEntityType (std::string entity_type, Json::Value& entities);
        int listGroups (Json::Value& groups);
        //int listEntitiesByGroup (std::string group_name, std::string owner, Json::Value& entities);
	
	static std::string jsonStringify (Json::Value data);
	static Json::Value stringJsonify (std::string value);

    private:
	Json::Value conf;
	AGAIL::EntityConnectionPool entityConnectionPool;
};

#endif
