#version 120

uniform vec2 center;
uniform float radius;

void main(){
    vec2 pos = gl_FragCoord.xy;
    float dist = distance(pos, center);
    float d = clamp(dist / radius, 0.0, 1.0);

    float falloff = 1.0 - smoothstep(0.0, radius, dist);
    falloff = pow(falloff, 1.35);

    vec3 coreColor = vec3(1.0, 1.0, 1.0);
    vec3 midColor  = vec3(0.36, 0.14, 0.19);
    vec3 edgeColor = vec3(00.19, 0.24, 0.48);

    vec3 color = mix(coreColor, midColor, smoothstep(0.0, 0.45, d));
    color = mix(color, edgeColor, smoothstep(0.45, 1.0, d));

    gl_FragColor = vec4(color * falloff, 1.0);
}