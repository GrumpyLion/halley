#pragma once
#include "halley/graphics/sprite/animation.h"
#include "halley/editor_extensions/scene_editor_gizmo.h"

namespace Halley {
	class Material;
	
	class SelectedBoundsGizmo final : public SceneEditorGizmo {
	public:
		SelectedBoundsGizmo(SnapRules snapRules, Resources& resources);

		void update(Time time, const ISceneEditor& sceneEditor, const SceneEditorInputState& inputState) override;
		void draw(Painter& painter, const ISceneEditor& sceneEditor) const override;

	private:
		std::shared_ptr<Material> stencilMaterial;
		std::shared_ptr<Material> outlineMaterial;

		bool shouldInclude(const Sprite& sprite, const ISceneEditor& sceneEditor) const;

		void drawSplit(Painter& painter, const ISceneEditor& sceneEditor, float width, Colour4f colour) const;
		void drawMerged(Painter& painter, const ISceneEditor& sceneEditor, float width, Colour4f colour) const;
		void drawStencilTree(Painter& painter, EntityRef entity, const ISceneEditor& sceneEditor) const;
		void drawOutlineTree(Painter& painter, EntityRef entity, const ISceneEditor& sceneEditor, float width, Colour4f colour) const;
		void drawStencilSprite(Painter& painter, const Sprite& sprite) const;
		void drawOutlineSprite(Painter& painter, const Sprite& sprite, float width, Colour4f colour) const;
	};
}
