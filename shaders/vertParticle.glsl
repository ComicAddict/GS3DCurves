#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 view;
uniform mat4 projection;

out vec3 fColor;
out vec3 fragPos;

void main() {
	fragPos = aPos;
	fColor = aColor;
	gl_Position = projection * view * vec4(fragPos, 1.0f);
}