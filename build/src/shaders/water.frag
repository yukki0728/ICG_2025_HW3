#version 330 core
out vec4 FragColor;

in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 viewPos;
uniform vec3 colorFilter;
uniform float alpha;

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(viewPos - vWorldPos);

    float fresnel = pow(1.0 - max(dot(N, V), 0.0), 3.0);

    vec3 baseColor = colorFilter * 1.2;
    vec3 edgeColor = vec3(1.0); //邊緣是白色的
    vec3 finalRGB = mix(baseColor, edgeColor, fresnel);

    float finalAlpha = mix(0.85, 1.0, fresnel);

    FragColor = vec4(finalRGB, finalAlpha * alpha);
}
