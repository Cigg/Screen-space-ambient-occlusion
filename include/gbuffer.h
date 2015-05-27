#ifndef GBUFFER_H
#define	GBUFFER_H

#include <GL/glew.h>

#define GBUFFER_POSITION_TEXTURE_UNIT 0
#define GBUFFER_DIFFUSE_TEXTURE_UNIT  1
#define GBUFFER_NORMAL_TEXTURE_UNIT   2
#define GBUFFER_TEXCOORD_TEXTURE_UNIT 3

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
    
class GBuffer
{
public:

    enum GBUFFER_TEXTURE_TYPE {
        GBUFFER_TEXTURE_TYPE_POSITION,
        GBUFFER_TEXTURE_TYPE_DIFFUSE,
        GBUFFER_TEXTURE_TYPE_NORMAL,
        // GBUFFER_TEXTURE_TYPE_TEXCOORD,
        GBUFFER_NUM_TEXTURES
    };
    
    GBuffer();

    ~GBuffer();

    bool Init(unsigned int screenWidth, unsigned int screenHeight);

    void BindForWriting();
   
    void BindForReading();  
    
    void BindTextures();

private:
    GLuint frameBuffer;
    GLuint depthTexture;
    GLuint textures[GBUFFER_NUM_TEXTURES];
};

#endif

