#pragma once
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class Names {
private:
	std::unordered_map<std::string, uint32_t> stringToId;
	std::vector<std::string> idToString;
	mutable std::shared_mutex mutex;

	Names() = default;

public:
	static Names& Get() {
		static Names instance;
		return instance;
	}

	// $O(1)$ String-to-ID lookup with Thread Safety
	uint32_t addName(std::string_view name) {
		std::unique_lock lock(mutex);

		std::string strName(name);
		auto it = stringToId.find(strName);
		if (it != stringToId.end()) {
			return it->second;
		}

		uint32_t id = static_cast<uint32_t>(idToString.size());
		idToString.push_back(strName);
		stringToId[strName] = id;
		return id;
	}

	// $O(1)$ ID-to-String lookup
	std::string getName(uint32_t index) const {
		std::shared_lock lock(mutex);
		if (index >= idToString.size()) return "";
		return idToString[index];
	}

	uint32_t getIndex(std::string_view name) const {
		std::shared_lock lock(mutex);
		auto it = stringToId.find(std::string(name));
		return (it != stringToId.end()) ? it->second : UINT32_MAX;
	}

	void clear() {
		std::unique_lock lock(mutex);
		stringToId.clear();
		idToString.clear();
	}
};