/*
 * HGTLoader.hpp
 * 
 * This loader loads the .hgt file into RAM and
 * checks its sample count, i.e., its resolution:
 * 1201 x 1201 or 3601 x 3601. It also returns
 * height data.
 * 
 * 01-2027 by madpl
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>


class HGTLoader
{
public:
	struct Data
	{
		int width = 0;
		int height = 0;
		std::vector<int16_t> samples;
		
		bool isValid() const;
		int sampleCount() const;
		int16_t getSample(int x, int y) const;
	};
	
public:
	bool load(const std::string& filePath, Data& outData);
	
private:
	bool detectResolution(std::size_t fileSize, int& outWidth, int& outHeight) const;
	int16_t readBigEndianInt16(unsigned char hi, unsigned char lo) const;
};
