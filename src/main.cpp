#include <raylib.h>
#include <iostream>

#include "abstractify.cpp"


int main()
{
    Vector2 screenResolution = { 800.0f, 800.0f };

    InitWindow(static_cast<int>(screenResolution.x), static_cast<int>(screenResolution.y), "Geometrize");
    SetTraceLogLevel(LOG_NONE);

    Image image     = LoadImage("image.png");
    Texture texture = LoadTexture("image.png");

    Abstractify abstractify(image, texture, Color{ 0, 0, 0, 0 }, &screenResolution);
    abstractify.SetShapesFlags(Abstractify::ShapeType::Circle);

    Camera2D camera = {};
    camera.offset   = Vector2{ 400.0f, 400.0f };
    camera.target   = Vector2{ (float)image.width / 2.0f, (float)image.height / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom     = 1.0f;

    while (!WindowShouldClose()) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 offset = GetMouseDelta();
            camera.offset  = Vector2{ camera.offset.x + offset.x, camera.offset.y + offset.y };
        }

        camera.zoom += ((float)GetMouseWheelMove() * 0.05f);

        if (camera.zoom > 3.0f)
            camera.zoom = 3.0f;
        else if (camera.zoom < 0.1f)
            camera.zoom = 0.1f;

        if (IsKeyPressed(KEY_R)) {
            camera.zoom     = 1.0f;
            camera.rotation = 0.0f;
        }

        abstractify.AddShape(1);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);
        {
            DrawTexture(abstractify.GetTexture(), 0, 0, WHITE);
        }
        EndMode2D();

        DrawFPS(0, 0);

        EndDrawing();
    }

    UnloadImage(image);
    UnloadTexture(texture);

    abstractify.UnloadData();

    CloseWindow();
    return 0;
}
