#include <raylib.h>
#include <rlgl.h>
#include <cstdint>
#include <limits>
#include <vector>
#include <math.h>

#include "utils/comp_funcs.hpp"
#include "compute_shader.hpp"
#include "rng.hpp"
#include "shape.hpp"


class Abstractify
{
public:
    Abstractify(const Image& originalImage, const Texture& originalImageTexture);

    void AddShape(int sampleCount);       // Adds a shape using a hill climbing approach
    void SetShapesFlags(int shapeFlags);  // Use the Shapes::ShapeType enum for the flags

    const Texture& GetTexture();

    void UnloadData();

    enum ShapeFlag
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
