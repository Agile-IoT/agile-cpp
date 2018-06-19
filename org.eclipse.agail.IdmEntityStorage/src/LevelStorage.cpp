#include "LevelStorage.h"

#include <iostream> // TODO: substitute by log library

int AGAIL::LevelStorage::init (Json::Value storageConf, std::function<void()> cb) {
    std::string filename = storageConf["dbName"].asString();
    if (entities != NULL || groups != NULL) {
	std::cout << "Error 500: Database already initialized";
	return 1;
    }
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, filename + "_entities", &entities);
    if (!status.ok()) {
        std::cout << "Error opening entities database: " << status.ToString() << std::endl;
        return 1;
    }
    status = leveldb::DB::Open(options, filename + "_groups", &groups);
    if (!status.ok()) {
        std::cout << "Error opening groups database: " << status.ToString() << std::endl;
        return 1; 
    }
    if (cb)
	cb();

    return 0;
}

void AGAIL::LevelStorage::close () {
    if (entities != NULL) {
	delete entities;
	entities = NULL;
    }
    if (groups != NULL) {
	delete groups;
	groups = NULL;
     }
}

/////////////////////////////////////////////////////
// Functions that operate on the entities database //
/////////////////////////////////////////////////////

int AGAIL::LevelStorage::createEntityPromise (std::string entity_id, std::string entity_type, std::string owner, Json::Value& data) {
    Json::Value pk = buildEntityPk (entity_id, entity_type);
    return createSomething ("entities", pk, owner, data, entities);
}

int AGAIL::LevelStorage::readEntityPromise (std::string entity_id, std::string entity_type, Json::Value& data) {
    Json::Value pk = buildEntityPk (entity_id, entity_type);
    return readSomething ("entities", pk, data, entities);
}

int AGAIL::LevelStorage::updateEntityPromise (std::string entity_id, std::string entity_type, Json::Value& data) {
    Json::Value pk = buildEntityPk (entity_id, entity_type);
    return updateSomething ("entities", pk, data, entities);
}

int AGAIL::LevelStorage::deleteEntityPromise (std::string entity_id, std::string entity_type) {
    std::cout << "Warning: group functionality not considered in deleteEntityPromise()!" << std::endl;
    Json::Value pk = buildEntityPk (entity_id, entity_type);
    return deleteSomething ("entities", pk, entities);
}
///////////////////////////////////////////////////
// Functions that operate on the groups database //
///////////////////////////////////////////////////

int AGAIL::LevelStorage::createGroupPromise (std::string group_name, std::string owner) {
    std::cout << "Warning: createGroupPromise() not implemented!" << std::endl;
    return 0;
}

int AGAIL::LevelStorage::readGroupPromise (std::string group_name, std::string owner, Json::Value& data) {
    std::cout << "Warning: readGroupPromise() not implemented!" << std::endl;
    return 0;
}

int AGAIL::LevelStorage::updateGroupPromise (std::string group_name, std::string owner, Json::Value data) {
    std::cout << "Warning: updateGroupPromise() not implemented!" << std::endl;
    return 0;
}

int AGAIL::LevelStorage::deleteGroupPromise (std::string group_name, std::string owner) {
    std::cout << "Warning: deleteGroupPromise() not implemented!" << std::endl;
    return 0;
}

///////////////////////////////////////////
// Functions that operate on both tables //
///////////////////////////////////////////

int AGAIL::LevelStorage::addEntityToGroupPromise (std::string group_name, std::string owner, std::string entity_id, std::string entity_type) {
    std::cout << "Warning: addEntityToGroupPromise() not implemented!" << std::endl;
    return 0;
}

int AGAIL::LevelStorage::removeEntityFromGroupPromise (std::string group_name, std::string owner, std::string entity_id, std::string entity_type) {
    std::cout << "Warning: removeEntityFromGroupPromise() not implemented!" << std::endl;
    return 0;
}

/////////////////////////////////////////////
// Functions that list entities and groups //
/////////////////////////////////////////////

int AGAIL::LevelStorage::listEntitiesByAttributeValueAndType (Json::Value attribute_constraints, std::string entity_type, Json::Value& data) {
    std::cout << "Warning: listEntitiesByAttributeValueAndType() not implemented!" << std::endl;
    return 0;
}

int AGAIL::LevelStorage::listEntitiesByEntityType (std::string entity_type, Json::Value& data) {
    std::function<bool (Json::Value)> keyAction = [entity_type] (Json::Value key) {
	if (entity_type.empty()) return true;
	else return entity_type == key["type"].asString(); };
    std::function<bool (Json::Value)> dataAction = [] (Json::Value data) {
	return true; };
    return iterateDbAndSearch("entities", keyAction, dataAction, data, entities);
}

int AGAIL::LevelStorage::listGroups (Json::Value& data) {
    std::cout << "Warning: listGroups() not implemented!" << std::endl;
    return 0;
}

int AGAIL::LevelStorage::listEntitiesByGroup (std::string group_name, std::string owner, Json::Value& data) {
    std::cout << "Warning: listEntitiesByGroup() not implemented!" << std::endl;
    return 0;
}

int AGAIL::LevelStorage::iterateDbAndSearch (std::string action_type, std::function<bool (Json::Value)> keyAction, std::function<bool (Json::Value)> dataAction, Json::Value& results, leveldb::DB* database) {
    if (database == NULL) {
        std::cout << "Error 500: Call without a database to iterateDbAndSearch()" << std::endl;
        return 1;
    }
    Json::Value key;
    Json::Value data;
    Json::Value output;
    leveldb::Iterator* it = database->NewIterator (leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
	key = stringJsonify (it->key().ToString());
	data = stringJsonify (it->value().ToString());
	if (keyAction (key) && dataAction (data))
	    output.append (buildReturnObject (key, data, std::string()));
    }
    delete it;
    results = output;

    return 0;
}

//////////////////////////////////////////////////
// Functions that operate on the given database //
//////////////////////////////////////////////////

int AGAIL::LevelStorage::createSomething (std::string action_type, Json::Value pk, std::string owner, Json::Value& data, leveldb::DB* database) {
    if (database == NULL) {
	std::cout << "Error 500: Call without a database to createSomething()" << std::endl;
	return 1;
    }
    std::string value;
    leveldb::Status status = database->Get(leveldb::ReadOptions(), jsonStringify (pk), &value);
    if (!status.IsNotFound()) {
        std::cout << "Error 409: " << action_type << " with pk " << jsonStringify(pk) << " already exists" << std::endl;
        return 1;
    }
    Json::Value entity = buildReturnObject (pk, data, owner);
    status = database->Put(leveldb::WriteOptions(), jsonStringify (pk), jsonStringify (entity));
    if (!status.ok()) {
        std::cout << "Error 500: Cannot write " << action_type << " with pk " << jsonStringify(pk) << ": " << status.ToString() << std::endl;
        return 1;
    }
    status = database->Get(leveldb::ReadOptions(), jsonStringify (pk), &value);
    if (!status.ok()) {
        std::cout << "Error 500: Cannot read " << action_type << " with pk " << jsonStringify(pk) << ": " << status.ToString() << std::endl;
        return 1;
    }
    data = stringJsonify (value);
    return 0;
}

int AGAIL::LevelStorage::readSomething (std::string action_type, Json::Value pk, Json::Value& data, leveldb::DB* database) {
    if (database == NULL) {
        std::cout << "Error 500: Call without a database to readSomething()" << std::endl;
        return 1;
    }
    std::string value;
    leveldb::Status status = database->Get(leveldb::ReadOptions(), jsonStringify (pk), &value);
    if (status.IsNotFound()) {
        std::cout << "Error 404: " << action_type << " with pk " << jsonStringify(pk) << " not found" << std::endl;
        return 1;
    }
    if (!status.ok()) {
        std::cout << "Error 500: Cannot read " << action_type << " with pk " << jsonStringify(pk) << ": " << status.ToString() << std::endl;
        return 1;
    }
    data = stringJsonify (value);
    return 0;
}

int AGAIL::LevelStorage::updateSomething (std::string action_type, Json::Value pk, Json::Value& data, leveldb::DB* database) {
    if (database == NULL) {
        std::cout << "Error 500: Call without a database to updateSomething()" << std::endl;
        return 1;
    }
    std::string value;
    leveldb::Status status = database->Get(leveldb::ReadOptions(), jsonStringify (pk), &value);
    if (status.IsNotFound()) {
        std::cout << "Error 404: " << action_type << " with pk " << jsonStringify(pk) << " not found" << std::endl;
        return 1;
    }
    if (!status.ok()) {
        std::cout << "Error 500: Cannot read " << action_type << " with pk " << jsonStringify(pk) << ": " << status.ToString() << std::endl;
        return 1;
    }
    Json::Value readData = stringJsonify (value);
    Json::Value entity = buildReturnObject (pk, data, readData["owner"].asString());
    status = database->Put(leveldb::WriteOptions(), jsonStringify (pk), jsonStringify (entity));
    if (!status.ok()) {
        std::cout << "Error 500: Cannot update " << action_type << " with pk " << jsonStringify(pk) << ": " << status.ToString() << std::endl;
        return 1;
    }
    status = database->Get(leveldb::ReadOptions(), jsonStringify (pk), &value);
    if (!status.ok()) {
        std::cout << "Error 500: Cannot read " << action_type << " with pk " << jsonStringify(pk) << ": " << status.ToString() << std::endl;
        return 1;
    }
    data = stringJsonify (value);
    return 0;
}

int AGAIL::LevelStorage::deleteSomething (std::string action_type, Json::Value pk, leveldb::DB* database) {
    if (database == NULL) {
        std::cout << "Error 500: Call without a database to deleteSomething()" << std::endl;
        return 1;
    }
    std::string value;
    leveldb::Status status = database->Get(leveldb::ReadOptions(), jsonStringify (pk), &value);
    if (status.IsNotFound()) {
        std::cout << "Error 404: " << action_type << " with pk " << jsonStringify(pk) << " not found" << std::endl;
        return 1;
    }
    if (!status.ok()) {
        std::cout << "Error 500: Cannot read " << action_type << " with pk " << jsonStringify(pk) << ": " << status.ToString() << std::endl;
        return 1;
    }
    status = database->Delete(leveldb::WriteOptions(), jsonStringify (pk));
    if (!status.ok()) {
        std::cout << "Error 500: Cannot delete " << action_type << " with pk " << jsonStringify(pk) << ": " << status.ToString() << std::endl;
        return 1;
    }
    return 0;
}

/////////////
// Helpers //
/////////////

Json::Value AGAIL::LevelStorage::buildEntityPk (std::string entity_id, std::string entity_type) {
    Json::Value entityPk;
    entityPk["id"] = entity_id;
    entityPk["type"] = entity_type;
    return entityPk;
}

Json::Value AGAIL::LevelStorage::buildGroupPk (std::string group_name, std::string owner) {
    Json::Value groupPk;
    groupPk["group_name"] = group_name;
    groupPk["owner"] = owner;
    return groupPk;
}

Json::Value AGAIL::LevelStorage::getPk (std::string action_type, Json::Value data) {
    Json::Value pk;

    if (action_type == "entities") {
	pk["id"] = data["id"];
	pk["type"] = data["type"];
    }
    else if (action_type == "groups") {
	pk["group_name"] = data["group_name"];
	pk["owner"] = data["owner"];
    }
    else
	std::cout << "Error 500: programmer mistake... attemting to get pk for unknown entity " << action_type << std::endl;

    return pk;
}

Json::Value AGAIL::LevelStorage::buildReturnObject (Json::Value pk, Json::Value data, std::string owner) {
    for (Json::Value::iterator it = pk.begin(); it != pk.end(); it++)
        data[it.key().asString()] = *it;
    if (!owner.empty())
	data["owner"] = owner;
    return data;
}

std::string AGAIL::LevelStorage::jsonStringify (Json::Value data) {
    Json::FastWriter writer;
    writer.omitEndingLineFeed ();
    return writer.write (data);
}

Json::Value AGAIL::LevelStorage::stringJsonify (std::string value) {
    Json::Value data;
    Json::Reader jsonReader;
    jsonReader.parse (value, data);
    return data;
}
