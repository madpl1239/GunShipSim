#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in float aHeightFactor;

uniform mat4 uMVP;

out vec3 vWorldPos;
out vec3 vNormal;
out float vHeightFactor;


void main()
{
    vWorldPos = aPos;
    vNormal = normalize(aNormal);
    vHeightFactor = aHeightFactor;

    gl_Position = uMVP * vec4(aPos, 1.0);
}
