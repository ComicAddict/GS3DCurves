#version 330 core
out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;
in vec3 fCol;
void main()
{
	vec3 lightPos = vec3(20.0f,20.0f,20.0f);
	float br = dot(normalize(lightPos-fragPos), normal);
	vec3 col = fCol * br;
	FragColor = vec4(col,1.0f);
}

