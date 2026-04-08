#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Music.hpp>

#include <map>
#include <memory>

namespace Core::ResourceLoader{

	inline bool LoadFromPath(sf::Font& resource, const std::string& path){
		return resource.openFromFile(path);
	}

	inline bool LoadFromPath(sf::Texture& resource, const std::string& path){
		return resource.loadFromFile(path);
	}

	inline bool LoadFromPath(sf::SoundBuffer& resource, const std::string& path){
		return resource.loadFromFile(path);
	}

	inline bool LoadFromPath(sf::Music& resource, const std::string& path){
		return resource.openFromFile(path);
	}
}

namespace Core{
	template <typename Resource, typename Identifier>
	class ResourceHolder{
		public:
			void Load(const Identifier& id, const std::string& path){
				auto resource = std::make_unique<Resource>();
				if (!ResourceLoader::LoadFromPath(*resource, path)){
					throw std::runtime_error("ResourceHolder::Load failed on: " + path);
				}
				mResourceMap.emplace(id, std::move(resource));
			}

			bool TryLoad(const Identifier& id, const std::string& path){
				auto resource = std::make_unique<Resource>();
				if (!ResourceLoader::LoadFromPath(*resource, path)){
					return false;
				}
				mResourceMap.emplace(id, std::move(resource));
				return true;
			}

			Resource& Get(const Identifier& id){
				const auto iterator = mResourceMap.find(id);
				if (iterator == mResourceMap.end()){
					throw std::runtime_error("ResourceHolder::GetResource id not loaded: " + std::to_string(static_cast<int>(id)));
				}
				return *iterator->second;
			}

			bool Contains(const Identifier& id) const{
				return mResourceMap.find(id) != mResourceMap.end();
			}

			void Unload(const Identifier& id){
				mResourceMap.erase(id);
			}

			void Clear(){
				mResourceMap.clear();
			}

		private:
			std::map<Identifier, std::unique_ptr<Resource>> mResourceMap;
	};
}