#pragma once

#include <string>

inline const std::string QUADCOMET = R"#(
uniform mat3 proj;
uniform vec4 color;
attribute vec2 pos;
attribute vec2 texcoord;
attribute vec4 colors;
varying vec4 v_color;
varying vec2 v_texcoord;

void main() {
    gl_Position = vec4(proj * vec3(pos, 1.0), 1.0);
    v_color = color;
    v_texcoord = texcoord;
})#";

inline const std::string FRAGCOMET = R"#(
precision highp float;
varying vec4 v_color;
varying vec2 v_texcoord;
uniform vec4 window;
uniform int time;
uniform vec2 resolution;

const float time_stabilizer = 500000000.0;

float rand(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float distToAlpha(vec4 window, vec2 position) {
    vec2 center  = vec2(window.x + window.z/2.0, window.y + window.w/2.0);

    return 1.0 - max(abs(center.x - position.x), abs(center.y - position.y));

}

void main() {

    vec2 position = gl_FragCoord.xy / resolution.xy; // + vec2(0, 0);

	vec4 pixColor = v_color;
        



	if (rand(gl_FragCoord.xy / 2.0) >= 0.995)
	{
		float r = rand(gl_FragCoord.xy);
        // r = (sin(r) + 1.0) / 2.0;
		float color = (cos(float(time) / r / time_stabilizer) + 1.0) / 2.0;
        pixColor = vec4(vec3(max(pixColor.r , color), max(pixColor.g , color), max(pixColor.b , color)), color);	
    
    } else {

        float a = distToAlpha(window, position); 
        pixColor.rgb *= a;
        pixColor = vec4(vec3(pixColor.xyz), a);
    }
	gl_FragColor = pixColor;


})#";
