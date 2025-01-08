#include "shape.hpp"


void Shape::mutate(const Vector2& imageSize, RNG& rng)
{
    static auto _mutate = [&imageSize, &rng](int& originalValue, const int& lo, const int& hi) {
        originalValue = Clamp(
            originalValue + rng.getRandomInt(-MAX_MUTATION_VAL, MAX_MUTATION_VAL),
            lo,
            hi
        );
    };

    switch (this->type) {
        case ShapeType::Circle: {
            if (rng.getRandomInt(1)) {
                // this->data[0] = Clamp(this->data[0] + rng.getRandomInt(-MAX_MUTATION_VAL, MAX_MUTATION_VAL), 0, static_cast<int>(imageSize.x) - 1);
                // this->data[1] = Clamp(this->data[1] + rng.getRandomInt(-MAX_MUTATION_VAL, MAX_MUTATION_VAL), 0, static_cast<int>(imageSize.y) - 1);
                _mutate(this->data[0], 0, static_cast<int>(imageSize.x) - 1);
                _mutate(this->data[1], 0, static_cast<int>(imageSize.y) - 1);
            }
            else {
                // this->data[2] = Clamp(this->data[2] + rng.getRandomInt(-MAX_MUTATION_VAL, MAX_MUTATION_VAL), MIN_CIRCLE_RADIUS, MAX_CIRCLE_RADIUS);
                _mutate(this->data[2], MIN_CIRCLE_RADIUS, MAX_CIRCLE_RADIUS);
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

        default: std::__throw_runtime_error("Failed to mutate this->");
    }
}


void Shape::randomize(const Vector2& imageSize, RNG& rng, const std::vector<ShapeType>& usableShapeTypes)
{
    switch (usableShapeTypes[rng.getRandomInt(usableShapeTypes.size() - 1)]) {
        case ShapeType::Circle: {
            this->type    = ShapeType::Circle;
            this->data[0] = rng.getRandomInt(imageSize.x - 1);
            this->data[1] = rng.getRandomInt(imageSize.y - 1);
            this->data[2] = rng.getRandomInt(MIN_CIRCLE_RADIUS, MAX_CIRCLE_RADIUS);

            this->color.r = static_cast<uint8_t>(rng.getRandomInt(0, 255));
            this->color.g = static_cast<uint8_t>(rng.getRandomInt(0, 255));
            this->color.b = static_cast<uint8_t>(rng.getRandomInt(0, 255));
        } break;

        case ShapeType::Ellipse: {
        } break;

        case ShapeType::Square: {
        } break;

        case ShapeType::Rectangle: {
        } break;

        case ShapeType::Triangle: {
            this->type = ShapeType::Triangle;

            this->data[0] = rng.getRandomInt(0, imageSize.x);
            this->data[1] = rng.getRandomInt(0, imageSize.y);

            this->data[2] = rng.getRandomInt(0, imageSize.x);
            this->data[3] = rng.getRandomInt(0, imageSize.y);

            this->data[4] = rng.getRandomInt(0, imageSize.x);
            this->data[5] = rng.getRandomInt(0, imageSize.y);

            this->color = Color{
                static_cast<uint8_t>(rng.getRandomInt(0, 255)),
                static_cast<uint8_t>(rng.getRandomInt(0, 255)),
                static_cast<uint8_t>(rng.getRandomInt(0, 255)),
                SHAPE_ALPHA
            };
        } break;

        case ShapeType::Line: {
        } break;

        case ShapeType::Curve: {
        } break;

        default: std::__throw_runtime_error("Failed to randomize this->");
    }
}
