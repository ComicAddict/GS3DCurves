#version 330 core
out vec4 FragColor;

in vec3 aCol;
in vec3 fragPos;

void main()
{
	FragColor = vec4(aCol, 1.0f);
}