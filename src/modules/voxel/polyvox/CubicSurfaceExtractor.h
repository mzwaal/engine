/**
 * @file
 */

#pragma once

#include "Array.h"
#include "Mesh.h"
#include "Voxel.h"
#include "Vertex.h"
#include <vector>
#include <list>
#include "core/Trace.h"

namespace voxel {

/**
 * This constant defines the maximum number of quads which can share a vertex in a cubic style mesh.
 *
 * We try to avoid duplicate vertices by checking whether a vertex has already been added at a given position.
 * However, it is possible that vertices have the same position but different materials. In this case, the
 * vertices are not true duplicates and both must be added to the mesh. As far as I can tell, it is possible to have
 * at most eight vertices with the same position but different materials. For example, this worst-case scenario
 * happens when we have a 2x2x2 group of voxels, all with different materials and some/all partially transparent.
 * The vertex position at the center of this group is then going to be used by all eight voxels all with different
 * materials.
 */
const uint32_t MaxVerticesPerPosition = 8;

/**
 * @section Data structures
 */
enum FaceNames {
	PositiveX, PositiveY, PositiveZ, NegativeX, NegativeY, NegativeZ, NoOfFaces
};

inline bool isHorizontalFace(FaceNames face) {
	return face == PositiveX || face == PositiveZ || face == NegativeX || face == NegativeZ;
}

inline bool isVerticalFace(FaceNames face) {
	 return face == PositiveY || face == NegativeY;
}

struct Quad {
	Quad(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3) {
		vertices[0] = v0;
		vertices[1] = v1;
		vertices[2] = v2;
		vertices[3] = v3;
	}

	uint32_t vertices[4];
};

struct IndexAndMaterial {
	int32_t iIndex;
	Voxel uMaterial;
};

/**
 * @section Vertex encoding/decoding
 */

// 0 is the darkest
// 3 is no occlusion at all
inline uint8_t vertexAmbientOcclusion(bool side1, bool side2, bool corner) {
	if (side1 && side2) {
		return 0;
	}
	return 3 - (side1 + side2 + corner);
}

/**
 * @section Surface extraction
 */

extern bool mergeQuads(Quad& q1, Quad& q2, Mesh<Vertex>* m_meshCurrent);

extern bool performQuadMerging(std::list<Quad>& quads, Mesh<Vertex>* m_meshCurrent);

extern int32_t addVertex(uint32_t uX, uint32_t uY, uint32_t uZ, const Voxel& uMaterialIn, Array<3, IndexAndMaterial>& existingVertices,
		Mesh<Vertex>* m_meshCurrent, const Voxel& face1, const Voxel& face2, const Voxel& corner);

/**
 * The CubicSurfaceExtractor creates a mesh in which each voxel appears to be rendered as a cube
 * Introduction
 *  ------------
 *  Games such as Minecraft and Voxatron have a unique graphical style in which each voxel in the world appears to be rendered
 *  as a single cube. Actually rendering a cube for each voxel would be very expensive, but in practice the only faces which need
 *  to be drawn are those which lie on the boundary between solid and empty voxels. The CubicSurfaceExtractor can be used to create
 *  such a mesh from PolyVox volume data. As an example, images from Minecraft and Voxatron are shown below:
 *
 *  \image html MinecraftAndVoxatron.jpg
 *
 *  Before we get into the specifics of the CubicSurfaceExtractor, it is useful to understand the principles which apply to *all* PolyVox
 *  surface extractors and which are described in the Surface Extraction document (ADD LINK). From here on, it is assumed that you
 *  are familier with PolyVox regions and how they are used to limit surface extraction to a particular part of the volume. The
 *  principles of allowing dynamic terrain are also common to all surface extractors and are described here (ADD LINK).
 *
 *  Basic Operation
 *  ---------------
 *  At its core, the CubicSurfaceExtractor works by by looking at pairs of adjacent voxels and determining whether a quad should be
 *  placed between then. The most simple situation to imagine is a binary volume where every voxel is either solid or empty. In this
 *  case a quad should be generated whenever a solid voxel is next to an empty voxel as this represents part of the surface of the
 *  solid object. There is no need to generate a quad between two solid voxels (this quad would never be seen as it is inside the
 *  object) and there is no need to generate a quad between two empty voxels (there is no object here). PolyVox allows the principle
 *  to be extended far beyond such simple binary volumes but they provide a useful starting point for understanding how the algorithm
 *  works.
 *
 *  As an example, lets consider the part of a volume shown below. We are going to explain the principles in only two dimensions as
 *  this makes it much simpler to illustrate, so you will need to mentally extend the process into the third dimension. Hopefully you will
 *  find this intuitive. The diagram below shows a small part of a larger volume (as indicated by the voxel coordinates on the axes) which
 *  contains only solid and empty voxels represented by solid and hollow circles respectively. The region on which we are running the
 *  surface extractor is marked in pink, and for the purpose of this example it corresponds to the whole of the diagram.
 *
 *  \image html CubicSurfaceExtractor1.png
 *
 *  The output of the surface extractor is the mesh marked in red. As you can see, this forms a closed object which corresponds to the
 *  shape of the underlying voxel data.
 *
 *  Working with Regions
 *  --------------------
 *   So far the behaviour is easy to understand, but let's look at what happens when the extraction is limited to a particular region of
 *   the volume. The figure below shows the same data set as the previous figure, but the extraction region (still marked in pink) has
 *   been limited to 13 to 16 in x and 47 to 51 in y:
 *
 *  \image html CubicSurfaceExtractor2.png
 *
 *  As you can see, the extractor continues to generate a number of quads as indicated by the solid red lines. However, you can also see
 *  that the shape is no longer closed. This is because the solid voxels actually extend outside the region which is being processed, and
 *  so the extractor does not encounter a boundary between solid and empty voxels. Although this may initially appear problematic, the
 *  hole in the mesh does not actually matter because it will be hidden by the mesh corresponding to the region adjacent to it (see next
 *  diagram).
 *
 *  More interestingly, the diagram also contains a couple of dotted red lines lying on the bottom and right hand side of the extracted
 *  region. These are present to illustrate a common point of confusion, which is that *no quads are generated at this position even though
 *  it is a boundary between solid and empty voxels*. This is indeed somewhat counter intuitive but there is a rational reasaoning behind
 *  it.
 *  If you consider the dashed line on the righthand side of the extracted region, then it is clear that this lies on a boundary between
 *  solid and empty voxels and so we do need to create quads here. But what is not so clear is whether these quads should be assigned to
 *  the mesh which corresponds to the region in pink, or whether they should be assigned to the region to the right of it which is marked
 *  in blue in the diagram below:
 *
 *  \image html CubicSurfaceExtractor3.png
 *
 *  We could choose to add the quads to *both* regions, but this can cause confusion when one of the region is modified (causing the face
 *  to disappear or a new one to be created) as *both* regions need to have their mesh regenerated to correctly represent the new state of
 *  the volume data. Such pairs of coplanar quads can also cause problems with physics engines, and may prevent transparent voxels from
 *  rendering correctly. Therefore we choose to instead only add the quad to one of the the regions and we always choose the one with the
 *  greater coordinate value in the direction in which they differ. In the above example the regions differ by the 'x' component of their
 *  position, and so the quad is added to the region with the greater 'x' value (the one marked in blue).
 *
 *  **Note:** *This behaviour has changed recently (September 2012). Earlier versions of PolyVox tried to be smart about this problem by
 *  looking beyond the region which was being processed, but this complicated the code and didn't work very well. Ultimatly we decided to
 *  simply stick with the convention outlined above.*
 *
 *  One of the practical implications of this is that when you modify a voxel *you may have to re-extract the mesh for regions other than
 *  region which actually contains the voxel you modified.* This happens when the voxel lies on the upper x,y or z face of a region.
 *  Assuming that you have some management code which can mark a region as needing re-extraction when a voxel changes, you should probably
 *  extend this to mark the regions of neighbouring voxels as invalid (this will have no effect when the voxel is well within a region,
 *  but will mark the neighbouring region as needing an update if the voxel lies on a region face).
 *
 *  Another scenario which sometimes results in confusion is when you wish to extract a region which corresponds to the whole volume,
 *  particularly when solid voxels extend right to the edge of the volume.
 */
template<typename VolumeType, typename IsQuadNeeded>
Mesh<Vertex> extractCubicMesh(VolumeType* volData, Region region, IsQuadNeeded isQuadNeeded, bool bMergeQuads) {
	Mesh<Vertex> result;
	extractCubicMeshCustom(volData, region, &result, isQuadNeeded, bMergeQuads);
	return result;
}

inline bool isQuadFlipped(const Vertex& v00, const Vertex& v01, const Vertex& v10, const Vertex& v11) {
	return v00.ambientOcclusion + v11.ambientOcclusion > v01.ambientOcclusion + v10.ambientOcclusion;
}

/**
 * This version of the function performs the extraction into a user-provided mesh rather than allocating a mesh automatically.
 * There are a few reasons why this might be useful to more advanced users:
 *
 *    1. It leaves the user in control of memory allocation and would allow them to implement e.g. a mesh pooling system.
 *    2. The user-provided mesh could have a different index type (e.g. 16-bit indices) to reduce memory usage.
 *    3. The user could provide a custom mesh class, e.g a thin wrapper around an openGL VBO to allow direct writing into this structure.
 *
 *  We don't provide a default MeshType here. If the user doesn't want to provide a MeshType then it probably makes
 *  more sense to use the other variant of this function where the mesh is a return value rather than a parameter.
 *
 * @note: This function is called 'extractCubicMeshCustom' rather than 'extractCubicMesh' to avoid ambiguity when only three parameters
 *  are provided (would the third parameter be a controller or a mesh?). It seems this can be fixed by using enable_if/static_assert to emulate concepts,
 *  but this is relatively complex and I haven't done it yet. Could always add it later as another overload.
 */
template<typename VolumeType, typename IsQuadNeeded>
void extractCubicMeshCustom(VolumeType* volData, Region region, Mesh<Vertex>* result, IsQuadNeeded isQuadNeeded, bool bMergeQuads) {
	core_trace_scoped(ExtractCubicMesh);
	// This extractor has a limit as to how large the extracted region can be, because the vertex positions are encoded with a single byte per component.
	int32_t maxRegionDimensionInVoxels = 255;
	core_assert_msg(region.getWidthInVoxels() <= maxRegionDimensionInVoxels, "Requested extraction region exceeds maximum dimensions");
	core_assert_msg(region.getHeightInVoxels() <= maxRegionDimensionInVoxels, "Requested extraction region exceeds maximum dimensions");
	core_assert_msg(region.getDepthInVoxels() <= maxRegionDimensionInVoxels, "Requested extraction region exceeds maximum dimensions");

	result->clear();

	// Used to avoid creating duplicate vertices.
	Array<3, IndexAndMaterial> m_previousSliceVertices(region.getUpperX() - region.getLowerX() + 2, region.getUpperY() - region.getLowerY() + 2,
			MaxVerticesPerPosition);
	Array<3, IndexAndMaterial> m_currentSliceVertices(region.getUpperX() - region.getLowerX() + 2, region.getUpperY() - region.getLowerY() + 2,
			MaxVerticesPerPosition);

	// During extraction we create a number of different lists of quads. All the
	// quads in a given list are in the same plane and facing in the same direction.
	typedef std::list<Quad> QuadList;
	typedef std::vector<QuadList> QuadListVector;
	QuadListVector m_vecQuads[NoOfFaces];

	std::memset(m_previousSliceVertices.getRawData(), 0xff, m_previousSliceVertices.getNoOfElements() * sizeof(IndexAndMaterial));
	std::memset(m_currentSliceVertices.getRawData(), 0xff, m_currentSliceVertices.getNoOfElements() * sizeof(IndexAndMaterial));

	m_vecQuads[NegativeX].resize(region.getUpperX() - region.getLowerX() + 2);
	m_vecQuads[PositiveX].resize(region.getUpperX() - region.getLowerX() + 2);

	m_vecQuads[NegativeY].resize(region.getUpperY() - region.getLowerY() + 2);
	m_vecQuads[PositiveY].resize(region.getUpperY() - region.getLowerY() + 2);

	m_vecQuads[NegativeZ].resize(region.getUpperZ() - region.getLowerZ() + 2);
	m_vecQuads[PositiveZ].resize(region.getUpperZ() - region.getLowerZ() + 2);

	typename VolumeType::Sampler volumeSampler(volData);

	for (int32_t z = region.getLowerZ(); z <= region.getUpperZ(); z++) {
		const uint32_t regZ = z - region.getLowerZ();

		for (int32_t y = region.getLowerY(); y <= region.getUpperY(); y++) {
			const uint32_t regY = y - region.getLowerY();

			volumeSampler.setPosition(region.getLowerX(), y, z);

			for (int32_t x = region.getLowerX(); x <= region.getUpperX(); x++) {
				const uint32_t regX = x - region.getLowerX();
				Voxel material; //Filled in by callback

				/**
				 *
				 *
				 *                  [D]
				 *            8 ____________ 7
				 *             /|          /|
				 *            / |         / |              ABOVE [D] |
				 *           /  |    [F] /  |              BELOW [C]
				 *        5 /___|_______/ 6 |  [B]       y           BEHIND  [F]
				 *    [A]   |   |_______|___|              |      z  BEFORE [E] /
				 *          | 4 /       |   / 3            |   /
				 *          |  / [E]    |  /               |  /   . center
				 *          | /         | /                | /
				 *          |/__________|/                 |/________   LEFT  RIGHT
				 *        1               2                          x   [A] - [B]
				 *               [C]
				 */

				const Voxel voxelCurrent          = volumeSampler.getVoxel();
				const Voxel voxelLeft             = volumeSampler.peekVoxel1nx0py0pz();
				const Voxel voxelBefore           = volumeSampler.peekVoxel0px0py1nz();
				const Voxel voxelLeftBefore       = volumeSampler.peekVoxel1nx0py1nz();
				const Voxel voxelRightBefore      = volumeSampler.peekVoxel1px0py1nz();
				const Voxel voxelLeftBehind       = volumeSampler.peekVoxel1nx0py1pz();

				const Voxel voxelAboveLeft        = volumeSampler.peekVoxel1nx1py0pz();
				const Voxel voxelAboveBefore      = volumeSampler.peekVoxel0px1py1nz();
				const Voxel voxelAboveLeftBefore  = volumeSampler.peekVoxel1nx1py1nz();
				const Voxel voxelAboveRightBefore = volumeSampler.peekVoxel1px1py1nz();
				const Voxel voxelAboveLeftBehind  = volumeSampler.peekVoxel1nx1py1pz();

				const Voxel voxelBelow            = volumeSampler.peekVoxel0px1ny0pz();
				const Voxel voxelBelowLeft        = volumeSampler.peekVoxel1nx1ny0pz();
				const Voxel voxelBelowRight       = volumeSampler.peekVoxel1px1ny0pz();
				const Voxel voxelBelowBefore      = volumeSampler.peekVoxel0px1ny1nz();
				const Voxel voxelBelowBehind      = volumeSampler.peekVoxel0px1ny1pz();
				const Voxel voxelBelowLeftBefore  = volumeSampler.peekVoxel1nx1ny1nz();
				const Voxel voxelBelowRightBefore = volumeSampler.peekVoxel1px1ny1nz();
				const Voxel voxelBelowLeftBehind  = volumeSampler.peekVoxel1nx1ny1pz();
				const Voxel voxelBelowRightBehind = volumeSampler.peekVoxel1px1ny1pz();

				// X [A] LEFT
				if (isQuadNeeded(voxelCurrent, voxelLeft, material, NegativeX, x, z)) {
					const uint32_t v_0_1 = addVertex(regX, regY,     regZ,     material, m_previousSliceVertices, result,
							voxelLeftBefore, voxelBelowLeft, voxelBelowLeftBefore);
					const uint32_t v_1_4 = addVertex(regX, regY,     regZ + 1, material, m_currentSliceVertices,  result,
							voxelBelowLeft, voxelLeftBehind, voxelBelowLeftBehind);
					const uint32_t v_2_8 = addVertex(regX, regY + 1, regZ + 1, material, m_currentSliceVertices,  result,
							voxelLeftBehind, voxelAboveLeft, voxelAboveLeftBehind);
					const uint32_t v_3_5 = addVertex(regX, regY + 1, regZ,     material, m_previousSliceVertices, result,
							voxelAboveLeft, voxelLeftBefore, voxelAboveLeftBefore);
					m_vecQuads[NegativeX][regX].push_back(Quad(v_0_1, v_1_4, v_2_8, v_3_5));
				}

				// X [B] RIGHT
				if (isQuadNeeded(voxelLeft, voxelCurrent, material, PositiveX, x, z)) {
					volumeSampler.moveNegativeX();

					const Voxel _voxelRightBefore      = volumeSampler.peekVoxel1px0py1nz();
					const Voxel _voxelRightBehind      = volumeSampler.peekVoxel1px0py1pz();

					const Voxel _voxelAboveRight       = volumeSampler.peekVoxel1px1py0pz();
					const Voxel _voxelAboveRightBefore = volumeSampler.peekVoxel1px1py1nz();
					const Voxel _voxelAboveRightBehind = volumeSampler.peekVoxel1px1py1pz();

					const Voxel _voxelBelowRight       = volumeSampler.peekVoxel1px1ny0pz();
					const Voxel _voxelBelowRightBefore = volumeSampler.peekVoxel1px1ny1nz();
					const Voxel _voxelBelowRightBehind = volumeSampler.peekVoxel1px1ny1pz();

					const uint32_t v_0_2 = addVertex(regX, regY,     regZ,     material, m_previousSliceVertices, result,
							_voxelBelowRight, _voxelRightBefore, _voxelBelowRightBefore);
					const uint32_t v_1_3 = addVertex(regX, regY,     regZ + 1, material, m_currentSliceVertices,  result,
							_voxelBelowRight, _voxelRightBehind, _voxelBelowRightBehind);
					const uint32_t v_2_7 = addVertex(regX, regY + 1, regZ + 1, material, m_currentSliceVertices,  result,
							_voxelAboveRight, _voxelRightBehind, _voxelAboveRightBehind);
					const uint32_t v_3_6 = addVertex(regX, regY + 1, regZ,     material, m_previousSliceVertices, result,
							_voxelAboveRight, _voxelRightBefore, _voxelAboveRightBefore);
					m_vecQuads[PositiveX][regX].push_back(Quad(v_0_2, v_3_6, v_2_7, v_1_3));

					volumeSampler.movePositiveX();
				}

				// Y [C] BELOW
				if (isQuadNeeded(voxelCurrent, voxelBelow, material, NegativeY, x, z)) {
					const uint32_t v_0_1 = addVertex(regX,     regY, regZ,     material, m_previousSliceVertices, result,
							voxelBelowBefore, voxelBelowLeft, voxelBelowLeftBefore);
					const uint32_t v_1_2 = addVertex(regX + 1, regY, regZ,     material, m_previousSliceVertices, result,
							voxelBelowRight, voxelBelowBefore, voxelBelowRightBefore);
					const uint32_t v_2_3 = addVertex(regX + 1, regY, regZ + 1, material, m_currentSliceVertices,  result,
							voxelBelowBehind, voxelBelowRight, voxelBelowRightBehind);
					const uint32_t v_3_4 = addVertex(regX,     regY, regZ + 1, material, m_currentSliceVertices,  result,
							voxelBelowLeft, voxelBelowBehind, voxelBelowLeftBehind);
					m_vecQuads[NegativeY][regY].push_back(Quad(v_0_1, v_1_2, v_2_3, v_3_4));
				}

				// Y [D] ABOVE
				if (isQuadNeeded(voxelBelow, voxelCurrent, material, PositiveY, x, z)) {
					volumeSampler.moveNegativeY();

					const Voxel _voxelAboveLeft        = volumeSampler.peekVoxel1nx1py0pz();
					const Voxel _voxelAboveRight       = volumeSampler.peekVoxel1px1py0pz();
					const Voxel _voxelAboveBefore      = volumeSampler.peekVoxel0px1py1nz();
					const Voxel _voxelAboveBehind      = volumeSampler.peekVoxel0px1py1pz();
					const Voxel _voxelAboveLeftBefore  = volumeSampler.peekVoxel1nx1py1nz();
					const Voxel _voxelAboveRightBefore = volumeSampler.peekVoxel1px1py1nz();
					const Voxel _voxelAboveLeftBehind  = volumeSampler.peekVoxel1nx1py1pz();
					const Voxel _voxelAboveRightBehind = volumeSampler.peekVoxel1px1py1pz();

					const uint32_t v_0_5 = addVertex(regX,     regY, regZ,     material, m_previousSliceVertices, result,
							_voxelAboveBefore, _voxelAboveLeft, _voxelAboveLeftBefore);
					const uint32_t v_1_6 = addVertex(regX + 1, regY, regZ,     material, m_previousSliceVertices, result,
							_voxelAboveRight, _voxelAboveBefore, _voxelAboveRightBefore);
					const uint32_t v_2_7 = addVertex(regX + 1, regY, regZ + 1, material, m_currentSliceVertices,  result,
							_voxelAboveBehind, _voxelAboveRight, _voxelAboveRightBehind);
					const uint32_t v_3_8 = addVertex(regX,     regY, regZ + 1, material, m_currentSliceVertices,  result,
							_voxelAboveLeft, _voxelAboveBehind, _voxelAboveLeftBehind);
					m_vecQuads[PositiveY][regY].push_back(Quad(v_0_5, v_3_8, v_2_7, v_1_6));

					volumeSampler.movePositiveY();
				}

				// Z [E] BEFORE
				if (isQuadNeeded(voxelCurrent, voxelBefore, material, NegativeZ, x, z)) {
					const uint32_t v_0_1 = addVertex(regX,     regY,     regZ, material, m_previousSliceVertices, result,
							voxelBelowBefore, voxelLeftBefore, voxelBelowLeftBefore); //1
					const uint32_t v_1_5 = addVertex(regX,     regY + 1, regZ, material, m_previousSliceVertices, result,
							voxelAboveBefore, voxelLeftBefore, voxelAboveLeftBefore); //5
					const uint32_t v_2_6 = addVertex(regX + 1, regY + 1, regZ, material, m_previousSliceVertices, result,
							voxelAboveBefore, voxelRightBefore, voxelAboveRightBefore); //6
					const uint32_t v_3_2 = addVertex(regX + 1, regY,     regZ, material, m_previousSliceVertices, result,
							voxelBelowBefore, voxelRightBefore, voxelBelowRightBefore); //2
					m_vecQuads[NegativeZ][regZ].push_back(Quad(v_0_1, v_1_5, v_2_6, v_3_2));
				}

				// Z [F] BEHIND
				if (isQuadNeeded(voxelBefore, voxelCurrent, material, PositiveZ, x, z)) {
					volumeSampler.moveNegativeZ();

					const Voxel _voxelLeftBehind       = volumeSampler.peekVoxel1nx0py1pz();
					const Voxel _voxelRightBehind      = volumeSampler.peekVoxel1px0py1pz();

					const Voxel _voxelAboveBehind      = volumeSampler.peekVoxel0px1py1pz();
					const Voxel _voxelAboveLeftBehind  = volumeSampler.peekVoxel1nx1py1pz();
					const Voxel _voxelAboveRightBehind = volumeSampler.peekVoxel1px1py1pz();

					const Voxel _voxelBelowBehind      = volumeSampler.peekVoxel0px1ny1pz();
					const Voxel _voxelBelowLeftBehind  = volumeSampler.peekVoxel1nx1ny1pz();
					const Voxel _voxelBelowRightBehind = volumeSampler.peekVoxel1px1ny1pz();

					const uint32_t v_0_4 = addVertex(regX,     regY,     regZ, material, m_previousSliceVertices, result,
							_voxelBelowBehind, _voxelLeftBehind, _voxelBelowLeftBehind); //4
					const uint32_t v_1_8 = addVertex(regX,     regY + 1, regZ, material, m_previousSliceVertices, result,
							_voxelAboveBehind, _voxelLeftBehind, _voxelAboveLeftBehind); //8
					const uint32_t v_2_7 = addVertex(regX + 1, regY + 1, regZ, material, m_previousSliceVertices, result,
							_voxelAboveBehind, _voxelRightBehind, _voxelAboveRightBehind); //7
					const uint32_t v_3_3 = addVertex(regX + 1, regY,     regZ, material, m_previousSliceVertices, result,
							_voxelBelowBehind, _voxelRightBehind, _voxelBelowRightBehind); //3
					m_vecQuads[PositiveZ][regZ].push_back(Quad(v_0_4, v_3_3, v_2_7, v_1_8));

					volumeSampler.movePositiveZ();
				}

				volumeSampler.movePositiveX();
			}
		}

		m_previousSliceVertices.swap(m_currentSliceVertices);
		std::memset(m_currentSliceVertices.getRawData(), 0xff, m_currentSliceVertices.getNoOfElements() * sizeof(IndexAndMaterial));
	}

	for (QuadListVector& vecListQuads : m_vecQuads) {
		for (QuadList& listQuads : vecListQuads) {
			if (bMergeQuads) {
				// Repeatedly call this function until it returns
				// false to indicate nothing more can be done.
				while (performQuadMerging(listQuads, result)) {
				}
			}

			for (const Quad& quad : listQuads) {
				const auto v00 = result->getVertex(quad.vertices[3]);
				const auto v01 = result->getVertex(quad.vertices[0]);
				const auto v10 = result->getVertex(quad.vertices[2]);
				const auto v11 = result->getVertex(quad.vertices[1]);

				if (isQuadFlipped(v00, v01, v10, v11)) {
					result->addTriangle(quad.vertices[1], quad.vertices[2], quad.vertices[3]);
					result->addTriangle(quad.vertices[1], quad.vertices[3], quad.vertices[0]);
				} else {
					result->addTriangle(quad.vertices[0], quad.vertices[1], quad.vertices[2]);
					result->addTriangle(quad.vertices[0], quad.vertices[2], quad.vertices[3]);
				}

			}
		}
	}

	result->setOffset(region.getLowerCorner());
	result->removeUnusedVertices();
}

}
