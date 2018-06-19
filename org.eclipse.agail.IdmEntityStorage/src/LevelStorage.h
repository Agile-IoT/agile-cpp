#ifndef __AGAIL_LEVELSTORAGE_H__
#define __AGAIL_LEVELSTORAGE_H__

#include <functional>

#include <leveldb/db.h>
#include <jsoncpp/json/json.h>

namespace AGAIL {
    class LevelStorage;
}

class AGAIL::LevelStorage {
    public:
	int init (Json::Value storageConf, std::function<void()> cb); // create tables and prepare statements
	void close ();

	// Functions that operate on the entities database
	int createEntityPromise (std::string entity_id, std::string entity_type, std::string owner, Json::Value& data); // inserts an entity in the database
	int readEntityPromise (std::string entity_id, std::string entity_type, Json::Value& data); // reads an entity from the database
	int updateEntityPromise (std::string entity_id, std::string entity_type, Json::Value& data); // updates the attributes of an entity in the database
	int deleteEntityPromise (std::string entity_id, std::string entity_type); // deletes an entity from the database

	// Functions that operate on the groups database
	int createGroupPromise (std::string group_name, std::string owner);
	int readGroupPromise (std::string group_name, std::string owner, Json::Value& data);
	int updateGroupPromise (std::string group_name, std::string owner, Json::Value data);
	int deleteGroupPromise (std::string group_name, std::string owner);

	// Functions that operate on both tables
	int addEntityToGroupPromise (std::string group_name, std::string owner, std::string entity_id, std::string entity_type);
	int removeEntityFromGroupPromise (std::string group_name, std::string owner, std::string entity_id, std::string entity_type);

	// Functions that list entities and groups
	int listEntitiesByAttributeValueAndType (Json::Value attribute_constraints, std::string entity_type, Json::Value& entities); // attribute_constraints is an array of objects with the following properties: attribute_type, attribute_value
	int listEntitiesByEntityType (std::string entity_type, Json::Value& entities);
	int listGroups (Json::Value& groups);
	int listEntitiesByGroup (std::string group_name, std::string owner, Json::Value& entities);

	static std::string jsonStringify (Json::Value data);
	static Json::Value stringJsonify (std::string value);

    private:
	leveldb::DB* entities = NULL;
	leveldb::DB* groups = NULL;

	// Functions that operate on the given database
	int createSomething (std::string action_type, Json::Value pk, std::string owner, Json::Value& data, leveldb::DB* database); // creates an object in a given level db connection (action_type)
	int readSomething (std::string action_type, Json::Value pk, Json::Value& data, leveldb::DB* database); // reads an object in a given level db connection (action_type)
	int updateSomething (std::string action_type, Json::Value pk, Json::Value& data, leveldb::DB* database); // updates an object in a given level db connection (action_type)
	int deleteSomething (std::string action_type, Json::Value pk, leveldb::DB* database); //deletes an object in a given level db connection (action_type)

	// Function to create lists of entities or groups
	// If keyAction and dataAction functions return true for an element in the database, that element is added to the list
	int iterateDbAndSearch (std::string action_type, std::function<bool(Json::Value)> keyAction, std::function<bool(Json::Value)> dataAction, Json::Value& results, leveldb::DB* database);

	// Helpers to build and print pks
        Json::Value buildEntityPk (std::string entity_id, std::string entity_type);
        Json::Value buildGroupPk (std::string group_name, std::string owner);
	Json::Value getPk (std::string action_type, Json::Value data);

	// Helpers to return object with pk+entity data
	// This function considers that owner can be null, e.g. when called for a group. However, the owner is already part of the entity.
	//Therefore, the result always contains an owner.
	Json::Value buildReturnObject (Json::Value pk, Json::Value data, std::string owner);
};

#endif
