#pragma once
#include "Systems/log.h"
#include <cstdint>
#include <functional>
#include <SDL3/SDL.h>
#include <vector>

class UI_element {
private:
	//because ideally the events would never change, but just be static Dear ImGui calls
	static std::vector<std::function<void>> events;
	bool isVisible = true;
	uint32_t id = 0;
public:
	static std::vector<std::function<void>> get_events() {
		return events;
	};
	bool get_isVisible() {
		return isVisible;
	};
	bool set_isVisible(bool new_isVisible) {
		isVisible = new_isVisible;
		return isVisible;
	};
	void set_id(uint32_t new_id) {
		id = new_id;
	};
};

class Renderer {
private:
	// Private constructor
	Renderer() = default;
	std::vector<UI_element> elements;

public:
	bool init() {
		/*
		Initialize the SDL3 renderer
		Initialize list of objects and stuff
		Initialize imgui
		Return true if successful, false otherwise
		*/
		printlog("Initializing renderer");

		SDL_Init();


	};

	static Renderer& Get() {
		static Renderer instance;
		return instance;
	};

	void add_element(const UI_element& element) {
		elements.push_back(element);
		elements.back().set_id(elements.size());
	};
	void render() {
		for (auto element : elements) {
			if (element.get_isVisible()) {
				// Render the element
				for (const auto& event : element.get_events()) {
					event();
				};
			};
		};
	};
};