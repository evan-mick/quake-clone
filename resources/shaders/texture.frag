#version 330 core

// Task 16: Create a UV coordinate in variable
in vec2 uv_coord;
uniform vec2 uv_traverse;

// Task 8: Add a sampler2D uniform
uniform sampler2D tex;


uniform bool filt;
uniform bool box_blur;

out vec4 fragColor;

void main()
{
    fragColor = vec4(1);
    // Task 17: Set fragColor using the sampler2D at the UV coordinate
    fragColor = texture(tex, uv_coord);


    if (box_blur) {
        fragColor = vec4(0);
        for (int i = -2; i < 3; i++) {
            for (int j = -2; j < 3; j++) {
                fragColor +=
                    1.f/25.f * texture(tex, uv_coord + vec2(uv_traverse.x * i, uv_traverse.y * j));
            }
        }
    }


    if (filt)
        fragColor = 1 - fragColor;
    // vec4(uv_coord, 1.f, 1.f);
    // Task 33: Invert fragColor's r, g, and b color channels if your bool is true

//    if (filter)
//        fragColor = 1 - fragColor;

}
