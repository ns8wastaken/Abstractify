#include <raylib.h>
#include <rlgl.h>
#include <cstdint>
#include <limits>
#include <vector>
#include <cstring>

#define SCREEN_MARGIN    100  // By how many pixels the shape values can exceed the screen borders
#define MAX_MUTATION_VAL 16   // Max amount of change per mutation (x ± MAX_MUTATION_OFFSET)

class Abstractify
{
private:
    // A sort of enum but for the shaders
    struct ShapeShader
    {
        Shader Circle;
        Shader Ellipse;
        Shader Square;
        Shader Rectangle;
        Shader Triangle;
        Shader Line;
        Shader Curve;
    };

public:
    Abstractify(const Image& originalImage, const Texture& originalImageTexture, const Color startingBackgroundColor, const Vector2* screenResolution);

    void AddShape(int sampleCount);       // Adds a shape using a hill climbing approach
    void SetShapesFlags(int shapeFlags);  // Use the Shapes::ShapeType enum

    const Texture& GetTexture();

    void UnloadData();

    enum ShapeType : uint8_t
    {
        Circle    = 1,
        Ellipse   = 2,
        Rectangle = 4,
        Triangle  = 8,
        Line      = 16,
        Curve     = 32
    };

    /*
     *  Shape data layouts:
     *
     *  Circle:    [ centerPosX, centerPosY, radius ]
     *  Ellipse:   [ centerPosX, centerPosY, ... ] TODO: Ellipse data
     *  Rectangle: [ topLeftX, topLeftY, width, height ]
     *  Triangle:  [ v1PosX, v1PoxY, v2PoxX, v2PosY, v3PosX, v3PosY ]
     *  Line:      [ startPosX, startPosY, endPosX, endPosY, width ]
     *  Curve:     [ ... ] TODO: Curve data
     */
    struct Shape
    {
        ShapeType type;
        uint64_t score;

        Color color;
        int data[8];      // Usage varies based on ShapeType
        size_t dataSize;  // Amount of data used (3 for circle)

        void mutate()
        {
        // Mutate data
            int d = GetRandomValue(0, static_cast<int>(dataSize - 1));
            int v = GetRandomValue(-MAX_MUTATION_VAL, MAX_MUTATION_VAL);
            data[d] += v;

        // TODO: Mutate color
        }
    };

private:
    uint8_t m_usableShapesFlag = 0;

    const Vector2* m_screenResolution; // TODO: Potentially remove

    const Image& m_originalImage;
    const Texture& m_originalTexture;

    Texture m_blankTexture;
    RenderTexture m_shapeCanvasRenderTexture;
    RenderTexture m_diffRenderTexture;

    RenderTexture m_resultRenderTexture;

    const Shader m_diffShader;
    const ShapeShader m_shapeShader;

    void m_DrawShape(RenderTexture& target, const Shape& triangle);

    Texture m_GetTextureDifference(const Texture& originalTexture, const Texture& otherTexture);

    uint64_t m_ComputeScore(const Texture& otherTexture);

    Shape m_GetRandomShape();  // TODO
    Shape m_HillClimb(Shape startingShape, int lifetime);  // TODO
};
