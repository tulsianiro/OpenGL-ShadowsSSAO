#include "pch.h"
#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const char* filename, bool linear)
{
	glGenTextures(1, &id); 

	unsigned char* pixels = stbi_load(filename, &width, &height, &numChannels, 0);
	if (pixels) // if any pixel data was actually loaded
	{
		GLenum imgFormat = numChannels == 3 ? GL_RGB : GL_RGBA; // assuming RGB or RGBA
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, imgFormat, width, height, 0, imgFormat, 
					 GL_UNSIGNED_BYTE, pixels);
		glGenerateMipmap(GL_TEXTURE_2D); // get mipmaps of the image

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (linear)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
	}
	else
	{
		std::cout << "Image Loading Error for: " << filename << std::endl;
	}

	stbi_image_free(pixels);
}

void Texture::bind(unsigned int unit)
{
	glActiveTexture(unit);
	glBindTexture(GL_TEXTURE_2D, id);
}
