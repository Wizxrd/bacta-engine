#include "Firework.hpp"
#include "Globals.hpp"

static sf::PrimitiveType getPrimitiveType(ShellData::BurstShape shape){
	switch (shape){
		case ShellData::BurstShape::Palm:
			return sf::PrimitiveType::LineStrip;
		case ShellData::BurstShape::Waterfall:
			return sf::PrimitiveType::LineStrip;
		case ShellData::BurstShape::Sphere:
			return sf::PrimitiveType::Points;
	}
}

static std::size_t getVertexCount(ShellData::BurstShape shape){
	switch (shape){
		case ShellData::BurstShape::Palm:
			return static_cast<std::size_t>(50);
		case ShellData::BurstShape::Waterfall:
			return static_cast<std::size_t>(200);
		case ShellData::BurstShape::Sphere:
			return static_cast<std::size_t>(25);
	}
}

static sf::Color getColor(const ShellData::Composition& composition){
	const auto& c = composition.colorants;
	const auto& f = composition.fuels;
	sf::Vector3f color = {0.f, 0.f, 0.f};
	float total = 0.f;
	auto addColor = [&](float amount, sf::Vector3f tint){
		color.x += tint.x * amount;
		color.y += tint.y * amount;
		color.z += tint.z * amount;
		total += amount;
	};
	addColor(c.strontium, {1.0f, 0.12f, 0.12f});
	addColor(c.barium,    {0.15f, 1.0f, 0.15f});
	addColor(c.copper,    {0.10f, 0.35f, 1.0f});
	addColor(c.sodium,    {1.0f, 0.85f, 0.10f});
	addColor(f.aluminum * 0.2f,  {1.0f, 1.0f, 1.0f});
	addColor(f.magnesium * 0.15f,{1.0f, 1.0f, 1.0f});
	addColor(f.titanium * 0.1f,  {1.0f, 1.0f, 1.0f});
	if (total > 0.f){
		color /= total;
	}
	float brightness = randomize(0.92f, 1.08f);
	color *= 255.f * brightness;
	return sf::Color(
		static_cast<std::uint8_t>(std::clamp(color.x, 0.f, 255.f)),
		static_cast<std::uint8_t>(std::clamp(color.y, 0.f, 255.f)),
		static_cast<std::uint8_t>(std::clamp(color.z, 0.f, 255.f))
	);
}

void Firework::Launch(sf::Vector2f origin){
	float angle = randomize(-10.f, 10.f);
	float radians = angle * (M_PI / 180.f);
	float power = getExplosivePower(mAerialShell.liftCharge.composition);
	mAerialShell.liftCharge.lifetime = mAerialShell.liftCharge.fuse;
	mAerialShell.liftCharge.physics.mass = getMass(mAerialShell.liftCharge.composition);
	mAerialShell.liftCharge.physics.drag = _G::DRAG;
	mAerialShell.liftCharge.physics.velocity = {
		std::sin(radians) * power,
		-std::cos(radians) * randomize(power-100.f, power+100.f)
	};
	mAerialShell.liftCharge.trail = sf::VertexArray(sf::PrimitiveType::Points, 100);
	sf::Color color = getColor(mAerialShell.liftCharge.composition);
	std::cout << (int)color.r << ", " << (int)color.g << ", " << (int)color.b << '\n';
	for (int i = 0; i < mAerialShell.liftCharge.trail.getVertexCount(); i++){
		mAerialShell.liftCharge.trail[i].position = origin;
		mAerialShell.liftCharge.trail[i].color = color;
	}
	mAerialShell.liftCharge.state = ShellData::State::Terminal;
	std::cout << power << '\n';
}

void Firework::Detonate(sf::Vector2f origin){
	int _ = 6.f;
	if (mAerialShell.burstCharge.shape == ShellData::BurstShape::Sphere){
		_ = 1.f;
	}
	for (int i = 0; i < 360; i+=_){
		double radians = i * (M_PI / 180.f);
		ShellData::BurstCharge burst = mAerialShell.burstCharge;
		float power = getExplosivePower(burst.composition);
		burst.lifetime = burst.duration;
		burst.physics.mass = getMass(burst.composition);
		burst.physics.drag = randomize(_G::DRAG, clamp(_G::DRAG+0.05f, .995f));
		float _t = randomize(0.f, 1.f);
		const float distance = _t * _t * power;
		const sf::Vector2f direction = { std::cos(radians), std::sin(radians) };
		if (burst.shape == ShellData::BurstShape::Sphere){
			burst.physics.velocity = direction *randomize(1.f, power/2.f);
		} else {
			burst.physics.velocity = {
				std::cos(radians) * randomize(power*.75f, power*1.25f),
				std::sin(radians) * randomize(power*.75f, power*1.25f)
			};
		}
		burst.trail = sf::VertexArray(getPrimitiveType(burst.shape), getVertexCount(burst.shape));
		sf::Color color = getColor(burst.composition);
		for (int j = 0; j < burst.trail.getVertexCount(); j++){
			if (burst.shape == ShellData::BurstShape::Sphere){
				burst.trail[j].position = {origin.x + direction.x * distance, origin.y + direction.y * distance};
			}else{
				burst.trail[j].position = origin;
			}
			burst.trail[j].color = sf::Color(color.r, color.g, color.b, 255);
		}
		mAerialShell.burstNodes.emplace_back(burst);
		std::cout << power << '\n';
	}
	mAerialShell.burstCharge.state = ShellData::State::Detonated;
}

void Firework::updateLiftCharge(sf::Time deltaTime){
	float dT = deltaTime.asSeconds();
	std::size_t count = mAerialShell.liftCharge.trail.getVertexCount();
	sf::Vector2f position = mAerialShell.liftCharge.trail[count-1].position;
	mAerialShell.liftCharge.lifetime-=dT;
	if (mAerialShell.liftCharge.state == ShellData::State::Terminal && mAerialShell.liftCharge.lifetime <= 0.f){
		mAerialShell.liftCharge.state = ShellData::State::Completed;
		mAerialShell.burstCharge.state = ShellData::State::Detonated;
		Detonate(position);
		return;
	}
	mAerialShell.liftCharge.physics.velocity.y += _G::GRAVITY * dT;
	mAerialShell.liftCharge.physics.velocity *= mAerialShell.liftCharge.physics.drag;
	position += mAerialShell.liftCharge.physics.velocity * dT;
	position.x = randomize(position.x - 1.f, position.x + 1.f);
	float alphaLeft = mAerialShell.liftCharge.lifetime / mAerialShell.liftCharge.fuse;
	sf::Color color = getColor(mAerialShell.liftCharge.composition);
	for (int j = 0; j < count - 1; j++){
		float trailAlpha = static_cast<float>(j + 1) / static_cast<float>(count);
		float alpha = 255.f * trailAlpha * alphaLeft;
		mAerialShell.liftCharge.trail[j].position = mAerialShell.liftCharge.trail[j+1].position;
		mAerialShell.liftCharge.trail[j].color = sf::Color(color.r, color.g, color.b, static_cast<uint8_t>(alpha));
	}
	mAerialShell.liftCharge.trail[count-1].position = position;
	mAerialShell.liftCharge.trail[count-1].color = sf::Color(color.r, color.g, color.b, static_cast<uint8_t>(alphaLeft));
	if (alphaLeft <= 0.f){
		mAerialShell.liftCharge.state = ShellData::State::Completed;
	}
}

void Firework::updateBurstCharge(sf::Time deltaTime){
	float dT = deltaTime.asSeconds();
	for (int i = 0; i < mAerialShell.burstNodes.size(); i++){
		ShellData::BurstCharge& burst = mAerialShell.burstNodes[i];
		burst.lifetime -= dT;
		if (burst.lifetime <= 0.f || burst.trail[0].position.y > _G::WINDOW_HEIGHT){
			mAerialShell.burstNodes[i] = std::move(mAerialShell.burstNodes.back());
			mAerialShell.burstNodes.pop_back();
			i--;
			continue;
		}
		float alphaLeft = burst.lifetime / burst.duration;
		std::size_t count = burst.trail.getVertexCount();
		sf::Vector2f head = burst.trail[count - 1].position;
		burst.physics.velocity.y += _G::GRAVITY * dT;
		burst.physics.velocity *= burst.physics.drag;
		head += burst.physics.velocity * dT;
		sf::Color color = getColor(burst.composition);
		for (int j = 0; j < count - 1; j++){
			burst.trail[j].position = burst.trail[j + 1].position;
			float trailAlpha = static_cast<float>(j + 1) / static_cast<float>(count);
			float alpha = 255.f * trailAlpha * alphaLeft;
			burst.trail[j].color = sf::Color(color.r, color.g, color.b, static_cast<std::uint8_t>(alpha));
		}
		burst.trail[count - 1].position = head;
		if (burst.shape == ShellData::BurstShape::Sphere){
			burst.trail[count - 1].color = sf::Color(color.r, color.g, color.b, static_cast<std::uint8_t>(255.f * alphaLeft));
		}
		/* without the head *sparkles*
		burst.trail[count - 1].color = sf::Color(255, 255, 255, static_cast<std::uint8_t>(255.f * alphaLeft));
		*/
	}
	if (mAerialShell.burstNodes.empty()){
		mAerialShell.burstCharge.state = ShellData::State::Completed;
	}
}

void Firework::Update(sf::Time deltaTime){
	if (mAerialShell.liftCharge.state != ShellData::State::Completed){
		updateLiftCharge(deltaTime);
	}
	if (mAerialShell.burstCharge.state == ShellData::State::Detonated){
		updateBurstCharge(deltaTime);
	}
}

void Firework::Render(sf::RenderTarget& target){
	target.draw(mAerialShell.liftCharge.trail);
	for (auto& burst : mAerialShell.burstNodes){
		target.draw(burst.trail);
	}
}

//////////////////////////////////////////////////////////////////////////

bool Firework::IsCompleted(){
	return mAerialShell.burstCharge.state == ShellData::State::Completed
		&& mAerialShell.liftCharge.state == ShellData::State::Completed;
}

std::size_t Firework::GetMemoryBytes() const{
	std::size_t total = sizeof(Firework);

	total += mAerialShell.liftCharge.trail.getVertexCount() * sizeof(sf::Vertex);
	total += mAerialShell.burstCharge.trail.getVertexCount() * sizeof(sf::Vertex);

	total += mAerialShell.burstNodes.capacity() * sizeof(ShellData::BurstCharge);
	for (const auto& burst : mAerialShell.burstNodes){
		total += burst.trail.getVertexCount() * sizeof(sf::Vertex);
	}

	return total;
}

//////////////////////////////////////////////////////////////////////////

float Firework::getExplosivePower(const ShellData::Composition& c){
	return
		c.oxidizers.potassiumPerchlorate * 1.6f +
		c.oxidizers.potassiumNitrate    * 1.0f +
		c.oxidizers.strontiumNitrate    * 1.2f +
		c.fuels.charcoal                * 0.7f +
		c.fuels.sulfur                  * 0.9f +
		c.fuels.aluminum                * 1.0f +
		c.fuels.magnesium               * 1.2f +
		c.fuels.titanium                * 0.6f;
}

float Firework::getMass(const ShellData::Composition& c){
	return
		c.colorants.strontium +
		c.colorants.barium +
		c.colorants.copper +
		c.colorants.sodium +
		c.fuels.charcoal +
		c.fuels.sulfur +
		c.fuels.aluminum +
		c.fuels.magnesium +
		c.fuels.titanium +
		c.oxidizers.potassiumNitrate +
		c.oxidizers.potassiumPerchlorate +
		c.oxidizers.strontiumNitrate;
}