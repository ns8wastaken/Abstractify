#include "abstractify.hpp"


void Abstractify::Shape::mutate(const Vector2& imageSize)
{
    switch (type) {
        case 1: { // Circle
            switch (GetRandomValue(0, 1)) {
                case 0: {
                    data[0] = Clamp(data[0] + GetRandomValue(-Settings::MAX_MUTATION_VAL, Settings::MAX_MUTATION_VAL), 0, static_cast<int>(imageSize.x) - 1);
                    data[1] = Clamp(data[1] + GetRandomValue(-Settings::MAX_MUTATION_VAL, Settings::MAX_MUTATION_VAL), 0, static_cast<int>(imageSize.y) - 1);
                } break;

                case 1: {
                    data[2] = Clamp(data[2] + GetRandomValue(-Settings::MAX_MUTATION_VAL, Settings::MAX_MUTATION_VAL), Settings::MIN_CIRCLE_RADIUS, Settings::MAX_CIRCLE_RADIUS);
                } break;
            }
        } break;

        case 2: { // Ellipse
        } break;

        case 4: { // Square
        } break;

        case 8: { // Rectangle
        } break;

        case 16: { // Triangle
        } break;

        case 32: { // Line
        } break;

        case 64: { // Curve
        } break;
    }
}


Abstractify::Abstractify(const Image& originalImage, const Texture& originalImageTexture, const Color startingBackgroundColor)
    : m_originalImage(originalImage),
      m_originalTexture(originalImageTexture),
      m_imageSize(Vector2{ static_cast<float>(m_originalImage.width), static_cast<float>(m_originalImage.height) }),
      m_shapeShader(ShapeShader{
          .Circle    = LoadShader(0, "src/shaders/circle.frag"),
          .Ellipse   = LoadShader(0, "src/shaders/ellipse.frag"),
          .Square    = LoadShader(0, "src/shaders/square.frag"),
          .Rectangle = LoadShader(0, "src/shaders/rectangle.frag"),
          .Triangle  = LoadShader(0, "src/shaders/triangle.frag"),
          .Line      = LoadShader(0, "src/shaders/line.frag"),
          .Curve     = LoadShader(0, "src/shaders/curve.frag") }
      ),
      m_diffComputeShader("src/shaders/diff_calc.comp"),
      m_colorComputeShader("src/shaders/color_calc.comp")
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

    unsigned int result = 0;
    m_diffComputeShader.AddSSBO(&result, sizeof(result));

    unsigned int rgbaSum[5];
    int shapeData[8];
    m_colorComputeShader.AddSSBO(rgbaSum, sizeof(rgbaSum));
    m_colorComputeShader.AddSSBO(shapeData, sizeof(shapeData));
}


void Abstractify::UnloadData()
{
    // Shaders
    UnloadShader(m_shapeShader.Circle);
    UnloadShader(m_shapeShader.Ellipse);
    UnloadShader(m_shapeShader.Square);
    UnloadShader(m_shapeShader.Rectangle);
    UnloadShader(m_shapeShader.Triangle);
    UnloadShader(m_shapeShader.Line);
    UnloadShader(m_shapeShader.Curve);

    // Compute shader
    m_diffComputeShader.Unload();
    m_colorComputeShader.Unload();

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
    // return m_shapeCanvasRenderTexture.texture;
}


void Abstractify::AddShape(int lifetime)
{
    Shape bestShape    = m_GetRandomShape();
    uint32_t bestScore = std::numeric_limits<uint32_t>::max();

    int age = 0;
    while (age < lifetime) {
        Shape tempShape = bestShape;

        tempShape.mutate(m_imageSize);

        tempShape.color   = m_ComputeColor(tempShape);
        tempShape.color.a = 180;

        BeginTextureMode(m_shapeCanvasRenderTexture);
        ClearBackground(BLANK);
        DrawTexture(m_resultRenderTexture.texture, 0, 0, WHITE);
        EndTextureMode();

        m_DrawShape(m_shapeCanvasRenderTexture, tempShape);

        const uint32_t score = m_ComputeScore(m_shapeCanvasRenderTexture.texture);

        if (score < bestScore) {
            bestShape = tempShape;
            bestScore = score;
            age       = -1;
        }

        ++age;
    }

    m_DrawShape(m_resultRenderTexture, bestShape);
}


uint32_t Abstractify::m_ComputeScore(const Texture& otherTexture)
{
    uint32_t result = 0;

    rlEnableShader(m_diffComputeShader.program);
    {
        m_diffComputeShader.UpdateSSBO(0, &result, sizeof(result));  // Reset SSBO

        int size[2] = { m_originalTexture.width, m_originalTexture.height };
        rlSetUniform(rlGetLocationUniform(m_diffComputeShader.program, "resolution"), size, SHADER_UNIFORM_IVEC2, 1);

        // Bind textures
        rlBindImageTexture(m_originalTexture.id, 0, m_originalTexture.format, true);
        rlBindImageTexture(otherTexture.id, 1, otherTexture.format, true);

        // Bind SSBO
        rlBindShaderBuffer(m_diffComputeShader.ssbos[0], 2);

        // Dispatch compute shader
        rlComputeShaderDispatch((m_originalTexture.width + 15) / 16, (m_originalTexture.height + 15) / 16, 1);
    }
    rlDisableShader();

    // Read result from SSBO
    rlReadShaderBuffer(m_diffComputeShader.ssbos[0], &result, sizeof(result), 0);

    return result;
}


Color Abstractify::m_ComputeColor(const Shape& shape)
{
    unsigned int data[5] = { 0, 0, 0, 0, 0 }; // { r, g, b, a, pixelCount }

    rlEnableShader(m_colorComputeShader.program);
    {
        // Reset / set SSBOs
        m_colorComputeShader.UpdateSSBO(0, data, sizeof(data));
        m_colorComputeShader.UpdateSSBO(1, shape.data, sizeof(shape.data));

        // Set uniforms
        int size[2] = { m_originalTexture.width, m_originalTexture.height };
        rlSetUniform(rlGetLocationUniform(m_colorComputeShader.program, "shapeType"), &shape.type, SHADER_UNIFORM_INT, 1);
        rlSetUniform(rlGetLocationUniform(m_colorComputeShader.program, "resolution"), size, SHADER_UNIFORM_IVEC2, 1);

        // Bind m_originalTexture
        rlBindImageTexture(m_originalTexture.id, 0, m_originalTexture.format, true);

        // Bind SSBOs
        rlBindShaderBuffer(m_colorComputeShader.ssbos[0], 1);
        rlBindShaderBuffer(m_colorComputeShader.ssbos[1], 2);

        // Dispatch compute shader
        rlComputeShaderDispatch((m_originalTexture.width + 15) / 16, (m_originalTexture.height + 15) / 16, 1);
    }
    rlDisableShader();

    // Read result from SSBO
    rlReadShaderBuffer(m_colorComputeShader.ssbos[0], data, sizeof(data), 0);

    return Color{
        static_cast<uint8_t>(data[0] / data[4]),
        static_cast<uint8_t>(data[1] / data[4]),
        static_cast<uint8_t>(data[2] / data[4]),
        static_cast<uint8_t>(data[3] / data[4])
    };
}


void Abstractify::m_DrawShape(RenderTexture& target, const Shape& shape)
{
    BeginTextureMode(target);

    switch (shape.type) {
        case ShapeType::Circle: {
            Vector2 pos = { static_cast<float>(shape.data[0]), static_cast<float>(shape.data[1]) };
            SetShaderValue(m_shapeShader.Circle, GetShaderLocation(m_shapeShader.Circle, "position"), &pos, SHADER_UNIFORM_VEC2);
            SetShaderValue(m_shapeShader.Circle, GetShaderLocation(m_shapeShader.Circle, "radius"), &shape.data[2], SHADER_UNIFORM_INT);

            {
                BeginShaderMode(m_shapeShader.Circle);
                DrawTexture(m_blankTexture, 0, 0, shape.color);
                EndShaderMode();
            }
        } break;

        case ShapeType::Ellipse: {
        } break;

        case ShapeType::Square: {
        } break;

        case ShapeType::Rectangle: {
            Vector2 pos  = { static_cast<float>(shape.data[0]), static_cast<float>(shape.data[1]) };
            Vector2 size = { static_cast<float>(shape.data[2]), static_cast<float>(shape.data[3]) };

            SetShaderValue(m_shapeShader.Rectangle, GetShaderLocation(m_shapeShader.Rectangle, "position"), &pos, SHADER_UNIFORM_VEC2);
            SetShaderValue(m_shapeShader.Rectangle, GetShaderLocation(m_shapeShader.Rectangle, "size"), &size, SHADER_UNIFORM_VEC2);

            {
                BeginShaderMode(m_shapeShader.Rectangle);
                DrawTexture(m_blankTexture, 0, 0, shape.color);
                EndShaderMode();
            }
        } break;

        case ShapeType::Triangle: {
            Vector2 v1 = { static_cast<float>(shape.data[0]), static_cast<float>(shape.data[1]) };
            Vector2 v2 = { static_cast<float>(shape.data[2]), static_cast<float>(shape.data[3]) };
            Vector2 v3 = { static_cast<float>(shape.data[4]), static_cast<float>(shape.data[5]) };

            SetShaderValue(m_shapeShader.Triangle, GetShaderLocation(m_shapeShader.Triangle, "v1"), &v1, SHADER_UNIFORM_VEC2);
            SetShaderValue(m_shapeShader.Triangle, GetShaderLocation(m_shapeShader.Triangle, "v2"), &v2, SHADER_UNIFORM_VEC2);
            SetShaderValue(m_shapeShader.Triangle, GetShaderLocation(m_shapeShader.Triangle, "v3"), &v3, SHADER_UNIFORM_VEC2);

            {
                BeginShaderMode(m_shapeShader.Triangle);
                DrawTexture(m_blankTexture, 0, 0, shape.color);
                EndShaderMode();
            }
        } break;

        case ShapeType::Line: {
        } break;

        case ShapeType::Curve: {
        } break;
    }

    EndTextureMode();
}


Abstractify::Shape Abstractify::m_GetRandomShape()
{
    // TODO: Get usableShapeTypes once instead of every frame
    std::vector<ShapeType> usableShapeTypes = {};

    for (int i = 0; i < 8; ++i) {
        if (m_usableShapesFlag & (1 << i)) {
            usableShapeTypes.push_back(static_cast<ShapeType>(1 << i));
        }
    }

    // printf("Usable shape types:\n");
    // for (ShapeType i : usableShapeTypes)
    //     printf("%d\n", static_cast<int>(i));
    // printf("\n");

    Shape shape = {};

    switch (usableShapeTypes[GetRandomValue(0, usableShapeTypes.size() - 1)]) {
        case ShapeType::Circle: {
            shape.type    = ShapeType::Circle;
            shape.data[0] = GetRandomValue(0, m_originalImage.width);
            shape.data[1] = GetRandomValue(0, m_originalImage.height);
            // shape.data[2] = GetRandomValue(10, std::min(m_originalImage.width, m_originalImage.height) / 2);
            shape.data[2] = GetRandomValue(Settings::MIN_CIRCLE_RADIUS, Settings::MAX_CIRCLE_RADIUS);

            shape.dataSize = 3;

            shape.color = Color{
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                200
            };
        } break;

        case ShapeType::Ellipse: {
        } break;

        case ShapeType::Square: {
        } break;

        case ShapeType::Rectangle: {
            shape.type = ShapeType::Rectangle;

            shape.data[0] = GetRandomValue(0, m_originalImage.width);
            shape.data[1] = GetRandomValue(0, m_originalImage.height);
            shape.data[2] = GetRandomValue(10, std::min(m_originalImage.width, m_originalImage.height) / 2);
            shape.data[3] = GetRandomValue(10, std::min(m_originalImage.width, m_originalImage.height) / 2);

            shape.dataSize = 4;

            shape.color = Color{
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                200
            };
        } break;

        case ShapeType::Triangle: {
            shape.type = ShapeType::Triangle;

            shape.data[0] = GetRandomValue(0, m_originalImage.width);
            shape.data[1] = GetRandomValue(0, m_originalImage.height);

            shape.data[2] = GetRandomValue(0, m_originalImage.width);
            shape.data[3] = GetRandomValue(0, m_originalImage.height);

            shape.data[4] = GetRandomValue(0, m_originalImage.width);
            shape.data[5] = GetRandomValue(0, m_originalImage.height);

            shape.dataSize = 6;

            shape.color = Color{
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                200
            };
        } break;

        case ShapeType::Line: {
        } break;

        case ShapeType::Curve: {
        } break;
    }

    return shape;
}
