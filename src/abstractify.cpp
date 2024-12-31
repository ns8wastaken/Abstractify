#include "abstractify.hpp"


Abstractify::Abstractify(
    const Image& originalImage,
    const Texture& originalImageTexture,
    const Color startingBackgroundColor,
    const Vector2* screenResolution
)
    : m_originalImage(originalImage),
      m_originalTexture(originalImageTexture),
      m_diffShader(LoadShader(0, "src/shaders/diff_calc.frag")),
      m_shapeShader(ShapeShader{
          .Circle    = LoadShader(0, "src/shaders/circle.frag"),
          .Ellipse   = LoadShader(0, "src/shaders/ellipse.frag"),
          .Square    = LoadShader(0, "src/shaders/square.frag"),
          .Rectangle = LoadShader(0, "src/shaders/rectangle.frag"),
          .Triangle  = LoadShader(0, "src/shaders/triangle.frag"),
          .Line      = LoadShader(0, "src/shaders/line.frag"),
          .Curve     = LoadShader(0, "src/shaders/curve.frag") }
      )
{
    // Generate blank texture
    Image img      = GenImageColor(originalImage.width, originalImage.height, BLANK);
    m_blankTexture = LoadTextureFromImage(img);
    UnloadImage(img);

    // Canvases used to draw temp texture onto
    m_shapeCanvasRenderTexture = LoadRenderTexture(originalImage.width, originalImage.height);
    m_diffRenderTexture        = LoadRenderTexture(originalImage.width, originalImage.height);

    m_resultRenderTexture = LoadRenderTexture(originalImage.width, originalImage.height);
    BeginTextureMode(m_resultRenderTexture);
    ClearBackground(startingBackgroundColor);
    EndTextureMode();

    m_screenResolution = screenResolution;
}


void Abstractify::UnloadData()
{
    // Shaders
    UnloadShader(m_diffShader);
    UnloadShader(m_shapeShader.Circle);
    UnloadShader(m_shapeShader.Ellipse);
    UnloadShader(m_shapeShader.Square);
    UnloadShader(m_shapeShader.Rectangle);
    UnloadShader(m_shapeShader.Triangle);
    UnloadShader(m_shapeShader.Line);
    UnloadShader(m_shapeShader.Curve);

    // Results
    UnloadRenderTexture(m_resultRenderTexture);

    // Other
    UnloadTexture(m_blankTexture);
    UnloadRenderTexture(m_shapeCanvasRenderTexture);
    UnloadRenderTexture(m_diffRenderTexture);
}


void Abstractify::SetShapesFlags(int shapesFlag)
{
    m_usableShapesFlag = shapesFlag;
}


const Texture& Abstractify::GetTexture()
{
    return m_resultRenderTexture.texture;
}


void Abstractify::AddShape(int sampleCount)
{
    Shape bestShape;
    bestShape.score = std::numeric_limits<uint64_t>::max();

    for (int j = 0; j < sampleCount; ++j) {
        // TODO: Generate random shape
        Vector2 position = {
            (float)GetRandomValue(0, m_originalImage.width),
            (float)GetRandomValue(0, m_originalImage.height)
        };

        int radius = GetRandomValue(5, 200);

        Color color = {
            (unsigned char)GetRandomValue(0, 255),
            (unsigned char)GetRandomValue(0, 255),
            (unsigned char)GetRandomValue(0, 255),
            255
        };

        // Shape shape = m_GetRandomShape();

        // Clear m_shapeCanvasRenderTexture background and draw shape on it
        BeginTextureMode(m_shapeCanvasRenderTexture);
        ClearBackground(BLANK);
        DrawTexture(m_resultRenderTexture.texture, 0, 0, BLANK);
        // DrawCircleV(Vector2{ static_cast<float>(shape.data[0]), static_cast<float>(shape.data[1]) }, shape.data[2], shape.color);
        DrawCircleV(position, radius, color);
        EndTextureMode();
        // m_DrawShape(m_resultRenderTexture, shape);

        uint64_t score = m_ComputeScore(m_shapeCanvasRenderTexture.texture);
        // uint64_t score = 0;
        // printf("Score: %d\n", score);

        // if (score < bestShape.score) {
        //     bestShape.type  = shape.type;
        //     bestShape.score = score;
        //     bestShape.color = shape.color;
        //     std::memcpy(bestShape.data, shape.data, sizeof(shape.data));
        // }
        if (score < bestShape.score) {
            bestShape.type    = ShapeType::Circle;
            bestShape.score   = score;
            bestShape.color   = color;
            bestShape.data[0] = position.x;
            bestShape.data[1] = position.y;
            bestShape.data[2] = radius;
        }
    }

    m_DrawShape(m_resultRenderTexture, bestShape);
}


uint64_t Abstractify::m_ComputeScore(const Texture& otherTexture)
{
    // Clear m_diffRenderTexture background
    BeginTextureMode(m_diffRenderTexture);
    ClearBackground(BLANK);
    EndTextureMode();

    // Draw the difference of colors to m_diffRenderTexture
    BeginTextureMode(m_diffRenderTexture);
    {
        ClearBackground(BLANK);

        BeginShaderMode(m_diffShader);
        {
            SetShaderValueTexture(m_diffShader, GetShaderLocation(m_diffShader, "tex0"), m_originalTexture);
            SetShaderValueTexture(m_diffShader, GetShaderLocation(m_diffShader, "tex1"), otherTexture);

            DrawTexture(m_blankTexture, 0, 0, BLANK);
        }
        EndShaderMode();
    }
    EndTextureMode();

    // Get diff image + pixel colors
    Image diffImage   = LoadImageFromTexture(m_diffRenderTexture.texture);
    Color* diffPixels = LoadImageColors(diffImage);

    // Calculate score
    uint64_t score = 0;
    for (int i = 0; i < diffImage.width * diffImage.height; i++) {
        score += diffPixels[i].r;
    }

    UnloadImageColors(diffPixels);
    UnloadImage(diffImage);

    return score;
}

// TODO
void Abstractify::m_DrawShape(RenderTexture& target, const Shape& shape)
{
    switch (shape.type) {
        case ShapeType::Circle: {
            Vector2 pos = { static_cast<float>(shape.data[0]), static_cast<float>(shape.data[1]) };
            SetShaderValue(m_shapeShader.Circle, GetShaderLocation(m_shapeShader.Circle, "position"), &pos, SHADER_UNIFORM_VEC2);
            SetShaderValue(m_shapeShader.Circle, GetShaderLocation(m_shapeShader.Circle, "radius"), &shape.data[2], SHADER_UNIFORM_INT);

            BeginTextureMode(target);
            {
                BeginShaderMode(m_shapeShader.Circle);
                DrawTexture(m_blankTexture, 0, 0, shape.color);
                EndShaderMode();
            }
            EndTextureMode();
        } break;

        case ShapeType::Ellipse: {
        } break;

        case ShapeType::Rectangle: {
            Vector2 pos  = { static_cast<float>(shape.data[0]), static_cast<float>(shape.data[1]) };
            Vector2 size = { static_cast<float>(shape.data[2]), static_cast<float>(shape.data[3]) };

            SetShaderValue(m_shapeShader.Rectangle, GetShaderLocation(m_shapeShader.Rectangle, "position"), &pos, SHADER_UNIFORM_VEC2);
            SetShaderValue(m_shapeShader.Rectangle, GetShaderLocation(m_shapeShader.Rectangle, "size"), &size, SHADER_UNIFORM_VEC2);

            BeginTextureMode(m_resultRenderTexture);
            {
                BeginShaderMode(m_shapeShader.Rectangle);
                DrawTexture(m_blankTexture, 0, 0, shape.color);
                EndShaderMode();
            }
            EndTextureMode();
        } break;

        case ShapeType::Triangle: {
            Vector2 v1 = { static_cast<float>(shape.data[0]), static_cast<float>(shape.data[1]) };
            Vector2 v2 = { static_cast<float>(shape.data[2]), static_cast<float>(shape.data[3]) };
            Vector2 v3 = { static_cast<float>(shape.data[4]), static_cast<float>(shape.data[5]) };

            SetShaderValue(m_shapeShader.Triangle, GetShaderLocation(m_shapeShader.Triangle, "v1"), &v1, SHADER_UNIFORM_VEC2);
            SetShaderValue(m_shapeShader.Triangle, GetShaderLocation(m_shapeShader.Triangle, "v2"), &v2, SHADER_UNIFORM_VEC2);
            SetShaderValue(m_shapeShader.Triangle, GetShaderLocation(m_shapeShader.Triangle, "v3"), &v3, SHADER_UNIFORM_VEC2);

            BeginTextureMode(target);
            {
                BeginShaderMode(m_shapeShader.Triangle);
                DrawTexture(m_blankTexture, 0, 0, shape.color);
                EndShaderMode();
            }
            EndTextureMode();
        } break;

        case ShapeType::Line: {
        } break;

        case ShapeType::Curve: {
        } break;
    }
}


Texture Abstractify::m_GetTextureDifference(const Texture& originalTexture, const Texture& otherTexture)
{
    RenderTexture diffTexture = LoadRenderTexture(originalTexture.width, originalTexture.height);

    BeginTextureMode(diffTexture);
    {
        BeginShaderMode(m_diffShader);

        SetShaderValueTexture(m_diffShader, GetShaderLocation(m_diffShader, "tex0"), originalTexture);
        SetShaderValueTexture(m_diffShader, GetShaderLocation(m_diffShader, "tex1"), otherTexture);

        DrawTexture(originalTexture, 0, 0, BLANK);
        EndShaderMode();
    }
    EndTextureMode();

    UnloadShader(m_diffShader);

    return diffTexture.texture;
}


Abstractify::Shape Abstractify::m_GetRandomShape()
{
    std::vector<ShapeType> usableShapeTypes = {};

    for (int i = 0; i < 8; ++i) {
        if (m_usableShapesFlag & (1 << i)) {
            usableShapeTypes.push_back(static_cast<ShapeType>(1 << i));
        }
    }

    Shape shape = {};

    switch (usableShapeTypes[GetRandomValue(0, usableShapeTypes.size() - 1)]) {
        case ShapeType::Circle: {
            shape.type = ShapeType::Circle;

            shape.data[0] = GetRandomValue(-SCREEN_MARGIN, m_screenResolution->x + SCREEN_MARGIN);
            shape.data[1] = GetRandomValue(-SCREEN_MARGIN, m_screenResolution->y + SCREEN_MARGIN);
            shape.data[2] = GetRandomValue(10, std::min(m_screenResolution->x, m_screenResolution->y) / 2);

            shape.dataSize = 3;

            shape.color = Color{
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255))
            };
        } break;

        case ShapeType::Ellipse: {
        } break;

        case ShapeType::Rectangle: {
            shape.type = ShapeType::Rectangle;

            shape.data[0] = GetRandomValue(-SCREEN_MARGIN, m_screenResolution->x + SCREEN_MARGIN);
            shape.data[1] = GetRandomValue(-SCREEN_MARGIN, m_screenResolution->y + SCREEN_MARGIN);
            shape.data[2] = GetRandomValue(10, std::min(m_screenResolution->x, m_screenResolution->y) / 2);
            shape.data[3] = GetRandomValue(10, std::min(m_screenResolution->x, m_screenResolution->y) / 2);

            shape.dataSize = 4;

            shape.color = Color{
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255))
            };
        } break;

        case ShapeType::Triangle: {
            shape.type = ShapeType::Triangle;

            shape.data[0] = GetRandomValue(-SCREEN_MARGIN, m_screenResolution->x + SCREEN_MARGIN);
            shape.data[1] = GetRandomValue(-SCREEN_MARGIN, m_screenResolution->y + SCREEN_MARGIN);

            shape.data[2] = GetRandomValue(-SCREEN_MARGIN, m_screenResolution->x + SCREEN_MARGIN);
            shape.data[3] = GetRandomValue(-SCREEN_MARGIN, m_screenResolution->y + SCREEN_MARGIN);

            shape.data[4] = GetRandomValue(-SCREEN_MARGIN, m_screenResolution->x + SCREEN_MARGIN);
            shape.data[5] = GetRandomValue(-SCREEN_MARGIN, m_screenResolution->y + SCREEN_MARGIN);

            shape.dataSize = 6;

            shape.color = Color{
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255))
            };
        } break;

        case ShapeType::Line: {
        } break;

        case ShapeType::Curve: {
        } break;
    }

    return shape;
}
