/*
 * TerrainData.hpp
 * 
 * 02-07-2026 by madpl
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>


class TerrainData
{
public:
	struct Sample
	{
		float x = 0.0f;
		float z = 0.0f;
		float height = 0.0f;
	};
	
	TerrainData();
	
	void clear();
	
	bool buildFromHGT(const std::vector<int16_t>& sourceSamples,
						int sourceWidth, int sourceHeight,
						float tileSouthLat, float tileWestLon,
						float missionCenterLat, float missionCenterLon,
						float missionSizeMeters, int targetResolution);
	
	bool isValid() const;
	
	int getWidth() const;
	int getHeight() const;
	
	float getMissionSizeMeters() const;
	float getHeightScale() const;
	
	float getMinHeight() const;
	float getMaxHeight() const;
	
	float getHeightAtGrid(int x, int z) const;
	float getHeightAtWorld(float worldX, float worldZ) const;
	float getHeightAtWorldPosition(float worldX, float worldZ) const;
	
	float getWorldSizeX() const;
	float getWorldSizeZ() const;
	
	const std::vector<float>& getHeights() const;
	
private:
	float sampleSourceAtNormalized(const std::vector<int16_t>& sourceSamples,
								   int sourceWidth, int sourceHeight, float u, float v) const;
	
	float sampleSourceBilinear(const std::vector<int16_t>& sourceSamples,
							   int sourceWidth, int sourceHeight, float x, float z) const;

private:
	int m_width;
	int m_height;
	
	float m_missionSizeMeters;
	float m_heightScale;
	
	float m_minHeight;
	float m_maxHeight;
	
	std::vector<float> m_heights;
};
