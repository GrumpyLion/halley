#pragma once
#include "halley/data_structures/config_node.h"
#include "halley/file_formats/config_file.h"

namespace Halley {
    class UIDefinition : public Resource {
    public:
        struct FindResult {
	        ConfigNode* result = nullptr;
            ConfigNode* parent = nullptr;
            int childIdx = -1;
        };

        UIDefinition();
        UIDefinition(UIDefinition&& other) = default;
        UIDefinition(const UIDefinition& other) = default;
        UIDefinition(ConfigNode node);
        UIDefinition(ConfigFile file);

    	static std::unique_ptr<UIDefinition> loadResource(ResourceLoader& loader);
		constexpr static AssetType getAssetType() { return AssetType::UIDefinition; }

		void reload(Resource&& resource) override;

		void serialize(Serializer& s) const;
		void deserialize(Deserializer& s);

    	const ConfigNode& getRoot() const;
        ConfigNode& getRoot();

        FindResult findUUID(const String& id);
        
        void parseYAML(gsl::span<const gsl::byte> yaml);
		String toYAML() const;
        void makeDefault();

    private:
        ConfigFile data;

        void assignIds(ConfigNode& node);
        FindResult findUUID(ConfigNode* parent, int childIdx, ConfigNode& node, const String& id);
    };
}
