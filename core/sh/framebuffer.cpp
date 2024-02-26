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

	FrameBuffer createGBuffer(unsigned int width, unsigned int height)
	{
		FrameBuffer framebuffer;
		framebuffer.width = width;
		framebuffer.height = height;

		glCreateFramebuffers(1, &framebuffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

		int formats[3] = {
			GL_RGB32F, //0 = World Position 
			GL_RGB16F, //1 = World Normal
			GL_RGB16F  //2 = Albedo
		};
		//Create 3 color textures
		for (size_t i = 0; i < 3; i++)
		{
			glGenTextures(1, &framebuffer.colorBuffer[i]);
			glBindTexture(GL_TEXTURE_2D, framebuffer.colorBuffer[i]);
			glTexStorage2D(GL_TEXTURE_2D, 1, formats[i], width, height);
			//Clamp to border so we don't wrap when sampling for post processing
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//Attach each texture to a different slot.
		//GL_COLOR_ATTACHMENT0 + 1 = GL_COLOR_ATTACHMENT1, etc
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, framebuffer.colorBuffer[i], 0);
		}
		//Explicitly tell OpenGL which color attachments we will draw to
		const GLenum drawBuffers[3] = {
				GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
		};
		glDrawBuffers(3, drawBuffers);

		//Add texture2D depth buffer
		glGenTextures(1, &framebuffer.depthBuffer);
		glBindTexture(GL_TEXTURE_2D, framebuffer.depthBuffer);
		//Create 16 bit depth buffer - must be same width/height of color buffer
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, width, height);
		//Attach to framebuffer (assuming FBO is bound)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffer.depthBuffer, 0);

		//TODO: Check for completeness
		GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
			printf("Framebuffer incomplete: %d", fboStatus);
		}

		//Clean up global state
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return framebuffer;
	}

	void sh::deleteFramebuffer(FrameBuffer buff)
	{
		glDeleteFramebuffers(1, &buff.fbo);
	}
}