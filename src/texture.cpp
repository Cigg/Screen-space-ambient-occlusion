#include <string>
#include <iostream>
#include <FreeImagePlus.h>
#include <GL/glew.h>

#include "texture.h"

// GLuint LoadTGAFile(const char *filename)
// {
// 	unsigned char imageTypeCode;
//     short int imageWidth;
//     short int imageHeight;
//     unsigned char bitCount;
//     unsigned char *imageData;

//     unsigned char ucharBad;
//     short int sintBad;
//     long imageSize;
//     int colorMode;
//     unsigned char colorSwap;

//     // Open the TGA file.
//     FILE *filePtr = fopen(filename, "rb");
//     if (filePtr == NULL) {
//     	printf("%s could not be opened.\n", filename);
//     	return 0;
//     }

//     // Read the two first bytes we don't need.
//     fread(&ucharBad, sizeof(unsigned char), 1, filePtr);
//     fread(&ucharBad, sizeof(unsigned char), 1, filePtr);

//     // Which type of image gets stored in imageTypeCode.
//     fread(&imageTypeCode, sizeof(unsigned char), 1, filePtr);

//     // For our purposes, the type code should be 2 (uncompressed RGB image)
//     // or 3 (uncompressed black-and-white images).
//     if (imageTypeCode != 2 && imageTypeCode != 3)
//     {
//     	printf("%s wrong TGA type.\n", filename);
//         fclose(filePtr);
//         return false;
//     }

//     //std::cout << "dataTypeCode: " << (unsigned int)imageTypeCode << std::endl;

//     // Read 13 bytes of data we don't need.
//     fread(&sintBad, sizeof(short int), 1, filePtr);
//     fread(&sintBad, sizeof(short int), 1, filePtr);
//     fread(&ucharBad, sizeof(unsigned char), 1, filePtr);
//     fread(&sintBad, sizeof(short int), 1, filePtr);
//     fread(&sintBad, sizeof(short int), 1, filePtr);
    
//     // Read the image's width and height.
//     fread(&imageWidth, sizeof(short int), 1, filePtr);
//     fread(&imageHeight, sizeof(short int), 1, filePtr);
//     //std::cout << "imageWidth: " << imageWidth << std::endl;
//     //std::cout << "imageHeight: " << imageHeight << std::endl;

//     // Read the bit depth.
//     fread(&bitCount, sizeof(unsigned char), 1, filePtr);
//     //std::cout << "bitCount: " << (unsigned int)bitCount << std::endl;

//     // Read one byte of data we don't need.
//     fread(&ucharBad, sizeof(unsigned char), 1, filePtr);

//     // Color mode -> 3 = BGR, 4 = BGRA.
//     colorMode = bitCount / 8;
//     imageSize = imageWidth * imageHeight * colorMode;

//     // Allocate memory for the image data.
//     imageData = (unsigned char*)malloc(sizeof(unsigned char)*imageSize);

//     // Read the image data.
//     fread(imageData, sizeof(unsigned char), imageSize, filePtr);

//     // Change from BGR to RGB so OpenGL can read the image data.
//     for (int imageIdx = 0; imageIdx < imageSize; imageIdx += colorMode)
//     {
//         colorSwap = imageData[imageIdx];
//         imageData[imageIdx] = imageData[imageIdx + 2];
//         imageData[imageIdx + 2] = colorSwap;
//     }

//    	// Everything is in memory now, the file wan be closed
//     fclose(filePtr);
   
// 	// Create one OpenGL texture
// 	GLuint textureID;
// 	glGenTextures(1, &textureID);
	
// 	// "Bind" the newly created texture : all future texture functions will modify this texture
// 	glBindTexture(GL_TEXTURE_2D, textureID);

// 	// Give the image to OpenGL
// 	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, imageData);

//    	// OpenGL has now copied the data. Free our own version
// 	//delete [] imageData;
// 	free(imageData);

// 	// Poor filtering, or ...
// 	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
// 	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

// 	// ... nice trilinear filtering.
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
// 	glGenerateMipmap(GL_TEXTURE_2D);

// 	// Return the ID of the texture we just created
// 	return textureID;
// }

// Method to load an image into a texture using the freeimageplus library. Returns the texture ID or dies trying.
GLuint loadTexture(std::string filenameString, GLenum minificationFilter = GL_LINEAR, GLenum magnificationFilter = GL_LINEAR) {
    // Get the filename as a pointer to a const char array to play nice with FreeImage
    const char* filename = filenameString.c_str();

    // Determine the format of the image.
    // Note: The second paramter ('size') is currently unused, and we should use 0 for it.
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename , 0);

    // Image not found? Abort! Without this section we get a 0 by 0 image with 0 bits-per-pixel but we don't abort, which
    // you might find preferable to dumping the user back to the desktop.
    if (format == -1) {
        std::cout << "Could not find image: " << filenameString << " - Aborting." << std::endl;
        return 0;
    }

    // Found image, but couldn't determine the file format? Try again...
    if (format == FIF_UNKNOWN)
    {
        //std::cout << "Couldn't determine file format - attempting to get from file extension..." << std::endl;

        // ...by getting the filetype from the filename extension (i.e. .PNG, .GIF etc.)
        // Note: This is slower and more error-prone that getting it from the file itself,
        // also, we can't use the 'U' (unicode) variant of this method as that's Windows only.
        format = FreeImage_GetFIFFromFilename(filename);

        // Check that the plugin has reading capabilities for this format (if it's FIF_UNKNOWN,
        // for example, then it won't have) - if we can't read the file, then we bail out =(
        if ( !FreeImage_FIFSupportsReading(format) ) {
            std::cout << "Detected image format cannot be read!" << std::endl;
            return 0;
        }
    }

    // If we're here we have a known image format, so load the image into a bitmap
    FIBITMAP* bitmap = FreeImage_Load(format, filename);

    // How many bits-per-pixel is the source image?
    int bitsPerPixel =  FreeImage_GetBPP(bitmap);

    // Convert our image up to 32 bits (8 bits per channel, Red/Green/Blue/Alpha) -
    // but only if the image is not already 32 bits (i.e. 8 bits per channel).
    // Note: ConvertTo32Bits returns a CLONE of the image data - so if we
    // allocate this back to itself without using our bitmap32 intermediate
    // we will LEAK the original bitmap data
    FIBITMAP* bitmap32;
    if (bitsPerPixel == 32) {
        //std::cout << "Source image has " << bitsPerPixel << " bits per pixel. Skipping conversion." << std::endl;
        bitmap32 = bitmap;
    }
    else {
        //std::cout << "Source image has " << bitsPerPixel << " bits per pixel. Converting to 32-bit colour." << std::endl;
        bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
    }

    // Some basic image info - strip it out if you don't care
    int imageWidth  = FreeImage_GetWidth(bitmap32);
    int imageHeight = FreeImage_GetHeight(bitmap32);
    //std::cout << "Image: " << filenameString << " is size: " << imageWidth << "x" << imageHeight << "." << std::endl;

    // Get a pointer to the texture data as an array of unsigned bytes.
    // Note: At this point bitmap32 ALWAYS holds a 32-bit colour version of our image - so we get our data from that.
    // Also, we don't need to delete or delete[] this textureData because it's not on the heap (so attempting to do
    // so will cause a crash) - just let it go out of scope and the memory will be returned to the stack.
    GLubyte* textureData = FreeImage_GetBits(bitmap32);

    // Generate a texture ID and bind to it
    GLuint tempTextureID;
    glGenTextures(1, &tempTextureID);
    glBindTexture(GL_TEXTURE_2D, tempTextureID);

    // Construct the texture.
    // Note: The 'Data format' is the format of the image data as provided by the image library. FreeImage decodes images into
    // BGR/BGRA format, but we want to work with it in the more common RGBA format, so we specify the 'Internal format' as such.
    glTexImage2D(GL_TEXTURE_2D,    // Type of texture
                 0,                // Mipmap level (0 being the top level i.e. full size)
                 GL_RGBA,          // Internal format
                 imageWidth,       // Width of the texture
                 imageHeight,      // Height of the texture,
                 0,                // Border in pixels
                 GL_BGRA,          // Data format
                 GL_UNSIGNED_BYTE, // Type of texture data
                 textureData);     // The image data to use for this texture

    // Specify our minification and magnification filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minificationFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnificationFilter);

    // If we're using MipMaps, then we'll generate them here.
    // Note: The glGenerateMipmap call requires OpenGL 3.0 as a minimum.
    if (minificationFilter == GL_LINEAR_MIPMAP_LINEAR   ||
        minificationFilter == GL_LINEAR_MIPMAP_NEAREST  ||
        minificationFilter == GL_NEAREST_MIPMAP_LINEAR  ||
        minificationFilter == GL_NEAREST_MIPMAP_NEAREST) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // Check for OpenGL texture creation errors
    GLenum glError = glGetError();
    if(glError) {
        std::cout << "There was an error loading the texture: "<< filenameString << std::endl;
        switch (glError) {
            case GL_INVALID_ENUM:
                std::cout << "Invalid enum." << std::endl;
                break;

            case GL_INVALID_VALUE:
                std::cout << "Invalid value." << std::endl;
                break;

            case GL_INVALID_OPERATION:
                std::cout << "Invalid operation." << std::endl;

            default:
                std::cout << "Unrecognised GLenum." << std::endl;
                break;
        }
    }

    // Unload the 32-bit colour bitmap
    FreeImage_Unload(bitmap32);

    // If we had to do a conversion to 32-bit colour, then unload the original
    // non-32-bit-colour version of the image data too. Otherwise, bitmap32 and
    // bitmap point at the same data, and that data's already been free'd, so
    // don't attempt to free it again! (or we'll crash).
    if (bitsPerPixel != 32)
    {
        FreeImage_Unload(bitmap);
    }

    // Finally, return the texture ID
    return tempTextureID;
}