/*
 * HGTLoader.cpp
 * 
 * Implementation.
 * 
 * 01-2027 by madpl
 */
#include <fstream>
#include <terrain/HGTLoader.hpp>


bool HGTLoader::Data::isValid() const
{
	return (width > 0 and height > 0 and !samples.empty());
}


int HGTLoader::Data::sampleCount() const
{
	return static_cast<int>(samples.size());
}


int16_t HGTLoader::Data::getSample(int x, int y) const
{
	if(x < 0 or y < 0 or x >= width or y >= height)
		return 0;
	
	return samples[y * width + x];
}


bool HGTLoader::load(const std::string& filePath, Data& outData)
{
	outData = Data();
	
	std::ifstream file(filePath, std::ios::binary);
	if(not file.is_open())
		return false;
	
	file.seekg(0, std::ios::end);
	std::size_t fileSize = static_cast<std::size_t>(file.tellg());
	file.seekg(0, std::ios::beg);
	
	int width = 0;
	int height = 0;
	
	if(not detectResolution(fileSize, width, height))
		return false;
	
	const int totalSamples = width * height;
	outData.width = width;
	outData.height = height;
	outData.samples.resize(totalSamples);
	
	for(int i = 0; i < totalSamples; ++i)
	{
		unsigned char bytes[2] = {0, 0};
		file.read(reinterpret_cast<char*>(bytes), 2);
		
		if(not file)
		{
			outData = Data();
			
			return false;
		}
		
		outData.samples[i] = readBigEndianInt16(bytes[0], bytes[1]);
	}
	
	return true;
}


bool HGTLoader::detectResolution(std::size_t fileSize, int& outWidth, int& outHeight) const
{
	outWidth = 0;
	outHeight = 0;
	
	const std::size_t size1201 = 1201ull * 1201ull * 2ull;
	const std::size_t size3601 = 3601ull * 3601ull * 2ull;
	
	if(fileSize == size1201)
	{
		outWidth = 1201;
		outHeight = 1201;
		
		return true;
	}
	
	if(fileSize == size3601)
	{
		outWidth = 3601;
		outHeight = 3601;
		
		return true;
	}
	
	return false;
}


int16_t HGTLoader::readBigEndianInt16(unsigned char hi, unsigned char lo) const
{
	return static_cast<int16_t>((static_cast<int16_t>(hi) << 8) |
									static_cast<int16_t>(lo));
}
