#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

uniform vec2 v1; // Triangle vertex 1
uniform vec2 v2; // Triangle vertex 2
uniform vec2 v3; // Triangle vertex 3

// Output fragment color
out vec4 finalColor;

float EdgeFunction(vec2 a, vec2 b, vec2 c)
{
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

void main()
{
    float area = EdgeFunction(v1, v2, v3);
    float w1 = EdgeFunction(v2, v3, fragTexCoord) / area;
    float w2 = EdgeFunction(v3, v1, fragTexCoord) / area;
    float w3 = EdgeFunction(v1, v2, fragTexCoord) / area;

    // Check if pixel is inside the triangle
    if (w1 >= 0.0 && w2 >= 0.0 && w3 >= 0.0)
    {
        finalColor = fragColor;
    }
    else
    {
        // finalColor = vec4(0.5, 0.5, 0.5, 1.0);
        finalColor = vec4(0.0);
    }
}
