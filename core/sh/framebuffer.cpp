#include "framebuffer.h"

namespace sh
{
	FrameBuffer sh::createFramebuffer(unsigned int width, unsigned int height, int colorFormat)
	{
		//establish the buffer to be returned
		FrameBuffer buff;

		//add in given easy values
		buff.width = width;
		buff.height = height;

		//openGL creation of the fbo, colorBuffer, and depthBuffer

		//Create Framebuffer Object
		glCreateFramebuffers(1, &buff.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, buff.fbo);
		//Create 8 bit RGBA color buffer
		glGenTextures(1, &buff.colorBuffer[0]);
		glBindTexture(GL_TEXTURE_2D, buff.colorBuffer[0]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
		//Attach color buffer to framebuffer
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, buff.colorBuffer[0], 0);

		glGenTextures(1, &buff.depthBuffer);
		glBindTexture(GL_TEXTURE_2D, buff.depthBuffer);
		//Create 16 bit depth buffer - must be same width/height of color buffer
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, width, height);
		//Attach to framebuffer (assuming FBO is bound)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buff.depthBuffer, 0);

		//check for errors
		GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
			printf("Framebuffer incomplete: %d", fboStatus);
		}

		return buff;
	}

	void sh::deleteFramebuffer(FrameBuffer buff)
	{
		glDeleteFramebuffers(1, &buff.fbo);
	}
}