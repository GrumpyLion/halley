#pragma once

#include "stats_view.h"
#include "halley/graphics/render_snapshot.h"

namespace Halley
{
	class System;

	class FrameDebugger : public StatsView
	{
	public:
		FrameDebugger(Resources& resources, const HalleyAPI& api);
		~FrameDebugger() override;

		void update(Time t) override;
		void draw(RenderContext& context) override;
		void paint(Painter& painter) override;

		bool isRendering() const;

	private:
		TextRenderer headerText;
		Sprite whitebox;

		std::unique_ptr<RenderSnapshot> renderSnapshot;
		RenderSnapshot::PlaybackResult lastPlaybackResult;
		bool waiting = false;
		
		int framesToDraw = 0;
		TargetBufferType blitType = TargetBufferType::Colour;
		
		std::shared_ptr<const MaterialDefinition> debugMaterial;
	};
}
