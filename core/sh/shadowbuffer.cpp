#include "shadowbuffer.h"

ShadowBuffer sh::createShadowBuffer()
{
	ShadowBuffer buff;

	glCreateFramebuffers(1, &buff.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, buff.fbo);

	glGenTextures(1, &buff.shadowMap);
	glBindTexture(GL_TEXTURE_2D, buff.shadowMap);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, 2048, 2048);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//Pixels outside of frustum should have max distance (white)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	return buff;
}

void sh::deleteShadowBuffer(ShadowBuffer buff)
{
	glDeleteFramebuffers(1, &buff.fbo);
}
