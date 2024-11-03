//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef RECASTEDITORCOMMON_H
#define RECASTEDITORCOMMON_H
#include "Recast/Include/Recast.h"
#include "NavEditor/Include/ChunkyTriMesh.h"

class Editor_StaticTileMeshCommon : public Editor
{
public:
	Editor_StaticTileMeshCommon();
	void cleanup();

	void renderRecastDebugMenu();
	void renderTileMeshData();

	void renderIntermediateTileMeshOptions();

	inline unsigned int getTileMeshDrawFlags() const { return m_tileMeshDrawFlags; }
	inline void setTileMeshDrawFlags(unsigned int flags) { m_tileMeshDrawFlags = flags; }

	inline void toggleTileMeshDrawFlag(unsigned int flag) { m_tileMeshDrawFlags ^= flag; }

protected:
	unsigned char* m_triareas;

	rcHeightfield* m_solid;
	rcCompactHeightfield* m_chf;
	rcContourSet* m_cset;
	rcPolyMesh* m_pmesh;
	rcPolyMeshDetail* m_dmesh;
	rcConfig m_cfg;

	unsigned int m_tileMeshDrawFlags;
	unsigned int m_tileCol;

	float m_lastBuiltTileBmin[3];
	float m_lastBuiltTileBmax[3];

	double m_totalBuildTimeMs;

	bool m_drawActiveTile;
	bool m_keepInterResults;
};

class Editor_DynamicTileMeshCommon : public Editor
{
public:
	Editor_DynamicTileMeshCommon();

	void renderRecastRenderOptions();
	void renderTileMeshData();

	inline unsigned int getTileMeshDrawFlags() const { return m_tileMeshDrawFlags; }
	inline void setTileMeshDrawFlags(unsigned int flags) { m_tileMeshDrawFlags = flags; }

	inline void toggleTileMeshDrawFlag(unsigned int flag) { m_tileMeshDrawFlags ^= flag; }

protected:
	class dtTileCache* m_tileCache;
	struct LinearAllocator* m_talloc;
	struct FastLZCompressor* m_tcomp;
	struct MeshProcess* m_tmproc;

	double m_cacheBuildTimeMs;
	int m_cacheCompressedSize;
	int m_cacheRawSize;
	int m_cacheLayerCount;

	unsigned int m_cacheBuildMemUsage;
	unsigned int m_tileMeshDrawFlags;

	bool m_keepInterResults;
};

int EditorCommon_SetAndRenderTileProperties(const InputGeom* const geom, 
	const int minTilebits, const int maxTileBits, const int tileSize,
	const float cellSize, int& maxTiles, int& maxPolysPerTile);

#endif // RECASTEDITORCOMMON_H
