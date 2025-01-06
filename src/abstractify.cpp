#include "abstractify.hpp"


void Abstractify::Shape::mutate(const Vector2& imageSize, RNG& rng)
{
    static auto mutate = [&imageSize, &rng](int& originalValue, const int& lo, const int& hi) {
        originalValue = Clamp(
            originalValue + rng.getRandomInt(-Settings::MAX_MUTATION_VAL, Settings::MAX_MUTATION_VAL),
            lo,
            hi
        );
    };

    switch (this->type) {
        case ShapeType::Circle: {
            if (rng.getRandomInt(1)) {
                // this->data[0] = Clamp(this->data[0] + rng.getRandomInt(-Settings::MAX_MUTATION_VAL, Settings::MAX_MUTATION_VAL), 0, static_cast<int>(imageSize.x) - 1);
                // this->data[1] = Clamp(this->data[1] + rng.getRandomInt(-Settings::MAX_MUTATION_VAL, Settings::MAX_MUTATION_VAL), 0, static_cast<int>(imageSize.y) - 1);
                mutate(this->data[0], 0, static_cast<int>(imageSize.x) - 1);
                mutate(this->data[1], 0, static_cast<int>(imageSize.y) - 1);
            }
            else {
                // this->data[2] = Clamp(this->data[2] + rng.getRandomInt(-Settings::MAX_MUTATION_VAL, Settings::MAX_MUTATION_VAL), Settings::MIN_CIRCLE_RADIUS, Settings::MAX_CIRCLE_RADIUS);
                mutate(this->data[2], Settings::MIN_CIRCLE_RADIUS, Settings::MAX_CIRCLE_RADIUS);
            }
        } break;

        case ShapeType::Ellipse: {
        } break;

        case ShapeType::Square: {
        } break;

        case ShapeType::Rectangle: {
        } break;

        case ShapeType::Triangle: {
        } break;

        case ShapeType::Line: {
        } break;

        case ShapeType::Curve: {
        } break;
    }
}


Abstractify::Abstractify(const Image& originalImage, const Texture& originalImageTexture)
    : m_originalImage(originalImage),
      m_originalTexture(originalImageTexture),
      m_imageSize(Vector2{ static_cast<float>(m_originalImage.width), static_cast<float>(m_originalImage.height) }),
      m_shapeShader(ShapeShader{
          .Circle    = LoadShader(0, "src/shaders/shapes/circle.frag"),
          .Ellipse   = LoadShader(0, "src/shaders/shapes/ellipse.frag"),
          .Square    = LoadShader(0, "src/shaders/shapes/square.frag"),
          .Rectangle = LoadShader(0, "src/shaders/shapes/rectangle.frag"),
          .Triangle  = LoadShader(0, "src/shaders/shapes/triangle.frag"),
          .Line      = LoadShader(0, "src/shaders/shapes/line.frag"),
          .Curve     = LoadShader(0, "src/shaders/shapes/curve.frag") }
      ),
      m_squaredErrorComputeShader("src/shaders/squared_error_calc.comp"),
      m_colorComputeShader("src/shaders/color_calc.comp"),
      m_avgColorComputeShader("src/shaders/avg_color.comp")
{
    // Generate blank texture
    Image img      = GenImageColor(originalImage.width, originalImage.height, BLANK);
    m_blankTexture = LoadTextureFromImage(img);
    UnloadImage(img);

    // Canvases used to draw temp texture onto
    m_shapeCanvasRenderTexture = LoadRenderTexture(originalImage.width, originalImage.height);
    m_diffRenderTexture        = LoadRenderTexture(originalImage.width, originalImage.height);

    {
        float result = 0;
        m_squaredErrorComputeShader.AddSSBO(&result, sizeof(result));
    }

    {
        unsigned int rgbaSum[5];
        int shapeData[8];
        m_colorComputeShader.AddSSBO(rgbaSum, sizeof(rgbaSum));
        m_colorComputeShader.AddSSBO(shapeData, sizeof(shapeData));
    }

    {
        unsigned int rgbaSum[4];
        m_avgColorComputeShader.AddSSBO(rgbaSum, sizeof(rgbaSum));
    }

    // Render texture for the final approximation
    m_resultRenderTexture = LoadRenderTexture(originalImage.width, originalImage.height);
    BeginTextureMode(m_resultRenderTexture);
    ClearBackground(m_ComputeAverageColor(m_originalTexture));
    EndTextureMode();
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
    m_squaredErrorComputeShader.Unload();
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
    Shape bestShape = m_GetRandomShape();
    bestShape.color = m_ComputeColor(bestShape);
    float bestScore = m_ComputeScore(bestShape);

    int age = 0;
    while (age < lifetime) {
        Shape tempShape = bestShape;

        tempShape.mutate(m_imageSize, m_RNG);
        tempShape.color = m_ComputeColor(tempShape);

        const float score = m_ComputeScore(tempShape);

        if (score < bestScore) {
            bestShape = tempShape;
            bestScore = score;
            age       = -1;
        }

        ++age;
    }

    m_DrawShape(m_resultRenderTexture, bestShape);
}


float Abstractify::m_ComputeScore(const Shape& shape)
{
    // Draw current approximation to canvas
    BeginTextureMode(m_shapeCanvasRenderTexture);
    ClearBackground(BLACK);
    DrawTexture(m_resultRenderTexture.texture, 0, 0, WHITE);
    EndTextureMode();

    m_DrawShape(m_shapeCanvasRenderTexture, shape);

    float squaredDiff = 0.0f;

    rlEnableShader(m_squaredErrorComputeShader.program);
    {
        // Reset SSBO
        m_squaredErrorComputeShader.UpdateSSBO(0, &squaredDiff, sizeof(squaredDiff));

        // Set uniform
        int size[2] = { m_originalTexture.width, m_originalTexture.height };
        rlSetUniform(rlGetLocationUniform(m_squaredErrorComputeShader.program, "resolution"), size, SHADER_UNIFORM_IVEC2, 1);

        // Bind textures
        rlBindImageTexture(m_originalTexture.id, 0, m_originalTexture.format, true);
        rlBindImageTexture(m_shapeCanvasRenderTexture.texture.id, 1, m_shapeCanvasRenderTexture.texture.format, true);

        // Bind SSBO
        rlBindShaderBuffer(m_squaredErrorComputeShader.ssbos[0], 2);

        // Dispatch compute shader
        rlComputeShaderDispatch((m_originalTexture.width + 15) / 16, (m_originalTexture.height + 15) / 16, 1);
    }
    rlDisableShader();

    // Read squared diff from SSBO
    rlReadShaderBuffer(m_squaredErrorComputeShader.ssbos[0], &squaredDiff, sizeof(squaredDiff), 0);

    return std::sqrt(squaredDiff / static_cast<float>(4 * m_imageSize.x * m_imageSize.y));
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
        // static_cast<uint8_t>(data[3] / data[4])
        Settings::SHAPE_ALPHA
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

            BeginShaderMode(m_shapeShader.Circle);
            DrawTexture(m_blankTexture, 0, 0, shape.color);
            EndShaderMode();
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

            BeginShaderMode(m_shapeShader.Rectangle);
            DrawTexture(m_blankTexture, 0, 0, shape.color);
            EndShaderMode();
        } break;

        case ShapeType::Triangle: {
            Vector2 v1 = { static_cast<float>(shape.data[0]), static_cast<float>(shape.data[1]) };
            Vector2 v2 = { static_cast<float>(shape.data[2]), static_cast<float>(shape.data[3]) };
            Vector2 v3 = { static_cast<float>(shape.data[4]), static_cast<float>(shape.data[5]) };

            SetShaderValue(m_shapeShader.Triangle, GetShaderLocation(m_shapeShader.Triangle, "v1"), &v1, SHADER_UNIFORM_VEC2);
            SetShaderValue(m_shapeShader.Triangle, GetShaderLocation(m_shapeShader.Triangle, "v2"), &v2, SHADER_UNIFORM_VEC2);
            SetShaderValue(m_shapeShader.Triangle, GetShaderLocation(m_shapeShader.Triangle, "v3"), &v3, SHADER_UNIFORM_VEC2);

            BeginShaderMode(m_shapeShader.Triangle);
            DrawTexture(m_blankTexture, 0, 0, shape.color);
            EndShaderMode();
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
            shape.data[0] = m_RNG.getRandomInt(m_originalImage.width - 1);
            shape.data[1] = m_RNG.getRandomInt(m_originalImage.height - 1);
            shape.data[2] = m_RNG.getRandomInt(Settings::MIN_CIRCLE_RADIUS, Settings::MAX_CIRCLE_RADIUS);

            shape.dataSize = 3;

            shape.color = Color{
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                Settings::SHAPE_ALPHA
            };
        } break;

        case ShapeType::Ellipse: {
        } break;

        case ShapeType::Square: {
        } break;

        case ShapeType::Rectangle: {
            shape.type = ShapeType::Rectangle;

            shape.data[0] = m_RNG.getRandomInt(m_originalImage.width - 1);
            shape.data[1] = m_RNG.getRandomInt(m_originalImage.height - 1);
            shape.data[2] = m_RNG.getRandomInt(10, Min(m_originalImage.width, m_originalImage.height) / 2);
            shape.data[3] = m_RNG.getRandomInt(10, Min(m_originalImage.width, m_originalImage.height) / 2);

            shape.dataSize = 4;

            shape.color = Color{
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                static_cast<uint8_t>(GetRandomValue(0, 255)),
                Settings::SHAPE_ALPHA
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
                Settings::SHAPE_ALPHA
            };
        } break;

        case ShapeType::Line: {
        } break;

        case ShapeType::Curve: {
        } break;
    }

    return shape;
}


Color Abstractify::m_ComputeAverageColor(const Texture& texture)
{
    unsigned int data[4] = { 0, 0, 0, 0 }; // { r, g, b, a }

    rlEnableShader(m_avgColorComputeShader.program);
    {
        // Reset / set SSBOs
        m_avgColorComputeShader.UpdateSSBO(0, data, sizeof(data));

        // Set resolution uniform
        int size[2] = { texture.width, texture.height };
        rlSetUniform(rlGetLocationUniform(m_avgColorComputeShader.program, "resolution"), size, SHADER_UNIFORM_IVEC2, 1);

        // Bind texture
        rlBindImageTexture(texture.id, 0, texture.format, true);

        // Bind SSBO
        rlBindShaderBuffer(m_avgColorComputeShader.ssbos[0], 1);

        // Dispatch compute shader
        rlComputeShaderDispatch((texture.width + 15) / 16, (texture.height + 15) / 16, 1);
    }
    rlDisableShader();

    // Read result from SSBO
    rlReadShaderBuffer(m_avgColorComputeShader.ssbos[0], data, sizeof(data), 0);

    int total = texture.width * texture.height;

    return Color{
        static_cast<uint8_t>(data[0] / total),
        static_cast<uint8_t>(data[1] / total),
        static_cast<uint8_t>(data[2] / total),
        static_cast<uint8_t>(data[3] / total)
    };
}
