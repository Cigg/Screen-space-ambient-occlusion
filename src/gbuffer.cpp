#include <stdio.h>
#include <cstring>

#include "gbuffer.h"

GBuffer::GBuffer()
{
    frameBuffer = 0;
    depthTexture = 0;
    std::memset(textures, 0, sizeof(textures));
}

GBuffer::~GBuffer()
{
    if (frameBuffer != 0) {
        glDeleteFramebuffers(1, &frameBuffer);
    }

    if (textures[0] != 0) {
        glDeleteTextures(ARRAY_SIZE_IN_ELEMENTS(textures), textures);
    }

	if (depthTexture != 0) {
		glDeleteTextures(1, &depthTexture);
	}
}


bool GBuffer::Init(unsigned int screenWidth, unsigned int screenHeight)
{
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    // The textures we're going to render to
    glGenTextures(GBUFFER_NUM_TEXTURES, textures);
    for(unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; i++) {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, textures[i], 0);
    }

    // Depth texture
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void GBuffer::BindForWriting()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0,
                             GL_COLOR_ATTACHMENT1,
                             GL_COLOR_ATTACHMENT2
                             //GL_COLOR_ATTACHMENT3 
                         };

    glDrawBuffers(ARRAY_SIZE_IN_ELEMENTS(DrawBuffers), DrawBuffers);
}


void GBuffer::BindForReading()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer);
}


void GBuffer::BindTextures()
{
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, textures[GBUFFER_TEXTURE_TYPE_POSITION]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[GBUFFER_TEXTURE_TYPE_DIFFUSE]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[GBUFFER_TEXTURE_TYPE_NORMAL]);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthTexture);

}


