#include "halley/ui/widgets/ui_label.h"
#include "halley/text/i18n.h"

using namespace Halley;

UILabel::UILabel(String id, UIStyle style, LocalisedString text)
	: UIWidget(std::move(id), {})
	, renderer(style.getTextRenderer("label"))
	, text(std::move(text))
	, aliveFlag(std::make_shared<bool>(true))
{
	styles.emplace_back(std::move(style));
	updateText();
}

UILabel::UILabel(String id, UIStyle style, TextRenderer renderer, LocalisedString text)
	: UIWidget(std::move(id), {})
	, renderer(std::move(renderer))
	, text(std::move(text))
	, aliveFlag(std::make_shared<bool>(true))
{
	styles.emplace_back(std::move(style));
	updateText();
}

UILabel::~UILabel()
{
	*aliveFlag = false;
}

void UILabel::draw(UIPainter& painter) const
{
	if (needsClip) {
		painter.withClip(Rect4f(getPosition(), getPosition() + getMinimumSize())).draw(renderer);
	} else {
		painter.draw(renderer);
	}
}

void UILabel::update(Time t, bool moved)
{
	if (flowLayout && lastCellWidth != getCellWidth()) {
		updateText();
	}
	if (marqueeSpeed) {
		updateMarquee(t);
	}
	if (text.checkForUpdates()) {
		updateText();
	}
	if (moved || marqueeSpeed) {
		renderer.setPosition(getPosition() + Vector2f(renderer.getAlignment() * textExtents.x - marqueePos, 0.0f));
	}
}

void UILabel::setFontSize(float size)
{
	renderer.setSize(size);
	updateMinSize();
}

void UILabel::setMarquee(std::optional<float> speed)
{
	marqueeSpeed = speed;
	if (marqueeSpeed) {
		wordWrapped = false;
	} else {
		marqueePos = 0;
		marqueeIdle = 0;
		marqueeDirection = -1;
	}
}

void UILabel::updateMinSize()
{
	lastCellWidth = getCellWidth();
	const float effectiveMaxWidth = std::min(lastCellWidth, maxWidth);

	needsClip = false;
	textExtents = renderer.getExtents();
	unclippedWidth = textExtents.x;
	if (textExtents.x > effectiveMaxWidth) {
		if (wordWrapped || flowLayout) {
			renderer.setText(renderer.split(effectiveMaxWidth));
			textExtents = renderer.getExtents();
			unclippedWidth = textExtents.x;
		} else {
			unclippedWidth = textExtents.x;
			textExtents.x = effectiveMaxWidth;
			needsClip = true;
		}
	}
	if (textExtents.y > maxHeight) {
		float maxLines = std::floor(maxHeight / renderer.getLineHeight());
		textExtents.y = maxLines * renderer.getLineHeight();
		needsClip = true;
	}

	if (flowLayout) {
		setMinSize(Vector2f(0.0f, textExtents.y));
	} else {
		setMinSize(textExtents);
	}
}

void UILabel::updateText() {
	renderer.setText(text);
	updateMinSize();
}

void UILabel::updateMarquee(Time t)
{
	if (needsClip) {
		if (marqueeIdle > 0) {
			marqueeIdle -= t;
			return;
		}
		const float speed = *marqueeSpeed;
		const float maxMarquee = unclippedWidth - maxWidth;
		marqueePos += float(marqueeDirection) * float(t) * speed;
		if (marqueePos < 0 || marqueePos > maxMarquee) {
			marqueePos = clamp(marqueePos, 0.0f, maxMarquee);
			marqueeDirection = -marqueeDirection;
			marqueeIdle = 0.5;
		}
	} else {
		marqueePos = 0;
		marqueeIdle = 0;
		marqueeDirection = -1;
	}
}

float UILabel::getCellWidth()
{
	if (flowLayout) {
		auto parent = getParent();
		if (parent) {
			auto max = parent->getMaxChildWidth();
			if (max) {
				return max.value();
			}
		}
	}
	
	return std::numeric_limits<float>::infinity();
}

void UILabel::setText(const LocalisedString& t)
{
	if (text != t) {
		text = t;
		updateText();
	}
}

void UILabel::setText(LocalisedString&& t)
{
	if (text != t) {
		text = std::move(t);
		updateText();
	}
}

void UILabel::setTextAndColours(LocalisedString text, Vector<ColourOverride> overrides)
{
	setText(std::move(text));
	setColourOverride(std::move(overrides));
}

void UILabel::setTextAndColours(std::pair<LocalisedString, Vector<ColourOverride>> textAndColours)
{
	setText(std::move(textAndColours.first));
	setColourOverride(std::move(textAndColours.second));
}

void UILabel::setFutureText(Future<String> futureText)
{
	const auto flag = aliveFlag;
	futureText.then(Executors::getMainUpdateThread(), [=] (const String& filtered)
	{
		if (*flag) {
			setText(LocalisedString::fromUserString(filtered));
		}
	});
}

const LocalisedString& UILabel::getText() const
{
	return text;
}

void UILabel::setColourOverride(Vector<ColourOverride> overrides)
{
	renderer.setColourOverride(std::move(overrides));
}

void UILabel::setMaxWidth(float m)
{
	if (std::abs(maxWidth - m) > 0.001f) {
		maxWidth = m;
		updateMinSize();
		updateText();
	}
}

void UILabel::setMaxHeight(float m)
{
	if (std::abs(maxHeight - m) > 0.001f) {
		maxHeight = m;
		updateMinSize();
		updateText();
	}
}

float UILabel::getMaxWidth() const
{
	return maxWidth;
}

float UILabel::getMaxHeight() const
{
	return maxHeight;
}

void UILabel::setWordWrapped(bool wrapped)
{
	wordWrapped = wrapped;
}

bool UILabel::isWordWrapped() const
{
	return wordWrapped;
}

bool UILabel::isClipped() const
{
	return needsClip;
}

void UILabel::setFlowLayout(bool flow)
{
	flowLayout = flow;
	updateText();
}

void UILabel::setAlignment(float alignment)
{
	renderer.setAlignment(alignment);
}

TextRenderer& UILabel::getTextRenderer()
{
	return renderer;
}

const TextRenderer& UILabel::getTextRenderer() const
{
	return renderer;
}

Colour4f UILabel::getColour() const
{
	return renderer.getColour();
}

void UILabel::setTextRenderer(TextRenderer r)
{
	r.setText(renderer.getText()).setPosition(renderer.getPosition());
	renderer = r;
	updateMinSize();
}

void UILabel::setColour(Colour4f colour)
{
	renderer.setColour(colour);
}

void UILabel::setSelectable(TextRenderer normalRenderer, TextRenderer selectedRenderer, bool preserveAlpha)
{
	setHandle(UIEventType::SetSelected, [=] (const UIEvent& event)
	{
		if (event.getBoolData()) {
			auto col = selectedRenderer.getColour();
			if (preserveAlpha) {
				col.a = getColour().a;
			}

			setColour(col);
			renderer.setOutline(selectedRenderer.getOutline());
			renderer.setOutlineColour(selectedRenderer.getOutlineColour());
		} else {
			auto col = normalRenderer.getColour();
			if (preserveAlpha) {
				col.a = getColour().a;
			}

			setColour(col);
			renderer.setOutline(normalRenderer.getOutline());
			renderer.setOutlineColour(normalRenderer.getOutlineColour());
		}
	});
}

/*
void UILabel::setSelectable(TextRenderer selectedRenderer)
{
	Colour4f origCol = getColour();
	Colour4f origOutlineCol = renderer.getOutlineColour();
	float origOutline = renderer.getOutline();

	setHandle(UIEventType::SetSelected, [=] (const UIEvent& event) mutable
	{
		if (event.getBoolData()) {
			origCol = getColour();
			origOutlineCol = renderer.getOutlineColour();
			origOutline = renderer.getOutline();

			setColour(selectedRenderer.getColour());
			renderer.setOutline(selectedRenderer.getOutline());
			renderer.setOutlineColour(selectedRenderer.getOutlineColour());
		} else {
			setColour(origCol);
			renderer.setOutline(origOutline);
			renderer.setOutlineColour(origOutlineCol);
		}
	});
}
*/

void UILabel::setDisablable(TextRenderer normalRenderer, TextRenderer disabledRenderer)
{
	setHandle(UIEventType::SetEnabled, [=] (const UIEvent& event)
	{
		if (event.getBoolData()) {
			setColour(normalRenderer.getColour());
			renderer.setOutline(normalRenderer.getOutline());
			renderer.setOutlineColour(normalRenderer.getOutlineColour());
		}
		else {
			setColour(disabledRenderer.getColour());
			renderer.setOutline(disabledRenderer.getOutline());
			renderer.setOutlineColour(disabledRenderer.getOutlineColour());
		}
	});
}

void UILabel::setHoverable(TextRenderer normalRenderer, TextRenderer hoveredRenderer)
{
	setHandle(UIEventType::SetHovered, [=](const UIEvent& event)
	{
		if (event.getBoolData()) {
			setColour(hoveredRenderer.getColour());
			renderer.setOutline(hoveredRenderer.getOutline());
			renderer.setOutlineColour(hoveredRenderer.getOutlineColour());
		}
		else {
			setColour(normalRenderer.getColour());
			renderer.setOutline(normalRenderer.getOutline());
			renderer.setOutlineColour(normalRenderer.getOutlineColour());
		}
	});
}

void UILabel::onParentChanged()
{
	if (flowLayout) {
		updateText();
	}
}
