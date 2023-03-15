#pragma once
#include "halley/graphics/sprite/animation.h"
#include "halley/editor_extensions/scene_editor_gizmo.h"

namespace Halley {
	class Material;
	
	class SelectionBoxGizmo final : public SceneEditorGizmo {
	public:
		SelectionBoxGizmo(SnapRules snapRules, Resources& resources);

		void update(Time time, const ISceneEditor& sceneEditor, const SceneEditorInputState& inputState) override;
		void draw(Painter& painter, const ISceneEditor& sceneEditor) const override;

	private:
		std::optional<Rect4f> bounds;
	};
}
