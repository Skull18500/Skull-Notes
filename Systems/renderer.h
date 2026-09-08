#pragma once
#include "includes/imgui_impl_sdlrenderer3.h"
#include "Systems/log.h"
#include <cstdint>
#include <functional>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <string>
#include <vector>

class UI_element {
private:
	//because ideally the events would never change, but just be static Dear ImGui calls
	inline static std::vector<std::function<void()>> events;
	bool isVisible = true;
	uint32_t id = 0;
public:
	static const std::vector<std::function<void()>>& get_events() {
		return events;
	};
	bool get_isVisible() const {
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
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	SDL_Window* window = nullptr;
	SDL_Renderer* sdlRenderer = nullptr;
	std::vector<UI_element> elements;
	bool running = false;
	bool initialized = false;

	void shutdown() {
		if (!initialized) {
			return;
		}

		printlog("Shutting down renderer");
		ImGui_ImplSDLRenderer3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();

		if (sdlRenderer != nullptr) {
			SDL_DestroyRenderer(sdlRenderer);
			sdlRenderer = nullptr;
		}
		if (window != nullptr) {
			SDL_DestroyWindow(window);
			window = nullptr;
		}
		SDL_Quit();
		initialized = false;
	}

public:
	~Renderer() {
		shutdown();
	}

	bool init() {
		printlog("Initializing renderer");

		if (initialized) {
			printlog("Renderer is already initialized");
			return true;
		}

		if (!SDL_Init(SDL_INIT_VIDEO)) {
			printlog(std::string("SDL initialization failed: ") + SDL_GetError());
			return false;
		}

		window = SDL_CreateWindow("Note App", 1280, 720, SDL_WINDOW_RESIZABLE);
		if (window == nullptr) {
			printlog(std::string("SDL window creation failed: ") + SDL_GetError());
			SDL_Quit();
			return false;
		}

		sdlRenderer = SDL_CreateRenderer(window, nullptr);
		if (sdlRenderer == nullptr) {
			printlog(std::string("SDL renderer creation failed: ") + SDL_GetError());
			SDL_DestroyWindow(window);
			window = nullptr;
			SDL_Quit();
			return false;
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGuiStyle& style = ImGui::GetStyle();
		// Corner Rounding & Spacing
		style.WindowRounding = 8.0f;
		style.FrameRounding = 6.0f;
		style.PopupRounding = 6.0f;
		style.GrabRounding = 4.0f;
		style.WindowBorderSize = 1.0f;
		style.FrameBorderSize = 0.0f;
		style.ItemSpacing = ImVec2(10, 10);
		style.WindowPadding = ImVec2(14, 14);

		// Dark Charcoal / Slate Palette
		ImVec4* colors = style.Colors;
		colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f); // #141417
		colors[ImGuiCol_ChildBg] = ImVec4(0.11f, 0.11f, 0.13f, 1.00f); // #1C1C21 (Cards/Panels)
		colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.18f, 0.18f, 0.21f, 1.00f); // Subtle card borders
		colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.21f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
		colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.94f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);

		if (!ImGui_ImplSDL3_InitForSDLRenderer(window, sdlRenderer) || !ImGui_ImplSDLRenderer3_Init(sdlRenderer)) {
			printlog("ImGui initialization failed");
			ImGui_ImplSDLRenderer3_Shutdown();
			ImGui_ImplSDL3_Shutdown();
			ImGui::DestroyContext();
			SDL_DestroyRenderer(sdlRenderer);
			sdlRenderer = nullptr;
			SDL_DestroyWindow(window);
			window = nullptr;
			SDL_Quit();
			return false;
		}

		initialized = true;
		running = true;
		printlog("Renderer initialized");
		return true;
	};

	static Renderer& Get() {
		static Renderer instance;
		return instance;
	};

	bool is_running() const {
		return running;
	};

	void process_events() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT) {
				printlog("Quit event received");
				quit();
			}
		}
	};

	void quit() {
		if (running) {
			printlog("Renderer quit requested");
		}
		running = false;
	};

	void add_element(const UI_element& element) {
		elements.push_back(element);
		elements.back().set_id(elements.size());
	};
	void render() {
		if (!initialized) {
			return;
		}

		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		for (const auto& element : elements) {
			if (element.get_isVisible()) {
				// Render the element
				for (const auto& event : element.get_events()) {
					event();
				};
			};

			ImGui::Render();
			SDL_SetRenderDrawColor(sdlRenderer, 25, 25, 25, 255);
			SDL_RenderClear(sdlRenderer);
			ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), sdlRenderer);
			SDL_RenderPresent(sdlRenderer);
		};
	};
};