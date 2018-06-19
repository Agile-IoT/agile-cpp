#include "Storage.h"

#include <iostream> // TODO: substitute by log library

AGAIL::Storage::Storage (Json::Value conf) {
    if (conf.isMember ("storage") && conf["storage"].isMember ("dbName"))
	this->conf = conf;
    else
	std::cout << "Error 500: Storage module not properly configured: " << jsonStringify (conf);
}

void AGAIL::Storage::disconnect () {
    entityConnectionPool.loadDb ("disconnect");
}

/////////////////////////////////////////////////////
// Functions that operate on the entities database //
/////////////////////////////////////////////////////

int AGAIL::Storage::createEntity (std::string entity_id, std::string entity_type, std::string owner, Json::Value& data) {
    return entityConnectionPool.loadDb (conf)->createEntityPromise (entity_id, entity_type, owner, data);
}

int AGAIL::Storage::readEntity (std::string entity_id, std::string entity_type, Json::Value& data) {
    return entityConnectionPool.loadDb (conf)->readEntityPromise (entity_id, entity_type, data);
}

int AGAIL::Storage::updateEntity (std::string entity_id, std::string entity_type, Json::Value& data) {
    return entityConnectionPool.loadDb (conf)->updateEntityPromise (entity_id, entity_type, data);
}

int AGAIL::Storage::deleteEntity (std::string entity_id, std::string entity_type) {
    return entityConnectionPool.loadDb (conf)->deleteEntityPromise (entity_id, entity_type);
}

///////////////////////////////////////////////////
// Functions that operate on the groups database //
///////////////////////////////////////////////////

int AGAIL::Storage::createGroup (std::string group_name, std::string owner) {
    return entityConnectionPool.loadDb (conf)->createGroupPromise (group_name, owner);
}

int AGAIL::Storage::readGroup (std::string group_name, std::string owner, Json::Value& data) {
    return entityConnectionPool.loadDb (conf)->readGroupPromise (group_name, owner, data);
}

int AGAIL::Storage::deleteGroup (std::string group_name, std::string owner) {
    return entityConnectionPool.loadDb (conf)->deleteGroupPromise (group_name, owner);
}

///////////////////////////////////////////
// Functions that operate on both tables //
///////////////////////////////////////////

int AGAIL::Storage::addEntityToGroup (std::string group_name, std::string owner, std::string entity_id, std::string entity_type) {
    return entityConnectionPool.loadDb (conf)->addEntityToGroupPromise (group_name, owner, entity_id, entity_type);
}

int AGAIL::Storage::removeEntityFromGroup (std::string group_name, std::string owner, std::string entity_id, std::string entity_type) {
    return entityConnectionPool.loadDb (conf)->removeEntityFromGroupPromise (group_name, owner, entity_id, entity_type);
}

/////////////////////////////////////////////
// Functions that list entities and groups //
/////////////////////////////////////////////

int AGAIL::Storage::listEntitiesByAttributeValueAndType (Json::Value attribute_constraints, std::string entity_type, Json::Value& data) {
    return entityConnectionPool.loadDb (conf)->listEntitiesByAttributeValueAndType (attribute_constraints, entity_type, data);
}

int AGAIL::Storage::listEntitiesByEntityType (std::string entity_type, Json::Value& data) {
    return entityConnectionPool.loadDb (conf)->listEntitiesByEntityType (entity_type, data);
}

int AGAIL::Storage::listGroups (Json::Value& data) {
    return entityConnectionPool.loadDb (conf)->listGroups (data);
}

/////////////
// Helpers //
/////////////

std::string AGAIL::Storage::jsonStringify (Json::Value data) {
    return AGAIL::LevelStorage::jsonStringify (data);
}

Json::Value AGAIL::Storage::stringJsonify (std::string value) {
    return AGAIL::LevelStorage::stringJsonify (value);
}
