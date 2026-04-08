#pragma once

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <filesystem>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace ResourceHolderDetail {
// SFML 3.0.x: Font uses openFromFile; Texture / SoundBuffer use loadFromFile.
inline bool loadFromPath(sf::Font& resource, const std::filesystem::path& filepath) {
	return resource.openFromFile(filepath);
}

inline bool loadFromPath(sf::Texture& resource, const std::filesystem::path& filepath) {
	return resource.loadFromFile(filepath);
}

inline bool loadFromPath(sf::SoundBuffer& resource, const std::filesystem::path& filepath) {
	return resource.loadFromFile(filepath);
}
} // namespace ResourceHolderDetail

template <typename Resource, typename Identifier>
class ResourceHolder {
	public:
		void load(const Identifier& id, const std::filesystem::path& filepath) {
			auto resource = std::make_unique<Resource>();
			if (!ResourceHolderDetail::loadFromPath(*resource, filepath)) {
				throw std::runtime_error(
					"ResourceHolder::load failed for: " + filepath.string());
			}
			mMap.insert_or_assign(id, std::move(resource));
		}

		bool tryLoad(const Identifier& id, const std::filesystem::path& filepath) {
			auto resource = std::make_unique<Resource>();
			if (!ResourceHolderDetail::loadFromPath(*resource, filepath)) {
				return false;
			}
			mMap.insert_or_assign(id, std::move(resource));
			return true;
		}

		Resource& get(const Identifier& id) {
			const auto it = mMap.find(id);
			if (it == mMap.end()) {
				throw std::runtime_error("ResourceHolder::get: id not loaded");
			}
			return *it->second;
		}

		const Resource& get(const Identifier& id) const {
			const auto it = mMap.find(id);
			if (it == mMap.end()) {
				throw std::runtime_error("ResourceHolder::get: id not loaded");
			}
			return *it->second;
		}

		bool contains(const Identifier& id) const {
			return mMap.find(id) != mMap.end();
		}

		void unload(const Identifier& id) { mMap.erase(id); }

		void clear() { mMap.clear(); }

	private:
		std::map<Identifier, std::unique_ptr<Resource>> mMap;
};
