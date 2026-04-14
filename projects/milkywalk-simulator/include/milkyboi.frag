#version 120

uniform vec2 center;
uniform float radius;

void main(){
    vec2 pos = gl_FragCoord.xy;
    float dist = distance(pos, center);
    float d = clamp(dist / radius, 0.0, 1.0);

    vec3 coreColor = vec3(1.0, 1.0, 1.0);
    vec3 midColor  = vec3(0.36, 0.14, 0.19);
    vec3 edgeColor = vec3(0.19, 0.24, 0.48);

    vec3 color = mix(coreColor, midColor, smoothstep(0.0, 0.25, d));
    color = mix(color, edgeColor, smoothstep(0.25, 0.75, d));

    gl_FragColor = vec4(color, 1.0);
}