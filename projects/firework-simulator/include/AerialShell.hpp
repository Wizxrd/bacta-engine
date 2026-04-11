#pragma once

#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>

#include <vector>

struct ShellData{
	enum class State{
		Idle,
		Terminal,
		Detonated,
		Completed
	};

	enum class BurstShape{
		Sphere,
		Waterfall,
		Palm
	};
	enum class Effect{
		Sparkle,
		ZigZag,
		Branch
	};

	struct Colorants{
		float strontium = 0.f;   // red
		float barium = 0.f;      // green
		float copper = 0.f;      // blue
		float sodium = 0.f;      // yellow
	};

	struct Fuels{
		float charcoal = 0.f;
		float sulfur = 0.f;
		float aluminum = 0.f;
		float magnesium = 0.f;
		float titanium = 0.f;
	};

	struct Oxidizers{
		float potassiumNitrate = 0.f;
		float potassiumPerchlorate = 0.f;
		float strontiumNitrate = 0.f;
	};

	struct Composition{
		Fuels fuels;
		Oxidizers oxidizers{};
		Colorants colorants{};
	};

	struct Physics{
		sf::Vector2f velocity{};
		float mass = 0.f;
		float drag = 0.f;
	};

	struct Charge{
		State state;
		Physics physics;
		Composition composition{};
		sf::VertexArray trail{};
		std::vector<Effect> effects{};
		float lifetime = 0.f;
	};

	struct LiftCharge : Charge{
		float fuse = 0.f;
	};

	struct BurstCharge : Charge{
		BurstShape shape;
		float duration = 0.f;
	};
};

struct AerialShell{

	ShellData::LiftCharge liftCharge;
	ShellData::BurstCharge burstCharge;
	std::vector<ShellData::BurstCharge> burstNodes{};
};