#version 330 core

// Task 4: declare a vec3 object-space position variable, using
//         the `layout` and `in` keywords.
layout(location = 0) in vec3 obs_position;
layout(location = 1) in vec3 obs_normal;

// Task 5: declare `out` variables for the world-space position and normal,
//         to be passed to the fragment shader
out vec4 world_position;
out vec3 world_normal;

//out vec3 suf_to_light;

//out float ka;

// Task 6: declare a uniform mat4 to store model matrix
uniform mat4 model_matrix;
//uniform mat3 inv_trans_model_mat;

// Task 7: declare uniform mat4's for the view and projection matrix
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

//uniform vec4 light_pos;




void main() {
    // Task 8: compute the world-space position and normal, then pass them to
    //         the fragment shader using the variables created in task 5

//    world_position = inverse(proj_matrix) * inverse(view_matrix) * gl_Position;
    world_position = model_matrix * vec4(obs_position, 1.f);
    // This only works because unit sphere, need to normalize obs position if its more
    world_normal = inverse(transpose(mat3(model_matrix))) * normalize(obs_normal); // normalize? i removed it but maybe thats the way

    // Recall that transforming normals requires obtaining the inverse-transpose of the model matrix!
    // In projects 5 and 6, consider the performance implications of performing this here.
//    suf_to_light = view_matrix * vec4(0, 0, 0, 1.f) - world_position;

//    suf_to_light = normalize(vec3(light_pos - world_position));


    // Task 9: set gl_Position to the object space position transformed to clip space
    gl_Position = proj_matrix * (view_matrix * model_matrix * vec4(obs_position, 1.f));
}
