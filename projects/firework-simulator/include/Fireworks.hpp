#pragma once

#include "AerialShell.hpp"

namespace Fireworks{
	inline AerialShell Example(){
		AerialShell shell{};
		shell.liftCharge.state = ShellData::State::Idle;
		shell.liftCharge.fuse = 1.5f;
		shell.liftCharge.lifetime = shell.liftCharge.fuse;
		shell.liftCharge.composition.fuels.charcoal = 200.f;
		shell.liftCharge.composition.fuels.sulfur = 150.f;
		shell.liftCharge.composition.oxidizers.potassiumNitrate = 250.f;
		shell.liftCharge.composition.oxidizers.potassiumPerchlorate = 250.f;
		shell.liftCharge.composition.colorants.copper = 250.f;

		shell.burstCharge.shape = ShellData::BurstShape::Palm;
		shell.burstCharge.state = ShellData::State::Idle;
		shell.burstCharge.duration = 6.f;
		shell.burstCharge.lifetime = shell.burstCharge.duration;
		shell.burstCharge.composition.colorants.strontium = 25.f;
		shell.burstCharge.composition.colorants.barium = 5.f;
		shell.burstCharge.composition.fuels.aluminum = 250.f;
		shell.burstCharge.composition.oxidizers.potassiumNitrate = 250.f;
		shell.burstCharge.effects.push_back(ShellData::Effect::Sparkle);
		return shell;
	}
};
