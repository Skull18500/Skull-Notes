#pragma once
#include "log.h"
#include "names.h"
#include <any>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class EventSystem {
public:
	// Listener struct allowing per-listener multithreading flags
	struct Listener {
		std::function<void(const std::any&)> callback;
		bool allowMultithreaded = false;
	};

	struct Event {
		// Id is for name index
		unsigned int ID;
		bool isMultithreaded = false;

		std::vector<Listener> listeners;
	};

	// Get singleton instance
	static EventSystem& Get() {
		static EventSystem instance;
		return instance;
	}

private:
	std::unordered_map<std::string, unsigned int> nameToEventID;
	std::vector<Event> events;

	// Private constructor
	EventSystem() = default;

public:
	// Create and register an event with optional multithreading flag
	unsigned int createEvent(const std::string& eventName, bool isMultithreaded = false) {

		// add a name for it and get its location
		unsigned int nameid = Names::Get().addName(eventName);

		Event newEvent = {};
		newEvent.ID = nameid;
		newEvent.isMultithreaded = isMultithreaded;

		// actually add event
		events.push_back(newEvent);

		// calculate location in array
		unsigned int id = static_cast<unsigned int>(events.size() - 1);

		nameToEventID[eventName] = id;

		printlog("Event created: " + eventName);
		printlog("Event ID: " + std::to_string(id));
		return id;
	}

	// Register a listener to an event with payload support and optional multithreading
	void bindListener(unsigned int id, std::function<void(const std::any&)> listener, bool allowMultithreaded = false) {
		if (id >= events.size()) {
			printlog("[Warning] Invalid event ID: " + std::to_string(id));
			return;
		}
		events[id].listeners.push_back({ listener, allowMultithreaded });
	}

	// Overload if the dev loses the ID. Just for convenience
	void bindListener(const std::string& name, std::function<void(const std::any&)> listener, bool allowMultithreaded = false) {
		auto it = nameToEventID.find(name);
		if (it == nameToEventID.end()) {
			printlog("[Warning] Event not found: " + name);
			return;
		}
		events[it->second].listeners.push_back({ listener, allowMultithreaded });
	}

	// Trigger an event with an optional payload
	void triggerEvent(unsigned int id, const std::any& payload = {}) {

		if (id >= events.size()) {
			printlog("[Warning] Invalid event ID: " + std::to_string(id));
			return;
		}

		const Event& it = events[id];

		for (size_t i = 0; i < it.listeners.size(); i++) {
			const auto& listener = it.listeners[i];

			// If the event itself OR this specific listener allows multithreading
			if (it.isMultithreaded || listener.allowMultithreaded) {
				std::thread([listener, payload]() {
					listener.callback(payload);
					}).detach();
			}
			else {
				// Synchronous main-thread execution
				listener.callback(payload);
			}
		}
	}

	// Convenient overload to trigger by name
	void triggerEvent(const std::string& name, const std::any& payload = {}) {
		auto it = nameToEventID.find(name);
		if (it == nameToEventID.end()) {
			printlog("[Warning] Event not found: " + name);
			return;
		}
		triggerEvent(it->second, payload);
	}

	// Clear all events and listeners
	void clear() {
		events.clear();
		nameToEventID.clear();
	}
};