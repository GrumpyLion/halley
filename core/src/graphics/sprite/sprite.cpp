#include "sprite.h"
#include "sprite_sheet.h"
#include "../painter.h"
#include "../material.h"

using namespace Halley;

Sprite::Sprite()
{
	vertices[0].vertPos = Vector2f(0, 0);
	vertices[1].vertPos = Vector2f(1, 0);
	vertices[2].vertPos = Vector2f(1, 1);
	vertices[3].vertPos = Vector2f(0, 1);
	setScale(Vector2f(1, 1));
	setColour(Colour4f(1, 1, 1, 1));
}

void Sprite::draw(Painter& painter) const
{
	if (dirty) {
		update();
		dirty = false;
	}

	assert(material->getVertexStride() == sizeof(SpriteVertexAttrib));
	painter.drawQuads(material, 4, vertices.data());
}

void Sprite::update() const // Not really "const", but needs to be called from draw()
{
	// Don't copy the last Vector2f (vertPos)
	constexpr size_t size = sizeof(SpriteVertexAttrib) - sizeof(Vector2f);
	memcpy(&vertices[1], &vertices[0], size);
	memcpy(&vertices[2], &vertices[0], size);
	memcpy(&vertices[3], &vertices[0], size);
}

void Sprite::computeSize()
{
	vertices[0].size = scale * size;
	if (flip) {
		vertices[0].size.x *= -1;
	}
}

bool Sprite::isInView(Rect4f v) const
{
	// TODO
	return true;
}

void Sprite::setPos(Vector2f v)
{
	if (v != vertices[0].pos) {
		dirty = true;
		vertices[0].pos = v;
	}
}

void Sprite::setRotation(Angle1f v)
{
	if (v != vertices[0].rotation.x) {
		dirty = true;
		vertices[0].rotation.x = v.getRadians();
	}
}

void Sprite::setColour(Colour4f v)
{
	if (vertices[0].colour != v) {
		dirty = true;
		vertices[0].colour = v;
	}
}

void Sprite::setScale(Vector2f v)
{
	if (scale != v) {
		dirty = true;
		scale = v;
		computeSize();
	}
}

void Sprite::setFlip(bool v)
{
	if (flip != v) {
		dirty = true;
		flip = v;
		computeSize();
	}
}

void Sprite::setPivot(Vector2f v)
{
	if (vertices[0].pivot != v) {
		dirty = true;
		vertices[0].pivot = v;
	}
}

void Sprite::setSize(Vector2f v)
{
	if (size != v) {
		dirty = true;
		size = v;
		computeSize();
	}
}

void Sprite::setTexRect(Rect4f v)
{
	if (vertices[0].texRect != v) {
		dirty = true;
		vertices[0].texRect = v;
	}
}

void Sprite::setSprite(const SpriteSheet& sheet, String name)
{
	setSprite(sheet.getSprite(name));
}

void Sprite::setSprite(const SpriteSheetEntry& entry)
{
	size = entry.size;
	vertices[0].pivot = entry.pivot;
	vertices[0].texRect = entry.coords;
	vertices[0].rotation.y = entry.rotated ? 1.0f : 0.0f;
	dirty = true;
	computeSize();
}
