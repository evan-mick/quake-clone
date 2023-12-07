#version 330 core


layout(location = 0) in vec3 objectSpacePos;
layout(location = 1) in vec3 objectSpaceNorm;

out vec3 worldSpacePos;
out vec3 worldSpaceNorm;

uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 projMatrix;

void main() {
    worldSpacePos = vec3(modelMatrix * vec4(objectSpacePos[0],objectSpacePos[1],objectSpacePos[2],1.0));
    mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
    worldSpaceNorm = normalize(normalMatrix * normalize(objectSpaceNorm));

    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(objectSpacePos, 1.0);
}
