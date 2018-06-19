#include "EntityConnectionPool.h"

#include <iostream> // TODO: substitute by log library

AGAIL::LevelStorage* AGAIL::EntityConnectionPool::loadDb (Json::Value conf) {
    if (conf.isString () and conf.asString () == "disconnect") {
	std::cout << "Closing database object" << std::endl;
	if (db != NULL) {
	    db->close();
	    delete db;
	    db = NULL;
        }
	return NULL;
    }
    else if (db != NULL) {
	return db;
    }
    else if (!conf.isMember ("storage")) {
	std::cout << "Error: cannot find field of name \"storage\" in configuration file" << std::endl;
	return NULL;
    }
    else {
	std::cout << "Opening database object" << std::endl;
	db = new LevelStorage ();
	db->init (conf["storage"], NULL);
	return db;
    }
}
