#include <raylib.h>
#include <iostream>

#include "abstractify.cpp"


#define MIN_CAMERA_ZOOM 0.01f
#define MAX_CAMERA_ZOOM 3.0f


int main()
{
    Vector2 screenResolution = { 800.0f, 800.0f };

    InitWindow(static_cast<int>(screenResolution.x), static_cast<int>(screenResolution.y), "Abstractify");
    // SetTraceLogLevel(LOG_NONE);

    Image image = LoadImage("image.png");
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Texture texture = LoadTextureFromImage(image);

    Abstractify abstractify(image, texture, Color{ 0, 0, 0, 255 });
    abstractify.SetShapesFlags(Abstractify::ShapeType::Circle);

    Camera2D camera = {};
    camera.offset   = Vector2{ 400.0f, 400.0f };
    camera.target   = Vector2{ static_cast<float>(image.width) / 2.0f, static_cast<float>(image.height) / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom     = 1.0f;

    while (!WindowShouldClose()) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 offset = GetMouseDelta();
            camera.target  = Vector2{ camera.target.x - offset.x / camera.zoom, camera.target.y - offset.y / camera.zoom };
        }

        camera.zoom += (static_cast<float>(GetMouseWheelMove()) * 0.2f * camera.zoom);

        if (camera.zoom > MAX_CAMERA_ZOOM)
            camera.zoom = MAX_CAMERA_ZOOM;
        else if (camera.zoom < MIN_CAMERA_ZOOM)
            camera.zoom = MIN_CAMERA_ZOOM;

        if (IsKeyPressed(KEY_R)) {
            camera.zoom     = 1.0f;
            camera.rotation = 0.0f;
        }

        abstractify.AddShape(10);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);
        {
            DrawTexture(abstractify.GetTexture(), 0, 0, WHITE);
        }
        EndMode2D();

        DrawFPS(5, 5);

        EndDrawing();
    }

    UnloadImage(image);
    UnloadTexture(texture);

    abstractify.UnloadData();

    CloseWindow();
    return 0;
}
