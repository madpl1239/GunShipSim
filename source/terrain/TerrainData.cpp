/*
 * TerrainData.cpp
 * 
 * 02-07-2026 by madpl
 */
#include <algorithm>
#include <cmath>
#include <terrain/TerrainData.hpp>


TerrainData::TerrainData():
	m_width(0),
	m_height(0),
	m_missionSizeMeters(0.0f),
	m_heightScale(1.0f),
	m_minHeight(0.0f),
	m_maxHeight(0.0f)
{
	// empty
}


void TerrainData::clear()
{
	m_width = 0;
	m_height = 0;
	m_missionSizeMeters = 0.0f;
	m_heightScale = 1.0f;
	m_minHeight = 0.0f;
	m_maxHeight = 0.0f;
	m_heights.clear();
}


bool TerrainData::buildFromHGT(const std::vector<int16_t>& sourceSamples,
							   int sourceWidth, int sourceHeight,
							   float tileSouthLat, float tileWestLon,
							   float missionCenterLat, float missionCenterLon,
							   float missionSizeMeters, int targetResolution)
{
	clear();
	
	if(sourceSamples.empty() or sourceWidth <= 1 or sourceHeight <= 1)
		return false;
	
	if(static_cast<std::size_t>(sourceWidth * sourceHeight) != sourceSamples.size())
		return false;
	
	if(missionSizeMeters <= 0.0f or targetResolution <= 1)
		return false;
	
	m_width = targetResolution;
	m_height = targetResolution;
	m_missionSizeMeters = missionSizeMeters;
	m_heightScale = 1.0f;
	
	m_heights.resize(static_cast<std::size_t>(m_width * m_height), 0.0f);
	
	const float tileNorthLat = tileSouthLat + 1.0f;
	const float tileEastLon = tileWestLon + 1.0f;
	
	const float halfMissionSizeMeters = missionSizeMeters * 0.5f;
	
	const float pi = 3.1415926535f;
	const float missionCenterLatRad = missionCenterLat * pi / 180.0f;
	
	const float metersPerDegreeLat = 111111.0f;
	const float metersPerDegreeLon = 111111.0f * std::cos(missionCenterLatRad);
	
	if(metersPerDegreeLon <= 0.0001f)
		return false;
	
	const float deltaLat = halfMissionSizeMeters / metersPerDegreeLat;
	const float deltaLon = halfMissionSizeMeters / metersPerDegreeLon;
	
	const float cropSouthLat = missionCenterLat - deltaLat;
	const float cropNorthLat = missionCenterLat + deltaLat;
	const float cropWestLon = missionCenterLon - deltaLon;
	const float cropEastLon = missionCenterLon + deltaLon;
	
	float cropMinU = (cropWestLon - tileWestLon) / (tileEastLon - tileWestLon);
	float cropMaxU = (cropEastLon - tileWestLon) / (tileEastLon - tileWestLon);
	
	float cropMinV = (tileNorthLat - cropNorthLat) / (tileNorthLat - tileSouthLat);
	float cropMaxV = (tileNorthLat - cropSouthLat) / (tileNorthLat - tileSouthLat);
	
	cropMinU = std::clamp(cropMinU, 0.0f, 1.0f);
	cropMaxU = std::clamp(cropMaxU, 0.0f, 1.0f);
	cropMinV = std::clamp(cropMinV, 0.0f, 1.0f);
	cropMaxV = std::clamp(cropMaxV, 0.0f, 1.0f);
	
	if(cropMinU >= cropMaxU or cropMinV >= cropMaxV)
		return false;
	
	for(int z = 0; z < m_height; ++z)
	{
		float v = (m_height == 1) ? 0.0f : static_cast<float>(z) / static_cast<float>(m_height - 1);
		
		for(int x = 0; x < m_width; ++x)
		{
			float u = (m_width == 1) ? 0.0f : static_cast<float>(x) / static_cast<float>(m_width - 1);
			
			float sourceU = cropMinU + u * (cropMaxU - cropMinU);
			float sourceV = cropMinV + v * (cropMaxV - cropMinV);
			
			float sourceHeightValue = sampleSourceAtNormalized(sourceSamples, sourceWidth, sourceHeight,
															   sourceU, sourceV);
			
			m_heights[static_cast<std::size_t>(z * m_width + x)] = sourceHeightValue;
		}
	}
	
	auto minmax = std::minmax_element(m_heights.begin(), m_heights.end());
	if(minmax.first != m_heights.end() && minmax.second != m_heights.end())
	{
		m_minHeight = *minmax.first;
		m_maxHeight = *minmax.second;
	}
	
	return true;
}


bool TerrainData::isValid() const
{
	return m_width > 0 and m_height > 0 and !m_heights.empty();
}


int TerrainData::getWidth() const
{
	return m_width;
}


int TerrainData::getHeight() const
{
	return m_height;
}


float TerrainData::getMissionSizeMeters() const
{
	return m_missionSizeMeters;
}


float TerrainData::getHeightScale() const
{
	return m_heightScale;
}


float TerrainData::getMinHeight() const
{
	return m_minHeight;
}


float TerrainData::getMaxHeight() const
{
	return m_maxHeight;
}


float TerrainData::getHeightAtGrid(int x, int z) const
{
	if(not isValid())
		return 0.0f;
	
	if(x < 0 or z < 0 or x >= m_width or z >= m_height)
		return 0.0f;
	
	return m_heights[static_cast<std::size_t>(z * m_width + x)];
}


float TerrainData::getHeightAtWorld(float worldX, float worldZ) const
{
	if(not isValid())
		return 0.0f;
	
	if(m_missionSizeMeters <= 0.0f)
		return 0.0f;
	
	float normalizedX = worldX / m_missionSizeMeters;
	float normalizedZ = worldZ / m_missionSizeMeters;
	
	normalizedX = std::clamp(normalizedX, 0.0f, 1.0f);
	normalizedZ = std::clamp(normalizedZ, 0.0f, 1.0f);
	
	float gridX = normalizedX * static_cast<float>(m_width - 1);
	float gridZ = normalizedZ * static_cast<float>(m_height - 1);
	
	int x0 = static_cast<int>(std::floor(gridX));
	int z0 = static_cast<int>(std::floor(gridZ));
	int x1 = std::min(x0 + 1, m_width - 1);
	int z1 = std::min(z0 + 1, m_height - 1);
	
	float tx = gridX - static_cast<float>(x0);
	float tz = gridZ - static_cast<float>(z0);
	
	float h00 = getHeightAtGrid(x0, z0);
	float h10 = getHeightAtGrid(x1, z0);
	float h01 = getHeightAtGrid(x0, z1);
	float h11 = getHeightAtGrid(x1, z1);
	
	float h0 = h00 * (1.0f - tx) + h10 * tx;
	float h1 = h01 * (1.0f - tx) + h11 * tx;
	
	return h0 * (1.0f - tz) + h1 * tz;
}


float TerrainData::getHeightAtWorldPosition(float worldX, float worldZ) const
{
	if(m_heights.empty() or m_width <= 1 or m_height <= 1 or m_missionSizeMeters <= 0.0f)
		return 0.0f;
	
	const float halfSize = m_missionSizeMeters * 0.5f;
	
	float u = (worldX + halfSize) / m_missionSizeMeters;
	float v = (worldZ + halfSize) / m_missionSizeMeters;
	
	u = std::clamp(u, 0.0f, 1.0f);
	v = std::clamp(v, 0.0f, 1.0f);
	
	float gridX = u * static_cast<float>(m_width - 1);
	float gridZ = v * static_cast<float>(m_height - 1);
	
	int x0 = static_cast<int>(std::floor(gridX));
	int z0 = static_cast<int>(std::floor(gridZ));
	
	int x1 = std::min(x0 + 1, m_width - 1);
	int z1 = std::min(z0 + 1, m_height - 1);
	
	float tx = gridX - static_cast<float>(x0);
	float tz = gridZ - static_cast<float>(z0);
	
	float h00 = m_heights[static_cast<std::size_t>(z0 * m_width + x0)];
	float h10 = m_heights[static_cast<std::size_t>(z0 * m_width + x1)];
	float h01 = m_heights[static_cast<std::size_t>(z1 * m_width + x0)];
	float h11 = m_heights[static_cast<std::size_t>(z1 * m_width + x1)];
	
	float hx0 = h00 + (h10 - h00) * tx;
	float hx1 = h01 + (h11 - h01) * tx;
	float h = hx0 + (hx1 - hx0) * tz;
	
	return h;
}


float TerrainData::getWorldSizeX() const
{
	return m_missionSizeMeters;
}


float TerrainData::getWorldSizeZ() const
{
	return m_missionSizeMeters;
}


const std::vector<float>& TerrainData::getHeights() const
{
	return m_heights;
}


float TerrainData::sampleSourceAtNormalized(const std::vector<int16_t>& sourceSamples,
											int sourceWidth, int sourceHeight,
											float u, float v) const
{
	float x = u * static_cast<float>(sourceWidth - 1);
	float z = v * static_cast<float>(sourceHeight - 1);
	
	return sampleSourceBilinear(sourceSamples, sourceWidth, sourceHeight, x, z);
}


float TerrainData::sampleSourceBilinear(const std::vector<int16_t>& sourceSamples,
										int sourceWidth, int sourceHeight, float x, float z) const
{
	x = std::clamp(x, 0.0f, static_cast<float>(sourceWidth - 1));
	z = std::clamp(z, 0.0f, static_cast<float>(sourceHeight - 1));

	int x0 = static_cast<int>(std::floor(x));
	int z0 = static_cast<int>(std::floor(z));
	int x1 = std::min(x0 + 1, sourceWidth - 1);
	int z1 = std::min(z0 + 1, sourceHeight - 1);

	float tx = x - static_cast<float>(x0);
	float tz = z - static_cast<float>(z0);

	auto at = [&](int sx, int sz) -> float
	{
		int16_t raw = sourceSamples[static_cast<std::size_t>(sz * sourceWidth + sx)];
		
		if(raw == static_cast<int16_t>(-32768))
		{
			return 0.0f;
		}
		
		return static_cast<float>(raw);
	};

	float h00 = at(x0, z0);
	float h10 = at(x1, z0);
	float h01 = at(x0, z1);
	float h11 = at(x1, z1);

	float h0 = h00 * (1.0f - tx) + h10 * tx;
	float h1 = h01 * (1.0f - tx) + h11 * tx;

	return h0 * (1.0f - tz) + h1 * tz;
}
