/**
 * @file
 */

#include "MapProvider.h"
#include "Map.h"
#include "io/Filesystem.h"
#include "core/Log.h"

namespace backend {

MapProvider::MapProvider(const io::FilesystemPtr& filesystem, const core::EventBusPtr& eventBus) :
		_filesystem(filesystem), _eventBus(eventBus) {
}

MapPtr MapProvider::map(MapId id) const {
	auto i = _maps.find(id);
	if (i == _maps.end()) {
		return MapPtr();
	}
	return i->second;
}

MapProvider::Maps MapProvider::worldMaps() const {
	return _maps;
}

bool MapProvider::init() {
	const MapPtr& map = std::make_shared<Map>(1, _eventBus);
	if (!map->init(_filesystem)) {
		Log::warn("Failed to init map %i", map->id());
		return false;
	}
	_maps.insert(std::make_pair(1, map));
	return true;
}

void MapProvider::shutdown() {
	_maps.clear();
}

}