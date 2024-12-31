#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D tex0; // Original image
uniform sampler2D tex1; // Other image

// Output fragment color
out vec4 finalColor;

void main()
{
    vec2 realCoord = vec2(fragTexCoord.x, 1.0 - fragTexCoord.y);
    vec4 colorA = texture2D(tex0, realCoord);
    vec4 colorB = texture2D(tex1, realCoord);

    vec4 diff = colorA - colorB;

    //     |(r1 - r2)² + (g1 - g2)² + (b1 - b2)² + (a1 - a2)²)|
    // sqrt|--------------------------------------------------|
    //     |                        4                         |
    finalColor = vec4(sqrt((diff.r * diff.r + diff.g * diff.g + diff.b * diff.b + diff.a * diff.a) / 4.0), 0.0, 0.0, 1.0);
}
