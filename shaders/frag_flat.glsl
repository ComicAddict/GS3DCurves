#version 330 core
out vec4 FragColor;

flat in vec3 Normal;
in vec3 fragPos;

void main()
{
	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(vec3(100.0f, 100.0f, 100.0f) - fragPos);
	
	FragColor = (0.2f + max(dot(normal,lightDir),0.0)) * vec4(1.0f, 1.0f, 1.0f, 1.0f);
}