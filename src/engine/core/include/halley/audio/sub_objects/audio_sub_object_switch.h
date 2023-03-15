#pragma once
#include "../audio_expression.h"
#include "../audio_sub_object.h"

namespace Halley {
    class AudioSubObjectSwitch final : public IAudioSubObject {
    public:
        AudioSubObjectSwitch() = default;
        AudioSubObjectSwitch(const ConfigNode& node);
    	ConfigNode toConfigNode() const override;
    	
	    void load(const ConfigNode& node) override;

        AudioSubObjectType getType() const override { return AudioSubObjectType::Switch; }
	    std::unique_ptr<AudioSource> makeSource(AudioEngine& engine, AudioEmitter& emitter) const override;

        String getName() const override;
        size_t getNumSubObjects() const override;
        AudioSubObjectHandle& getSubObject(size_t n) override;
        Vector<String> getSubCategories(const AudioProperties& audioProperties) const override;
        String getSubObjectCategory(size_t n) const override;
        bool canAddObject(AudioSubObjectType type, const std::optional<String>& caseName) const override;
        void addObject(AudioSubObjectHandle handle, const std::optional<String>& caseName, size_t idx) override;
        AudioSubObjectHandle removeObject(const IAudioObject* object) override;

        void loadDependencies(Resources& resources) override;
	    void serialize(Serializer& s) const override;
	    void deserialize(Deserializer& s) override;

    	const String& getSwitchId() const;
        void setSwitchId(String value);

    private:
		String switchId;
		HashMap<String, AudioSubObjectHandle> cases;
    };
}
