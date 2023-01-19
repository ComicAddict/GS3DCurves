#version 330 core

layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aNormal;
layout (location = 1) in vec3 aColor;
//layout (location = 3) in vec2 aTexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

//out vec3 fragPos;
out vec3 fCol;
void main() {
	//fragPos = vec3(model * vec4(aPos, 1.0f));
	fCol = aColor;
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}