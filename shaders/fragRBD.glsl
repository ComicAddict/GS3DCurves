#version 330 core
out vec4 FragColor;

flat in vec3 fragPos;
flat in vec3 fNormal;

uniform vec3 lightPos;

// texture samplers
// uniform sampler2D texture1;
// uniform sampler2D texture2;

void main()
{
	float br = 0.1f + dot(normalize(lightPos-fragPos), fNormal);
	FragColor = vec4(br,br,br,1.0f);
}

