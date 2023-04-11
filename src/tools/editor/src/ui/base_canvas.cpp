#include "base_canvas.h"

#include <utility>
using namespace Halley;

BaseCanvas::BaseCanvas(String id, UIStyle style, UISizer sizer, std::shared_ptr<InputKeyboard> keyboard)
	: UIClickable(std::move(id), {}, std::move(sizer))
	, keyboard(std::move(keyboard))
	, draggingButton({false, false})
{
	bg = style.getSprite("background");
	bgSize = bg.getSize();
	border = style.getSprite("border");
	
	setHandle(UIEventType::MouseWheel, [this] (const UIEvent& event)
	{
		onMouseWheel(event);
	});
}

float BaseCanvas::getZoomLevel() const
{
	return std::pow(2.0f, float(zoomExp));
}

void BaseCanvas::setZoomListener(ZoomListener listener)
{
	zoomListener = listener;
}

void BaseCanvas::setMousePosListener(MousePosListener listener)
{
	mousePosListener = listener;
}

void BaseCanvas::setZoomEnabled(bool enabled)
{
	zoomEnabled = enabled;
}

void BaseCanvas::setScrollEnabled(bool enabled)
{
	scrollEnabled = enabled;
}

void BaseCanvas::setLeftClickScrollEnabled(bool enabled)
{
	leftClickScrollEnabled = enabled;
}

void BaseCanvas::setLeftClickScrollKey(std::optional<KeyCode> key)
{
	leftClickScrollKey = key;
}

void BaseCanvas::setMouseMirror(std::shared_ptr<UIWidget> widget)
{
	mouseMirror = widget;
}

void BaseCanvas::doSetState(State state)
{
}

void BaseCanvas::update(Time t, bool moved)
{
	const auto scale = Vector2f(1, 1) / (bgSize * getZoomLevel());

	const auto centrePos = getScrollPosition() * scale;
	const auto size = getSize() * scale;

	bg
		.setPos(getPosition())
		.setSize(getSize())
		.setTexRect(Rect4f(centrePos - 0.5f * size, centrePos + 0.5f * size));

	border
		.setPos(getPosition())
		.scaleTo(getSize());

	UIClickable::update(t, moved);
}

void BaseCanvas::draw(UIPainter& painter) const
{
	painter.draw(bg);
}

void BaseCanvas::drawAfterChildren(UIPainter& painter) const
{
	painter.draw(border);
}

void BaseCanvas::pressMouse(Vector2f mousePos, int button, KeyMods keyMods)
{
	const bool canLeftClickScroll = leftClickScrollEnabled && (!leftClickScrollKey || keyboard && keyboard->isButtonDown(*leftClickScrollKey));
	if (button == 0 && canLeftClickScroll) {
		draggingButton[0] = true;
	}
	if (button == 1) {
		draggingButton[1] = true;
	}
	const bool shouldDrag = scrollEnabled && (draggingButton[0] || draggingButton[1]);

	if (shouldDrag && !dragging) {
		dragging = true;
		mouseStartPos = mousePos;
		startScrollPos = getScrollPosition();
	}

	UIClickable::pressMouse(mousePos, button, keyMods);

	if (mouseMirror && !dragging) {
		mouseMirror->pressMouse(mousePos, button, keyMods);
	}
}

void BaseCanvas::releaseMouse(Vector2f mousePos, int button)
{
	if (button == 0 || button == 1) {
		draggingButton[button] = false;
	}
	const bool shouldDrag = scrollEnabled && (draggingButton[0] || draggingButton[1]);

	UIClickable::releaseMouse(mousePos, button);

	if (mouseMirror && !dragging) {
		mouseMirror->releaseMouse(mousePos, button);
	}
	
	if (dragging && !shouldDrag) {
		onMouseOver(mousePos);
		dragging = false;
	}
}

void BaseCanvas::onMouseOver(Vector2f mousePos)
{
	lastMousePos = mousePos;
	if (dragging) {
		setScrollPosition(mouseStartPos - mousePos + startScrollPos);
	}

	if (mousePosListener) {
		mousePosListener(mousePos);
	}

	UIClickable::onMouseOver(mousePos);

	if (mouseMirror) {
		if (dragging) {
			mouseMirror->onMouseLeft(mousePos);
		} else {
			mouseMirror->onMouseOver(mousePos);
		}
	}
}

void BaseCanvas::onDoubleClicked(Vector2f mousePos, KeyMods keyMods)
{
	if (mouseMirror) {
		mouseMirror->sendEvent(UIEvent(UIEventType::CanvasDoubleClicked, getId()));
	} else {
		sendEvent(UIEvent(UIEventType::CanvasDoubleClicked, getId()));
	}
}

void BaseCanvas::refresh()
{
}

void BaseCanvas::onMouseWheel(const UIEvent& event)
{
	if (mouseMirror) {
		mouseMirror->sendEventDown(event);
	}

	if (!zoomEnabled) {
		return;
	}
	
	const float oldZoom = getZoomLevel();
	zoomExp = clamp(zoomExp + signOf(event.getIntData()), -5, 5);
	const float zoom = getZoomLevel();

	if (zoom != oldZoom) {
		const Vector2f childPos = getChildren().at(0)->getPosition() - getPosition();

		const Vector2f panelScrollPos = getScrollPosition();

		if (zoomListener) {
			zoomListener(zoom);
		}

		refresh();

		const Vector2f relMousePos = lastMousePos - getBasePosition();
		const Vector2f oldMousePos = (relMousePos - childPos + panelScrollPos) / oldZoom;
		const Vector2f newScrollPos = oldMousePos * zoom - relMousePos;

		setScrollPosition(newScrollPos);
	}
}

