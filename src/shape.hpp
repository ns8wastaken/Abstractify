#pragma once
#include <raylib.h>
#include <cstdint>

#include "rng.hpp"
#include "utils/comp_funcs.hpp"

// General
#define MAX_MUTATION_VAL 16  // Max amount of change per mutation (x ± MAX_MUTATION_OFFSET)
#define SHAPE_ALPHA      128

// Circle
#define MIN_CIRCLE_RADIUS 1
#define MAX_CIRCLE_RADIUS 1000

// Square
#define MIN_SQUARE_SIZE 10
#define MAX_SQUARE_SIZE 150


enum class ShapeType
{
    Circle    = 1,
    Ellipse   = 2,
    Square    = 3,
    Rectangle = 4,
    Triangle  = 5,
    Line      = 6,
    Curve     = 7
};


/*
 *  Shape data layouts
 *  @param Circle    [ centerPosX, centerPosY, radius ]
 *  @param Ellipse   [ centerPosX, centerPosY, ... ] TODO: Ellipse data
 *  @param Square    [ topLeftX, topLeftY, size ]
 *  @param Rectangle [ topLeftX, topLeftY, width, height ]
 *  @param Triangle  [ v1PosX, v1PoxY, v2PoxX, v2PosY, v3PosX, v3PosY ]
 *  @param Line      [ startPosX, startPosY, endPosX, endPosY, width ]
 *  @param Curve     [ ... ] TODO: Curve data
 */
struct Shape
{
    ShapeType type;
    uint32_t score = 0;

    Color color     = { 0, 0, 0, SHAPE_ALPHA };
    int data[8]     = { 0 };  // Usage varies based on shape type
    size_t dataSize = 0;      // Amount of data used (3 for circle)

    void mutate(const Vector2& imageSize, RNG& rng);
    void randomize(const Vector2& imageSize, RNG& rng, const std::vector<ShapeType>& usableShapeTypes);
};
