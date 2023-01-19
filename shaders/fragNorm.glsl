#version 330 core
out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;
in vec3 fCol;

float near = .01f; 
float far  = 200.0f; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

uniform vec3 lightPos;
void main()
{
	float br = (dot(normalize(lightPos-fragPos), normal) + 1.0f)/2.0f;// / length(lightPos-fragPos);
	vec3 col = fCol * br;
	float depth = LinearizeDepth(gl_FragCoord.z) / far;
	FragColor = vec4(vec3(1.0f-depth) * col,1.0f);
}

