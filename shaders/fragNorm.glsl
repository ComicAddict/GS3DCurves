#version 330 core
out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;
in vec3 color;

uniform vec3 lightPos;
void main()
{
	//vec3 lightPos = vec3(20.0f,20.0f,20.0f);
	float br = dot(normalize(lightPos-fragPos), normal);// / length(lightPos-fragPos);
	vec3 col = color * br+0.1f;
	FragColor = vec4(col,1.0f);
}

