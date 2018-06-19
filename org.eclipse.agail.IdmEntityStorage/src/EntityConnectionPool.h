#ifndef __AGAIL_ENTITYCONNECTIONPOOL_H__
#define __AGAIL_ENTITYCONNECTIONPOOL_H__

#include "LevelStorage.h"

namespace AGAIL {
    class EntityConnectionPool;
}

class AGAIL::EntityConnectionPool {
    public:
	AGAIL::LevelStorage* loadDb (Json::Value conf);

    private:
	AGAIL::LevelStorage* db = NULL;
};

#endif
