#include "PixelGridLayoutTests.h"
#include "unit_test.h"

#include "Light/RGBPixelGridComponent.h"

#include <assert.h>
#include <memory>
#include <math.h>
#include <set>
#include <stdio.h>
#include <vector>

namespace
{
const eDMXPixelGridOrigin k_allOrigins[]= {
	eDMXPixelGridOrigin::upperLeft,
	eDMXPixelGridOrigin::upperRight,
	eDMXPixelGridOrigin::lowerLeft,
	eDMXPixelGridOrigin::lowerRight,
};

// CommonConfig is non-copyable, so the definitions are built on the heap
RGBPixelGridDefinitionPtr makeGrid(int columns, int rows, eDMXPixelGridOrigin origin, bool bZigZag)
{
	auto definition= std::make_shared<RGBPixelGridDefinition>();
	definition->resizeGrid(columns, rows);
	definition->setOriginPixel(origin);
	definition->setZigZag(bZigZag);
	return definition;
}
} // namespace

bool run_pixel_grid_layout_tests()
{
	UNIT_TEST_MODULE_BEGIN("pixel_grid_layout")
	UNIT_TEST_MODULE_CALL_TEST(pixel_grid_test_mapping_is_a_bijection);
	UNIT_TEST_MODULE_CALL_TEST(pixel_grid_test_origin_and_zigzag_order);
	UNIT_TEST_MODULE_CALL_TEST(pixel_grid_test_local_geometry);
	UNIT_TEST_MODULE_END()
}

bool pixel_grid_test_mapping_is_a_bijection()
{
	UNIT_TEST_BEGIN("every origin covers each cell exactly once and round-trips")

	const int columns= 5;
	const int rows= 3;

	for (const eDMXPixelGridOrigin origin : k_allOrigins)
	{
		for (int zigZagPass= 0; zigZagPass < 2; ++zigZagPass)
		{
			RGBPixelGridDefinitionPtr definition= makeGrid(columns, rows, origin, zigZagPass == 1);

			std::set<int> seenWireIndices;
			for (int row= 0; row < rows; ++row)
			{
				for (int col= 0; col < columns; ++col)
				{
					const int wireIndex= definition->getPixelWireIndex(col, row);

					// In range, unclaimed, and it maps back to the cell it came from
					success&= (wireIndex >= 0 && wireIndex < columns * rows);
					assert(success);
					success&= seenWireIndices.insert(wireIndex).second;
					assert(success);

					int backCol= -1, backRow= -1;
					success&= definition->getPixelGridPosition(wireIndex, backCol, backRow);
					assert(success);
					success&= (backCol == col && backRow == row);
					assert(success);
				}
			}

			success&= ((int)seenWireIndices.size() == columns * rows);
			assert(success);
		}
	}

	// Out of range in either axis is refused both ways
	RGBPixelGridDefinitionPtr definition= makeGrid(columns, rows, eDMXPixelGridOrigin::upperLeft, false);
	success&= (definition->getPixelWireIndex(-1, 0) == -1);
	assert(success);
	success&= (definition->getPixelWireIndex(columns, 0) == -1);
	assert(success);
	success&= (definition->getPixelWireIndex(0, rows) == -1);
	assert(success);
	int outCol= 0, outRow= 0;
	success&= !definition->getPixelGridPosition(-1, outCol, outRow);
	assert(success);
	success&= !definition->getPixelGridPosition(columns * rows, outCol, outRow);
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool pixel_grid_test_origin_and_zigzag_order()
{
	UNIT_TEST_BEGIN("origin picks the corner and zig-zag reverses odd rows")

	const int columns= 4;
	const int rows= 3;

	// Upper left with no zig-zag is the mapping projects used before the
	// layout properties existed, so old wiring must not shift
	RGBPixelGridDefinitionPtr upperLeft= makeGrid(columns, rows, eDMXPixelGridOrigin::upperLeft, false);
	for (int row= 0; row < rows; ++row)
	{
		for (int col= 0; col < columns; ++col)
		{
			success&= (upperLeft->getPixelWireIndex(col, row) == row * columns + col);
			assert(success);
		}
	}

	// Each origin puts its own corner first
	success&= (upperLeft->getPixelWireIndex(0, 0) == 0);
	assert(success);
	RGBPixelGridDefinitionPtr upperRight= makeGrid(columns, rows, eDMXPixelGridOrigin::upperRight, false);
	success&= (upperRight->getPixelWireIndex(columns - 1, 0) == 0);
	assert(success);
	RGBPixelGridDefinitionPtr lowerLeft= makeGrid(columns, rows, eDMXPixelGridOrigin::lowerLeft, false);
	success&= (lowerLeft->getPixelWireIndex(0, rows - 1) == 0);
	assert(success);
	RGBPixelGridDefinitionPtr lowerRight= makeGrid(columns, rows, eDMXPixelGridOrigin::lowerRight, false);
	success&= (lowerRight->getPixelWireIndex(columns - 1, rows - 1) == 0);
	assert(success);

	// Zig-zag leaves the even scan rows alone and reverses the odd ones
	RGBPixelGridDefinitionPtr zigZag= makeGrid(columns, rows, eDMXPixelGridOrigin::upperLeft, true);
	success&= (zigZag->getPixelWireIndex(0, 0) == 0);
	assert(success);
	success&= (zigZag->getPixelWireIndex(columns - 1, 0) == columns - 1);
	assert(success);
	// Row 1 runs backwards: its rightmost pixel is the first of that row
	success&= (zigZag->getPixelWireIndex(columns - 1, 1) == columns);
	assert(success);
	success&= (zigZag->getPixelWireIndex(0, 1) == 2 * columns - 1);
	assert(success);
	// Row 2 is even again
	success&= (zigZag->getPixelWireIndex(0, 2) == 2 * columns);
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool pixel_grid_test_local_geometry()
{
	UNIT_TEST_BEGIN("pixel centres are centered on the origin and match the half extents")

	const int columns= 4;
	const int rows= 2;
	RGBPixelGridDefinitionPtr definition= makeGrid(columns, rows, eDMXPixelGridOrigin::upperLeft, false);
	definition->setPixelSizeMM(MikanVector3f(20.f, 20.f, 10.f));
	definition->setPixelSeparationMM(MikanVector2f(50.f, 40.f));

	// Columns run along +X and rows along -Y
	const glm::vec3 first= definition->getPixelLocalCenter(0, 0);
	const glm::vec3 lastCol= definition->getPixelLocalCenter(columns - 1, 0);
	const glm::vec3 lastRow= definition->getPixelLocalCenter(0, rows - 1);
	success= (lastCol.x > first.x);
	assert(success);
	success&= (lastRow.y < first.y);
	assert(success);

	// The whole span is centered on the component origin
	success&= fabsf(first.x + lastCol.x) < 1e-5f;
	assert(success);
	success&= fabsf(first.y + lastRow.y) < 1e-5f;
	assert(success);
	success&= fabsf(first.z) < 1e-5f;
	assert(success);

	// Separation is centre to centre, in metres
	success&= fabsf((lastCol.x - first.x) - (float)(columns - 1) * 0.050f) < 1e-5f;
	assert(success);
	success&= fabsf((first.y - lastRow.y) - (float)(rows - 1) * 0.040f) < 1e-5f;
	assert(success);

	// The half extents reach the outer pixels' far edges
	const glm::vec3 halfExtents= definition->getGridLocalHalfExtents();
	success&= fabsf(halfExtents.x - (lastCol.x + 0.010f)) < 1e-5f;
	assert(success);
	success&= fabsf(halfExtents.y - (first.y + 0.010f)) < 1e-5f;
	assert(success);
	success&= fabsf(halfExtents.z - 0.005f) < 1e-5f;
	assert(success);

	// A single pixel with no separation still has the pixel's own size
	RGBPixelGridDefinitionPtr single= makeGrid(1, 1, eDMXPixelGridOrigin::upperLeft, false);
	single->setPixelSizeMM(MikanVector3f(20.f, 20.f, 10.f));
	single->setPixelSeparationMM(MikanVector2f(0.f, 0.f));
	const glm::vec3 singleExtents= single->getGridLocalHalfExtents();
	success&= fabsf(singleExtents.x - 0.010f) < 1e-5f && fabsf(singleExtents.y - 0.010f) < 1e-5f;
	assert(success);

	UNIT_TEST_COMPLETE()
}
