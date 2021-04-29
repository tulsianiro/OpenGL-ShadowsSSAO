#pragma once

struct Texture
{
	Texture(const char* filename, bool linear = true);
	void bind(unsigned int unit);
	
	unsigned int id;
	int width, height, numChannels;
};