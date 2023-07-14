#include "halley/tools/distance_field/distance_field_generator.h"
#include <cassert>
#include <halley/file_formats/image.h>
#include <gsl/gsl_assert>

using namespace Halley;

namespace {
	float getDistanceAt(const int* src, int srcW, int srcH, int xCentre, int yCentre, float radius)
	{
		auto getAlpha = [&](int x, int y) { return (src[x + y * srcW] & 0xFF000000) >> 24; };
		bool isInside = getAlpha(xCentre, yCentre) > 127;
		if (radius < 0.001f) {
			return isInside ? 1.0f : 0.0f;
		}

		int iRadius = int(ceil(radius));
		int x0 = std::max(0, xCentre - iRadius);
		int x1 = std::min(xCentre + iRadius, srcW - 1);
		int y0 = std::max(0, yCentre - iRadius);
		int y1 = std::min(yCentre + iRadius, srcH - 1);

		int bestDistSqr = 2147483647;
		for (int y = y0; y <= y1; y++) {
			for (int x = x0; x <= x1; x++) {
				bool thisInside = getAlpha(x, y) > 127;
				if (isInside != thisInside) {
					// Candidate for best neighbour
					int distSqr = (x - xCentre) * (x - xCentre) + (y - yCentre) * (y - yCentre);
					bestDistSqr = std::min(distSqr, bestDistSqr);
				}
			}
		}

		const float dist = float(sqrt(bestDistSqr));
		const float normalDistance = (2 * dist - 1) / (2 * radius);
		const float finalValue = 0.5f * (isInside ? 1.0f + normalDistance : 1.0f - normalDistance);

		return finalValue;
	}

	std::unique_ptr<Image> DistanceFieldGenerator::generateSDFInternal(Image& srcImg, Vector2i size, float radius)
	{
		Expects(!srcImg.getPixelBytes().empty());
		Expects(srcImg.getFormat() == Image::Format::RGBA);
		const int srcW = srcImg.getWidth();
		const int srcH = srcImg.getHeight();
		const auto src = srcImg.getPixels4BPP();

		auto dstImg = std::make_unique<Image>(Image::Format::SingleChannel, size);

		const int w = size.x;
		const int h = size.y;
		const auto dstStart = dstImg->getPixelBytes();

		int texelW = srcW / w;
		int texelH = srcH / h;

		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				unsigned char* dst = &dstStart[x + y * w];
				float distAcc = 0;
				// For each sub-pixel, compute the distance to closest pixel of the opposite value
				// Then average it all
				for (int j = 0; j < texelH; j++) {
					for (int i = 0; i < texelW; i++) {
						distAcc += getDistanceAt(src.data(), srcW, srcH, x * srcW / w + i, y * srcH / h + j, radius * srcW / w);
					}
				}
				int distance = clamp(int(distAcc * 255 / (texelW * texelH)), 0, 255);
				*dst = static_cast<unsigned char>(distance);
			}
		}

		return dstImg;
	}
}

std::unique_ptr<Image> DistanceFieldGenerator::generateSDF(Image& srcImg, Vector2i size, float radius)
{
	return generateSDFInternal(srcImg, size, radius);
}

std::unique_ptr<Image> DistanceFieldGenerator::generateMSDF(Image& src, Vector2i size, float radius)
{
	// TODO
	return {};
}
