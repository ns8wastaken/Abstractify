#include <raylib.h>
#include <rlgl.h>
#include <cmath>
#include <stdio.h>

#include "./../src/compute_shader.hpp"

int main()
{
    Vector2 screenResolution = { 1000.0f, 800.0f };

    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(static_cast<int>(screenResolution.x), static_cast<int>(screenResolution.y), "Abstractify");
    SetTraceLogLevel(LOG_NONE);

    ComputeShader squaredErrorComputeShader("src/shaders/squared_error_calc.comp");

    float result = 0;
    float rmse   = 0.0f;
    squaredErrorComputeShader.AddSSBO(&result, sizeof(result));

    Image image1 = LoadImage("tests/image1.png");
    Image image2 = LoadImage("tests/image2.png");
    ImageFormat(&image1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&image2, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Texture texture1 = LoadTextureFromImage(image1);
    Texture texture2 = LoadTextureFromImage(image2);
    UnloadImage(image1);
    UnloadImage(image2);

    Image image3 = LoadImage("tests/image3.png");
    Image image4 = LoadImage("tests/image4.png");
    ImageFormat(&image3, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&image4, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Texture texture3 = LoadTextureFromImage(image3);
    Texture texture4 = LoadTextureFromImage(image4);
    UnloadImage(image3);
    UnloadImage(image4);


    result = 0;

    rlEnableShader(squaredErrorComputeShader.program);
    {
        // Reset SSBO
        squaredErrorComputeShader.UpdateSSBO(0, &result, sizeof(result));

        // Set uniform
        int size[2] = { texture1.width, texture1.height };
        rlSetUniform(rlGetLocationUniform(squaredErrorComputeShader.program, "resolution"), size, SHADER_UNIFORM_IVEC2, 1);

        // Bind textures
        rlBindImageTexture(texture1.id, 0, texture1.format, true);
        rlBindImageTexture(texture2.id, 1, texture2.format, true);

        // Bind SSBO
        rlBindShaderBuffer(squaredErrorComputeShader.ssbos[0], 2);

        // Dispatch compute shader
        rlComputeShaderDispatch((texture1.width + 15) / 16, (texture1.height + 15) / 16, 1);
    }
    rlDisableShader();

    // Read result from SSBO
    rlReadShaderBuffer(squaredErrorComputeShader.ssbos[0], &result, sizeof(result), 0);

    rlCheckErrors();

    printf("\nImages 1 & 2:\nEquation = sqrt( (255 * 255 * 3) / (4 * 1000 * 1000) )\n");
    printf("Squared error: %f\n", result);

    rmse = std::sqrt(result / static_cast<float>(4 * texture1.width * texture1.height));
    printf("RMSE: %f\n", rmse);




    result = 0;

    rlEnableShader(squaredErrorComputeShader.program);
    {
        // Reset SSBO
        squaredErrorComputeShader.UpdateSSBO(0, &result, sizeof(result));

        // Set uniform
        int size[2] = { texture3.width, texture3.height };
        rlSetUniform(rlGetLocationUniform(squaredErrorComputeShader.program, "resolution"), size, SHADER_UNIFORM_IVEC2, 1);

        // Bind textures
        rlBindImageTexture(texture3.id, 0, texture3.format, true);
        rlBindImageTexture(texture4.id, 1, texture4.format, true);

        // Bind SSBO
        rlBindShaderBuffer(squaredErrorComputeShader.ssbos[0], 2);

        // Dispatch compute shader
        rlComputeShaderDispatch((texture3.width + 15) / 16, (texture3.height + 15) / 16, 1);
    }
    rlDisableShader();

    // Read result from SSBO
    rlReadShaderBuffer(squaredErrorComputeShader.ssbos[0], &result, sizeof(result), 0);

    rlCheckErrors();

    printf("\nImages 3 & 4:\nEquation = sqrt( (255 * 255 * 500 * 250) / (4 * 500 * 500) )\n");
    printf("Squared error: %f\n", result);

    rmse = std::sqrt(result / static_cast<float>(4 * texture3.width * texture3.height));
    printf("RMSE: %f\n", rmse);

    printf("\n");


    squaredErrorComputeShader.Unload();
    CloseWindow();
    return 0;
}
