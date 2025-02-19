#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// uniform vec2 screenResolution;
uniform vec2 position;
uniform int radius;

// Output fragment color
out vec4 finalColor;

// Draw a circle at vec2 `pos` with radius `rad` and color `color`.
// vec4 Circle(vec2 uv, vec2 pos, float rad, vec4 color)
// {
//     float dist = length(pos - uv) - rad;
//     return color * step(dist, 1.0);
// }

void main()
{
    // finalColor = Circle(gl_FragCoord.xy, position, radius, fragColor);
    finalColor = fragColor * step(length(position - gl_FragCoord.xy) - radius, 1.0);
}