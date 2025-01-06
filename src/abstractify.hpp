#include <raylib.h>
#include <rlgl.h>
#include <cstdint>
#include <limits>
#include <vector>
#include <math.h>

#include "utils/comp_funcs.hpp"
#include "compute_shader.hpp"
#include "rng.hpp"


class Abstractify
{
public:
    Abstractify(const Image& originalImage, const Texture& originalImageTexture);

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
        static constexpr inline int MAX_MUTATION_VAL = 16;  // Max amount of change per mutation (x ± MAX_MUTATION_OFFSET)
        static constexpr inline uint8_t SHAPE_ALPHA  = 128;

        // Circle
        static constexpr inline int MIN_CIRCLE_RADIUS = 1;
        static constexpr inline int MAX_CIRCLE_RADIUS = 1000;

        // Square
        static constexpr inline int MIN_SQUARE_SIZE = 10;
        static constexpr inline int MAX_SQUARE_SIZE = 150;
    } Settings;

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
        uint32_t score;

        Color color;
        int data[8];      // Usage varies based on shape type
        size_t dataSize;  // Amount of data used (3 for circle)

        void mutate(const Vector2& imageSize, RNG& rng);
    };

    RNG m_RNG;

    uint8_t m_usableShapesFlag = 0;

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

    ComputeShader m_squaredErrorComputeShader;
    ComputeShader m_colorComputeShader;
    ComputeShader m_avgColorComputeShader;

    void m_DrawShape(RenderTexture& target, const Shape& triangle);  // TODO
    float m_ComputeScore(const Shape& shape);
    Color m_ComputeColor(const Shape& shape);
    Shape m_GetRandomShape();  // TODO

    Color m_ComputeAverageColor(const Texture& texture);
};
