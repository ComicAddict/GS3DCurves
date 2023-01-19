#version 330 core

layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aNormal;
layout (location = 1) in vec3 aColor;
//layout (location = 3) in vec2 aTexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

out vec3 cv;
void main() {
	float PI = 3.14159265359f;
	float cmax = max(aColor.x, aColor.y);
	cmax = max(cmax, aColor.z);
	float cmin = min(aColor.x, aColor.y);
	cmin = min(cmin, aColor.z);
	float del = cmax - cmin;

	float h;
	float s;

	if(del == 0){
		s = 0;
		h = 0;
	} else {
		s = del / cmax;
		if(cmax == aColor.x){
			h = (aColor.g - aColor.b)/del;
		} else if(cmax == aColor.y){
			h = 2.0f + (aColor.b - aColor.r)/del;
		} else if(cmax == aColor.z){
			h = 4.0f + (aColor.r - aColor.g)/del;
		}
		h *= PI/3.0f;
	}
	
	cv.z = cmax;
	cv.x = cos(h) * s;
	cv.y = sin(h) * s;
	//fragPos = vec3(model * vec4(aPos, 1.0f));
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}
