//sh/framebuffer.h
#pragma once

#include <stdio.h>
#include "../ew/external/glad.h"
namespace sh
{
	struct FrameBuffer
	{
		unsigned int fbo;
		unsigned int colorBuffer[8];
		unsigned int depthBuffer;
		unsigned int width;
		unsigned int height;
	};
	FrameBuffer createFramebuffer(unsigned int width, unsigned int height, int colorFormat);
	void deleteFramebuffer(FrameBuffer buff);
}