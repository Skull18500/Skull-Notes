#include "Systems/eventsystem.h"
#include "Systems/log.h"
#include "Systems/renderer.h"


void loop() {
	bool shouldQuit = false;
	while (shouldQuit == false) {
		/*
		Poll events, render, and check for quit event
		Check for file changes, periodically save(if changed)
		Trigger async tasks.
		*/
	}

}

int main() {
	// Initialize the renderer
	if (!Renderer::Get().init()) {
		printlog("Failed to initialize renderer");
		return -1;
	}
	//create the begin event
	EventSystem::Get().createEvent("begin");
	//create the quit event
	EventSystem::Get().createEvent("quit");

	return 0;
}