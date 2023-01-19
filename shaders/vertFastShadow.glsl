#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

//out vec3 fragPos;
out vec2 TexCoord;

void main() {
	//fragPos = vec3(model * vec4(aPos, 1.0f));
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}