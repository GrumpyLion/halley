#pragma once
#include "../ui_widget.h"
#include "halley/graphics/text/text_renderer.h"
#include <climits>
#include "halley/text/i18n.h"
#include "halley/concurrency/future.h"

namespace Halley {
	class UILabel : public UIWidget {
	public:
		explicit UILabel(String id, UIStyle style, LocalisedString text);
		explicit UILabel(String id, UIStyle style, TextRenderer renderer, LocalisedString text);
		~UILabel();

		void setText(const LocalisedString& text);
		void setText(LocalisedString&& text);
		void setTextAndColours(LocalisedString text, Vector<ColourOverride> overrides);
		void setTextAndColours(std::pair<LocalisedString, Vector<ColourOverride>> textAndColours);
		void setFutureText(Future<String> text);
		const LocalisedString& getText() const;
		void setColourOverride(Vector<ColourOverride> overrides);

		void setMaxWidth(float maxWidth);
		void setMaxHeight(float maxHeight);
		float getMaxWidth() const;
		float getMaxHeight() const;
		void setWordWrapped(bool wrapped);
		bool isWordWrapped() const;
		bool isClipped() const;
		void setMarquee(std::optional<float> marqueeSpeed);
		void setFlowLayout(bool flow);

		void setAlignment(float alignment);

		void setTextRenderer(TextRenderer renderer);
		TextRenderer& getTextRenderer();
		const TextRenderer& getTextRenderer() const;

		void setColour(Colour4f colour);		
		Colour4f getColour() const;
		
		void setSelectable(TextRenderer normalRenderer, TextRenderer selectedRenderer, bool preserveAlpha = false);
		void setDisablable(TextRenderer normalRenderer, TextRenderer disabledRenderer);
		void setHoverable(TextRenderer normalRenderer, TextRenderer hoveredRenderer);
		
		void draw(UIPainter& painter) const override;
		void update(Time t, bool moved) override;

		void setFontSize(float size);

	protected:
		void onParentChanged() override;

	private:
		TextRenderer renderer;
		LocalisedString text;
		const std::shared_ptr<bool> aliveFlag;

		Vector2f textExtents;
		float maxWidth = std::numeric_limits<float>::infinity();
		float maxHeight = std::numeric_limits<float>::infinity();
		bool wordWrapped = true;
		bool needsClip = false;
		bool flowLayout = false;

		Time marqueeIdle;
		std::optional<float> marqueeSpeed;
		int marqueeDirection = -1;
		float marqueePos = 0;
		float unclippedWidth;
		float lastCellWidth;

		void updateMinSize();
		void updateText();
		void updateMarquee(Time t);
		float getCellWidth();
	};
}
