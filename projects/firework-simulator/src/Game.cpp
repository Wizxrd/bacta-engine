#include "Game.hpp"
#include "Fireworks.hpp"
#include "Globals.hpp"

#include <imgui.h>
#include <imgui-SFML.h>

static void DrawBurstShapeEditor(const char* label, ShellData::BurstShape& shape){
	int current = static_cast<int>(shape);
	const char* items[] = { "Sphere", "Waterfall", "Palm" };
	if (ImGui::Combo(label, &current, items, IM_ARRAYSIZE(items))){
		shape = static_cast<ShellData::BurstShape>(current);
	}
}

static void DrawEffectList(const char* label, std::vector<ShellData::Effect>& effects){
	if (ImGui::TreeNode(label)){
		for (int i = 0; i < static_cast<int>(effects.size()); i++){
			ImGui::PushID(i);
			int current = static_cast<int>(effects[i]);
			const char* items[] = { "Sparkle", "ZigZag", "Branch" };
			ImGui::SetNextItemWidth(160.f);
			if (ImGui::Combo("Effect", &current, items, IM_ARRAYSIZE(items))){
				effects[i] = static_cast<ShellData::Effect>(current);
			}
			ImGui::SameLine();
			if (ImGui::Button("Remove")){
				effects.erase(effects.begin() + i);
				ImGui::PopID();
				break;
			}
			ImGui::PopID();
		}

		if (ImGui::Button("Add Sparkle")){
			effects.push_back(ShellData::Effect::Sparkle);
		}
		ImGui::SameLine();
		if (ImGui::Button("Add ZigZag")){
			effects.push_back(ShellData::Effect::ZigZag);
		}
		ImGui::SameLine();
		if (ImGui::Button("Add Branch")){
			effects.push_back(ShellData::Effect::Branch);
		}

		ImGui::TreePop();
	}
}

static void DrawColorants(ShellData::Colorants& colorants){
	if (ImGui::TreeNode("Colorants")){
		ImGui::DragFloat("Strontium", &colorants.strontium, 0.1f, 0.f, 1000.f);
		ImGui::DragFloat("Barium", &colorants.barium, 0.1f, 0.f, 1000.f);
		ImGui::DragFloat("Copper", &colorants.copper, 0.1f, 0.f, 1000.f);
		ImGui::DragFloat("Sodium", &colorants.sodium, 0.1f, 0.f, 1000.f);
		ImGui::TreePop();
	}
}

static void DrawFuels(ShellData::Fuels& fuels){
	if (ImGui::TreeNode("Fuels")){
		ImGui::DragFloat("Charcoal", &fuels.charcoal, 0.1f, 0.f, 1000.f);
		ImGui::DragFloat("Sulfur", &fuels.sulfur, 0.1f, 0.f, 1000.f);
		ImGui::DragFloat("Aluminum", &fuels.aluminum, 0.1f, 0.f, 1000.f);
		ImGui::DragFloat("Magnesium", &fuels.magnesium, 0.1f, 0.f, 1000.f);
		ImGui::DragFloat("Titanium", &fuels.titanium, 0.1f, 0.f, 1000.f);
		ImGui::TreePop();
	}
}

static void DrawOxidizers(ShellData::Oxidizers& oxidizers){
	if (ImGui::TreeNode("Oxidizers")){
		ImGui::DragFloat("Potassium Nitrate", &oxidizers.potassiumNitrate, 0.1f, 0.f, 1000.f);
		ImGui::DragFloat("Potassium Perchlorate", &oxidizers.potassiumPerchlorate, 0.1f, 0.f, 1000.f);
		ImGui::DragFloat("Strontium Nitrate", &oxidizers.strontiumNitrate, 0.1f, 0.f, 1000.f);
		ImGui::TreePop();
	}
}

static void DrawComposition(ShellData::Composition& composition){
	if (ImGui::TreeNode("Composition")){
		DrawFuels(composition.fuels);
		DrawOxidizers(composition.oxidizers);
		DrawColorants(composition.colorants);
		ImGui::TreePop();
	}
}

static void DrawLiftCharge(ShellData::LiftCharge& liftCharge){
	if (ImGui::TreeNode("Lift Charge")){
		ImGui::DragFloat("Fuse", &liftCharge.fuse, 0.01f, 0.f, 60.f);
		DrawComposition(liftCharge.composition);
		DrawEffectList("Effects", liftCharge.effects);
		ImGui::TreePop();
	}
}

static void DrawBurstCharge(ShellData::BurstCharge& burstCharge, const char* label = "Burst Charge"){
	if (ImGui::TreeNode(label)){
		DrawBurstShapeEditor("Shape", burstCharge.shape);
		ImGui::DragFloat("Duration", &burstCharge.duration, 0.01f, 0.f, 60.f);
		DrawComposition(burstCharge.composition);
		DrawEffectList("Effects", burstCharge.effects);
		ImGui::TreePop();
	}
}

static void DrawAerialShellEditor(AerialShell& shell){
	DrawLiftCharge(shell.liftCharge);
	DrawBurstCharge(shell.burstCharge);
}

static AerialShell example = Fireworks::Example();

void Game::ProcessEvent(sf::window& window, const sf::Event& event){
	if (ImGui::GetIO().WantCaptureMouse){return;}
	if (event.is<sf::Event::MouseButtonPressed>()){
		bool launch = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Space);
		sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
		if (event.getIf<sf::Event::MouseButtonPressed>()->button == sf::Mouse::Button::Left){
			AerialShell copy = example;
			if (launch){
				mFireworks.emplace_back(copy);
				mFireworks.back().Launch({randomize(250, _G::WINDOW_WIDTH-250.f), _G::WINDOW_HEIGHT});
			} else{
				copy.liftCharge.state = ShellData::State::Completed;
				copy.burstCharge.state = ShellData::State::Detonated;
				mFireworks.emplace_back(copy);
				mFireworks.back().Detonate(mouse);
			}
		}
	}
}

void Game::UpdateImGui(sf::Time deltaTime){
	float dT = deltaTime.asSeconds();
	ImGui::Begin("Performance Monitor");
	ImGui::Text("FPS: %.f", 1.f / dT);
	ImGui::Text("%.fms", dT * 1000.f);
	std::size_t bytes = 0;
	for (const Firework& firework : mFireworks){
		bytes += firework.GetMemoryBytes();
	}
	float kb = static_cast<float>(bytes) / 1024.f;
	float mb = static_cast<float>(bytes) / (1024.f * 1024.f);
	ImGui::Separator();
	ImGui::Text("Fireworks: %zu", mFireworks.size());
	ImGui::Text("Memory: %zu bytes", bytes);
	ImGui::Text("Memory: %.2f KB", kb);
	ImGui::Text("Memory: %.2f MB", mb);
	ImGui::End();

	ImGui::Begin("Aerial Shell Editor");
	DrawAerialShellEditor(example);
	ImGui::End();
}

void Game::Update(sf::Time deltaTime){
	for (int i = 0; i < mFireworks.size(); i++){
		Firework& firework = mFireworks[i];
		firework.Update(deltaTime);
		if (firework.IsCompleted()){
			mFireworks[i] = std::move(mFireworks.back());
			mFireworks.pop_back();
			i--;
		}
	}
}

void Game::Render(sf::RenderTarget& target){
	for (Firework& firework : mFireworks){
		firework.Render(target);
	}
}