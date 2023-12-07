#version 330 core

in vec3 worldSpacePos;
in vec3 worldSpaceNorm;
out vec4 fragColor;
uniform float k_a;
uniform float k_d;
uniform float k_s;
uniform int shininess;

uniform vec4 cAmbient;
uniform vec4 cDiffuse;
uniform vec4 cSpecular;

uniform vec4 lightPos[8];
uniform vec4 lightColor[8];
uniform int lightType[8];
uniform vec4 lightDir[8];
uniform vec2 lightAngle[8];
uniform vec3 lightAtt[8];
uniform int numLights;

uniform vec4 worldSpaceCameraPos;

float attentuation(int i) {
    if(lightType[i]==1) {
        return 1.0;
    }
    float dist = distance(vec3(worldSpacePos),vec3(lightPos[i]));
    return min(1.0,1.0/(lightAtt[i][0] + lightAtt[i][1] * dist + lightAtt[i][2] * dist * dist));
}

vec4 ambient() {
    return k_a*cAmbient;
}

float angularFalloff(float x, float inner, float outer) {
    float ratio = (x - inner) / (outer - inner);
    return -2.0 * pow(ratio, 3.0) + 3.0 * pow(ratio, 2.0);
}

vec4 falloffIllumination(int i) {
    if(lightType[i]!=2) {
        return lightColor[i];
    }
    vec3 currentDir = normalize(worldSpacePos - vec3(lightPos[i]));
    float x = acos(dot(currentDir,normalize(vec3(lightDir[i]))));
    if(x <= lightAngle[i][0] - lightAngle[i][1]) {
        return lightColor[i];
    } else if(x>lightAngle[i][0]){
        return vec4(0,0,0,0);
    }
    return lightColor[i] * (1.f - angularFalloff(x,lightAngle[i][0] - lightAngle[i][1],lightAngle[i][0]));
}

vec4 diffuse(int i) {
    vec3 L;
    if(lightType[i]!=1) {
        L = worldSpacePos - vec3(lightPos[i]);
    } else {
        L = vec3(lightDir[i]);
    }
    float dotLN = clamp(dot(-1*normalize(L),normalize(worldSpaceNorm)),0,1.0);
    float diffuse = dotLN * k_d;
    return diffuse * falloffIllumination(i) * cDiffuse * attentuation(i);
}

vec4 specular(int i) {
    vec3 L;
    if(lightType[i]!=1) {
        L = normalize(vec3(lightPos[i])-worldSpacePos);
    } else {
        L = -1*normalize(vec3(lightDir[i]));
    }
    vec3 E = normalize(vec3(worldSpaceCameraPos) - vec3(worldSpacePos));
    float dotLE = clamp(dot(reflect(-L,normalize(worldSpaceNorm)),E),0.0,1.0);
    float specular = k_s * pow(dotLE,shininess);
    if(specular > 0) {
        return specular * falloffIllumination(i) * cSpecular * attentuation(i);
    } return vec4(0,0,0,0);
}

void main() {
//    normalize(worldSpaceNorm);
//    float r = min(max(worldSpaceNorm[0],0.f),1.f);
//    float g = min(max(worldSpaceNorm[1],0.f),1.f);
//    float b = min(max(worldSpaceNorm[2],0.f),1.f);
//    fragColor = vec4(r,g,b,1.0);
    vec4 res;
    res += ambient();
    for(int i=0;i<numLights;i++) {
        res += diffuse(i);
        res += specular(i);
    }
    res[3] = 1;
    fragColor = res;
}
