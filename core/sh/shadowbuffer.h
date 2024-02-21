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
	};
	ShadowBuffer createShadowBuffer();
	void deleteShadowBuffer(ShadowBuffer buff);
}