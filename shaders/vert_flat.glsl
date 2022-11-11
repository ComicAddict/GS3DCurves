#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

flat out vec3 Normal;
out vec3 fragPos;

void main() {
	fragPos = vec3(model * vec4(aPos, 1.0f));
	Normal = aNormal;
	gl_Position = projection * view * vec4(fragPos, 1.0f);
}