#include <iostream>
#include <fstream>

#include "EntityConnectionPool.h"

int main () {
    // Open and parse the configuration file
    Json::Value conf;
    std::ifstream config_doc ("org.eclipse.agail.test/conf/idm-entity-storage-conf.json", std::ifstream::binary);
    if (!config_doc.is_open()) {
        std::cout << "Error opening the conf file" << std::endl;
        return 1;
    }
    Json::Reader jsonReader;
    bool jsonError = jsonReader.parse (config_doc, conf);
    if (!jsonError) {
        std::cout << "Error reading the conf file: " << jsonReader.getFormattedErrorMessages() << std::endl;
        return 1;
    }

    // Create entity
    AGAIL::EntityConnectionPool entityConnectionPool;
    std::string entity_id = "0000";
    std::string entity_type = "/tester";
    std::string owner = "me";
    Json::Value data;
    data["description"] = "This is a test";
    int ret = entityConnectionPool.loadDb (conf)->createEntityPromise (entity_id, entity_type, owner, data);
    if (ret == 0)
        std::cout << "Created entity: " << AGAIL::LevelStorage::jsonStringify (data) << std::endl;

    // Read created entity
    ret = entityConnectionPool.loadDb (conf)->readEntityPromise (entity_id, entity_type, data);
    if (ret == 0)
        std::cout << "Read data: " << AGAIL::LevelStorage::jsonStringify (data) << std::endl;

    // Update entity
    data["description"] = "This is a modified test";
    data["extra_field"] = "This is an added field";
    ret = entityConnectionPool.loadDb (conf)->updateEntityPromise (entity_id, entity_type, data);
    if (ret == 0)
        std::cout << "Updated entity: " <<AGAIL::LevelStorage::jsonStringify (data) << std::endl;

    // List the whole table
    ret = entityConnectionPool.loadDb (conf)->listEntitiesByEntityType (std::string(), data);
    if (ret == 0)
        std::cout << "Whole table content: " << AGAIL::LevelStorage::jsonStringify (data) << std::endl;

    // Delete entity
    ret = entityConnectionPool.loadDb (conf)->deleteEntityPromise (entity_id, entity_type);
    if (ret == 0)
        std::cout << "Entity deleted" << std::endl;

    // Close database
    entityConnectionPool.loadDb (Json::Value ("disconnect"));
    std::cout << "Database closed successfully" << std::endl;
}
