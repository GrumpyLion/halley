#pragma once
#include "entity_factory.h"
#include "halley/game/scene_editor_interface.h"

namespace Halley {
    class PrefabSceneData final : public ISceneData {
    public:
    	PrefabSceneData(Prefab& prefab, std::shared_ptr<EntityFactory> factory, World& world, Resources& gameResources);

        std::optional<EntityNodeData> tryGetWriteableEntityNodeData(const String& id) override;
        std::optional<ConstEntityNodeData> tryGetEntityNodeData(const String& id) override;

    	std::pair<Vector<UUID>, Vector<EntityData*>> getWriteableEntityDatas(gsl::span<const UUID> ids) override;

    	void reloadEntities(gsl::span<const String> ids, gsl::span<const EntityData*> datas) override;
        EntityTree getEntityTree() const override;
        std::pair<String, size_t> reparentEntity(const String& entityId, const String& newParentId, size_t childIndex) override;
        std::pair<String, size_t> getEntityParenting(const String& entityId) override;
        bool isSingleRoot() override;
    	Vector<const EntityData*> getEntityDataStack(const String& entityId) override;
    	
    private:
    	Prefab& prefab;
        std::shared_ptr<EntityFactory> factory;
        World& world;
    	Resources& gameResources;

    	struct EntityAndParent {
    		EntityData* entity = nullptr;
    		EntityData* parent = nullptr;
    		size_t childIdx = 0;
    	};

        void fillEntityTree(const EntityData& node, EntityTree& tree) const;

        EntityData& findEntity(const String& id);

    	static EntityData* findEntity(gsl::span<EntityData> node, const String& id);
    	static EntityAndParent findEntityAndParent(gsl::span<EntityData> node, EntityData* previous, size_t idx, const String& id);
        static EntityAndParent findEntityAndParent(EntityData& node, EntityData* previous, size_t idx, const String& id);

        void getWriteableEntityDatas(Vector<EntityData*>& result, gsl::span<EntityData> datas, gsl::span<const UUID> sortedIds);

        static void addChild(EntityData& parent, int index, EntityData child);
        static EntityData removeChild(EntityData& parent, const String& childId);
        static void moveChild(EntityData& parent, const String& childId, int targetIndex);

    	static void makeTransformRelative(EntityRef entity, std::optional<EntityRef> newParent, EntityData& entityData);
    	static void makeTransformRelative2D(EntityRef entity, std::optional<EntityRef> newParent, EntityData& entityData);
    };
}
