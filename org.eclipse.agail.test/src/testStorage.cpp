#include "Storage.h"

#include <iostream>
#include <fstream>

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
    AGAIL::Storage storage (conf);
    std::string entity_id = "0000";
    std::string entity_type = "/tester";
    std::string owner = "me";
    Json::Value data;
    data["description"] = "This is a test";
    int ret = storage.createEntity (entity_id, entity_type, owner, data);
    if (ret == 0)
        std::cout << "Created entity: " << AGAIL::Storage::jsonStringify (data) << std::endl;

    // Read created entity
    ret = storage.readEntity (entity_id, entity_type, data);
    if (ret == 0)
        std::cout << "Read data: " << AGAIL::Storage::jsonStringify (data) << std::endl;

    // Update entity
    data["description"] = "This is a modified test";
    data["extra_field"] = "This is an added field";
    ret = storage.updateEntity (entity_id, entity_type, data);
    if (ret == 0)
        std::cout << "Updated entity: " <<AGAIL::Storage::jsonStringify (data) << std::endl;

    // List the whole table
    ret = storage.listEntitiesByEntityType (std::string(), data);
    if (ret == 0)
        std::cout << "Whole table content: " << AGAIL::Storage::jsonStringify (data) << std::endl;

    // Delete entity
    ret = storage.deleteEntity (entity_id, entity_type);
    if (ret == 0)
        std::cout << "Entity deleted" << std::endl;

    // Close database
    storage.disconnect ();
    std::cout << "Database closed successfully" << std::endl;
}
