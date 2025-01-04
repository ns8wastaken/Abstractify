#include <raylib.h>
#include <rlgl.h>
#include <cstdint>
#include <limits>
#include <vector>

#include "utils/comp_funcs.hpp"
#include "compute_shader.hpp"


// #define MAX_MUTATION_VAL  16      // Max amount of change per mutation (x ± MAX_MUTATION_OFFSET)
// #define MIN_CIRCLE_RADIUS 15
// #define MAX_CIRCLE_RADIUS 150


class Abstractify
{
public:
    Abstractify(const Image& originalImage, const Texture& originalImageTexture, const Color startingBackgroundColor);

    void AddShape(int sampleCount);       // Adds a shape using a hill climbing approach
    void SetShapesFlags(int shapeFlags);  // Use the Shapes::ShapeType enum for the flags

    const Texture& GetTexture();

    void UnloadData();

    enum ShapeType
    {
        Circle    = 1,
        Ellipse   = 2,
        Square    = 4,
        Rectangle = 8,
        Triangle  = 16,
        Line      = 32,
        Curve     = 64
    };

private:
    typedef struct Settings
    {
        // General
        static constexpr inline int MAX_MUTATION_VAL = 16;

        // Circle
        static constexpr inline int MIN_CIRCLE_RADIUS = 10;
        static constexpr inline int MAX_CIRCLE_RADIUS = 150;

        // Square
        static constexpr inline int MIN_SQUARE_SIZE = 10;
        static constexpr inline int MAX_SQUARE_SIZE = 150;
    } Settings;

    /*
     *  Shape data layouts:
     *
     *  Circle:    [ centerPosX, centerPosY, radius ]
     *  Ellipse:   [ centerPosX, centerPosY, ... ] TODO: Ellipse data
     *  Square:    [ topLeftX, topLeftY, size ]
     *  Rectangle: [ topLeftX, topLeftY, width, height ]
     *  Triangle:  [ v1PosX, v1PoxY, v2PoxX, v2PosY, v3PosX, v3PosY ]
     *  Line:      [ startPosX, startPosY, endPosX, endPosY, width ]
     *  Curve:     [ ... ] TODO: Curve data
     */
    struct Shape
    {
        ShapeType type;
        uint32_t score;

        Color color;
        int data[8];      // Usage varies based on shape type
        size_t dataSize;  // Amount of data used (3 for circle)

        void mutate(const Vector2& imageSize);
    };

    uint8_t m_usableShapesFlag = 0;
    uint32_t m_bestScore       = std::numeric_limits<uint32_t>::max();

    const Image& m_originalImage;
    const Texture& m_originalTexture;
    const Vector2 m_imageSize;

    Texture m_blankTexture;
    RenderTexture m_shapeCanvasRenderTexture;
    RenderTexture m_diffRenderTexture;
    RenderTexture m_resultRenderTexture;

    struct ShapeShader
    {
        Shader Circle, Ellipse, Square, Rectangle, Triangle, Line, Curve;
    } const m_shapeShader;
    ComputeShader m_diffComputeShader;
    ComputeShader m_colorComputeShader;

    void m_DrawShape(RenderTexture& target, const Shape& triangle);  // TODO
    uint32_t m_ComputeScore(const Texture& otherTexture);
    Color m_ComputeColor(const Shape& shape);
    Shape m_GetRandomShape();  // TODO
};
