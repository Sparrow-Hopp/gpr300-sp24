//sh/framebuffer.h
#pragma once

#include <stdio.h>
#include "../ew/external/glad.h"
namespace sh
{
	struct ShadowBuffer
	{
		unsigned int fbo;
		unsigned int shadowMap;
		unsigned int width;
		unsigned int height;
	};
	ShadowBuffer createShadowBuffer(unsigned int width, unsigned int height);
	void deleteShadowBuffer(ShadowBuffer buff);
}