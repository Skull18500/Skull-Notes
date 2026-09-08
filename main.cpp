#include "Systems/eventsystem.h"
#include "Systems/log.h"
#include "Systems/renderer.h"


void loop() {
	while (Renderer::Get().is_running()) {
		/*
		Build note-taking UI here between event processing and rendering.
		Check for file changes, periodically save(if changed)
		Trigger async tasks.
		*/
		Renderer::Get().process_events();
		Renderer::Get().render();
	}
}

int main() {
	// Initialize the renderer
	if (!Renderer::Get().init()) {
		printlog("Failed to initialize renderer");
		return -1;
	}

	loop();
	Renderer::Get().quit();
	printlog("Application closed");

	return 0;
}