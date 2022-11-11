#version 330 core

layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aCol;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

out vec3 fragPos;
//out vec3 fragCol;

void main() {
	fragPos = vec3(model * vec4(aPos, 1.0f));
	//fragCol = aCol;
	gl_Position = projection * view * vec4(fragPos, 1.0f);
}