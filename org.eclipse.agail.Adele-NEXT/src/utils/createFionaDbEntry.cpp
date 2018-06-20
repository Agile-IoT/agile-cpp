#include <iostream>
#include <fstream>

#include "Storage.h"

const std::string usermail = "next-test@yopmail.com";
const std::string usrid2 = "335753f7ce12c345b45975b259fa3d9d";
const std::string avname = "Next";
const std::string avatarId = "864";

const std::string protocol = "http:";
const std::string smUrl = "sm.adelerobots.com";
const std::string defAioUrl = "aio0001.adelerobots.com";
const std::string rtmpServerUrl = "195.55.126.59";

int main () {
    // Open and parse the configuration file
    Json::Value conf;
    std::ifstream config_doc ("org.eclipse.agail.Adele-NEXT/conf/idm-entity-storage-conf.json", std::ifstream::binary);
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
    std::string entity_id = "fiona";
    std::string entity_type = "/client";
    std::string owner = "root";
    Json::Value data;
    data["usermail"] = usermail;
    data["usrid2"] = usrid2;
    data["avname"] = avname;
    data["avatarId"] = avatarId;
    data["protocol"] = protocol;
    data["smUrl"] = smUrl;
    data["defAioUrl"] = defAioUrl;
    data["rtmpServerUrl"] = rtmpServerUrl;
    int ret = storage.createEntity (entity_id, entity_type, owner, data);
    if (ret == 0)
        std::cout << "Created entity: " << AGAIL::Storage::jsonStringify (data) << std::endl;

    // Close database
    storage.disconnect ();
}
