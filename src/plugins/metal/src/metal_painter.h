#pragma once
#include <halley/graphics/painter.h>
#include <Metal/Metal.h>

namespace Halley {
	class MetalVideo;

	class MetalPainter : public Painter
	{
	public:
		explicit MetalPainter(MetalVideo& video, Resources& resources);
		void doClear(std::optional<Colour> colour, std::optional<float> depth, std::optional<uint8_t> stencil) override;
		void setMaterialPass(const Material& material, int pass) override;
		void doStartRender() override;
		void doEndRender() override;
		void setVertices(const MaterialDefinition& material, size_t numVertices, const void* vertexData, size_t numIndices, const IndexType* indices, bool standardQuadsOnly) override;
		void drawTriangles(size_t numIndices) override;
		void setViewPort(Rect4i rect) override;
		void setClip(Rect4i clip, bool enable) override;
		void setMaterialData(const Material& material) override;
		void onUpdateProjection(Material& material, bool hashChanged) override;
		void startEncoding(id<MTLTexture> texture);
		void endEncoding();

	private:
		void setBlending(BlendType blendType, MTLRenderPipelineColorAttachmentDescriptor* colorAttachment);
		void setBlendFactor(MTLRenderPipelineColorAttachmentDescriptor* colorAttachment, MTLBlendFactor src, MTLBlendFactor dst);
		MTLRenderPassDescriptor* renderPassDescriptorForTextureAndColour(id<MTLTexture> texture, Colour& colour);

		MetalVideo& video;
		id<MTLRenderCommandEncoder> encoder;
		id<MTLBuffer> indexBuffer;
	};
}
