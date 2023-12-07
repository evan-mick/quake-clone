#version 330 core

// Task 16: Create a UV coordinate in variable
in vec2 UV;

uniform int width;
uniform int height;

uniform sampler2D textureSample;


uniform int kernel;
uniform int perpixel;

out vec4 fragColor;

vec4 convolveBlur(vec2 UV,int radius) {
    float horStep = 1.f/width;
    float vertStep = 1.f/height;
    int side = 2*radius + 1;
    float coeff = 1.f/(side * side);
    vec3 res = vec3(0);


    for(int r=-radius;r<=radius;r++) {
        for(int c=-radius;c<=radius;c++) {
            vec2 updatedUV = UV + vec2(r*vertStep,c*horStep);
            res += coeff*vec3(texture(textureSample,updatedUV));
        }
    }
//    normalize(res);
    return vec4(res,1);
}

void main()
{
//    fragColor = vec4(1);

    fragColor = texture(textureSample,UV);

    if(kernel==1) {
        fragColor = convolveBlur(UV,2);
    }

    if(perpixel==1) {
        fragColor = vec4(1) - fragColor;
    }
}
