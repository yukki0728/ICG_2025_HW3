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

    // 計算菲涅耳係數 (1.0 - cosθ)
    // pow 裡面的數值越大，光暈越往邊緣集中
    float fresnel = pow(1.0 - max(dot(N, V), 0.0), 3.0);

    // 基礎顏色與邊緣亮色的混合
    vec3 baseColor = colorFilter * 1.2;
    vec3 edgeColor = vec3(1.0); // 邊緣亮白色
    vec3 finalRGB = mix(baseColor, edgeColor, fresnel);

    // 關鍵：透明度也受菲涅耳影響
    // 讓邊緣更厚實 (alpha 較高)，中間更通透
    float finalAlpha = mix(0.85, 1.0, fresnel);

    FragColor = vec4(finalRGB, finalAlpha * alpha);
}