#version 330 core

// Task 5: declare "in" variables for the world-space position and normal,
//         received post-interpolation from the vertex shader
in vec4 world_position;
in vec3 world_normal;
//in vec3 suf_to_light;


// Task 10: declare an out vec4 for your output color
out vec4 fragColor;


uniform float ka;
uniform float ks;
uniform float kd;

uniform float shininess;

uniform vec4 cam_pos;


uniform vec4 lights[8];
uniform vec3 light_fun[8];
uniform vec4 light_dir[8];
uniform vec3 light_in[8];
uniform int light_num;
uniform int type[8];
//uniform int is_directional[8];
//uniform int is_spot[8];

uniform float angle[8];
uniform float penumbra[8];


//uniform vec4 light_dir_;
//light_fun

uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;


float falloff(float x) {
    return -2. * x * x * x + 3 * x * x;
}


void main() {

    fragColor = vec4(0, 0, 0, 1.f);

    fragColor += ka * ambient;

    for (int i = 0; i < light_num; i++) {

        // Of note for types,
        // type 0 is point light
        // type 1 is directional
        // type 2 is spot light

        vec3 world_normal_normalized = vec3(normalize(world_normal));

        float intensity = 1.f;

        vec4 light_direction = normalize(-light_dir[i]);

        if (type[i] != 1)
            light_direction = normalize((lights[i] - world_position));

        // Spotlight logic
        if (type[i] == 2) {
            float inner = angle[i] - penumbra[i];
            float outer = angle[i];

            float ang = acos(dot(vec3(light_direction), -normalize(vec3(light_dir[i]))));

            if (ang > inner && ang <= outer) {
                float val = (ang - inner) / (outer - inner);
                intensity = intensity * (1.f - falloff(val));
            } else if (ang > outer) {
                intensity = 0.f;
            } else {
                intensity = 1.f;
            }
        }

        // dist and attenuation
        float dist = length(vec3(lights[i]) - vec3(world_position));
        float atten = min(1.f, 1.f / (light_fun[i].x + (dist * light_fun[i].y) + (dist * dist * light_fun[i].z)));

        if (type[i] == 1)
            atten = 1.f;

        // Add to frag col
        fragColor += intensity * atten * vec4(light_in[i], 1.f) * kd * (clamp(dot(light_direction, vec4(world_normal_normalized, 1.f)), 0, 1)) * diffuse;

        fragColor += intensity * atten * vec4(light_in[i], 1.f) * ks * pow(
                    clamp(dot(normalize(vec3(reflect(-light_direction, vec4(world_normal_normalized, 1.f)))),
                              normalize(vec3(cam_pos - world_position))), 0, 1),
                    shininess) * specular;

    }

}
