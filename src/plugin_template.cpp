#include "./plugin_template.hpp"

#include <imgui.h>

float PluginTemplate::render(float delta) {
	if (ImGui::Begin("my imgui window")) {
		ImGui::Text("hello world");
	}
	ImGui::End();

	return 2.f;
}

