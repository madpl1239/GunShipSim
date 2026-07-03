#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;
in float vHeightFactor;

uniform vec3 uLightDirection;
uniform vec3 uCameraPosition;
uniform float uAmbientStrength;

out vec4 FragColor;


void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDirection);

    float diffuse = max(dot(N, L), 0.0);

    float slope = 1.0 - clamp(N.y, 0.0, 1.0);
    float height = clamp(vHeightFactor, 0.0, 1.0);

    vec3 sandLow  = vec3(0.76, 0.68, 0.48);
    vec3 sandHigh = vec3(0.58, 0.50, 0.34);
    vec3 rock     = vec3(0.42, 0.38, 0.31);
    vec3 ridge    = vec3(0.66, 0.58, 0.40);

    vec3 heightColor = mix(sandLow, sandHigh, smoothstep(0.10, 0.85, height));
    vec3 slopeColor = mix(heightColor, rock, smoothstep(0.18, 0.55, slope));
    vec3 ridgeColor = mix(slopeColor, ridge, smoothstep(0.55, 0.90, height) * 0.35);

    float terrainShade = uAmbientStrength + diffuse * 0.85;

    vec3 viewDir = normalize(uCameraPosition - vWorldPos);
    float rim = pow(1.0 - max(dot(N, viewDir), 0.0), 2.0);
    terrainShade += rim * 0.08;

    vec3 litColor = ridgeColor * terrainShade;

    float dist = distance(uCameraPosition, vWorldPos);
    float fogStart = 2500.0;
    float fogEnd   = 9000.0;
    float fogFactor = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);

    vec3 fogColor = vec3(0.78, 0.82, 0.86);
    vec3 finalColor = mix(litColor, fogColor, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}
