# version 330 core
layout(lines) in;
layout(triangle_strip, max_vertices = 96) out;

out vec3 normal;
out vec3 fragPos;
//varying out vec3 fragPos;
const float PI = 3.1415926;

in vec3 vCol[];

out vec3 fCol;
uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

float atan2(in float y, in float x)
{
    bool s = (abs(x) > abs(y));
    return mix(PI/2.0 - atan(x,y), atan(y,x), s);
}

void main(){
	vec4 dir = gl_in[1].gl_Position - gl_in[0].gl_Position;
	float az = atan2(dir.y, dir.x);
	float biz = atan2(dir.z, dir.x);
	vec4 pos = projection * view * model * gl_in[0].gl_Position;
	fragPos = (model * gl_in[0].gl_Position).xyz;
	fCol = vCol[0];
	for(int i = 0; i < 16; i++){
		float angle1 = i * 2*PI/16;
		float angle2 = (i+1)* 2 *PI/16;
		float r = 0.5f;
		
		vec3 v1 = r * vec3(cos(angle1),sin(angle1),0.0f);
		vec3 v2 = r * vec3(cos(angle2),sin(angle2),0.0f);
		vec3 v3 = vec3(0.0f,0.0f,0.0f);
		normal = normalize(cross(v1-v2, v2-v3));
		
		gl_Position = pos + projection * view * model * vec4(v1,0.0f);
		EmitVertex();
		gl_Position = pos + projection * view * model * vec4(v2,0.0f);
		EmitVertex();
		gl_Position = pos + projection * view * model * vec4(v3,0.0f);
		EmitVertex();
	}
	EndPrimitive();
	pos = projection * view * model * gl_in[1].gl_Position;
	for(int i = 0; i < 16; i++){
		float angle1 = i * 2*PI/16;
		float angle2 = (i+1)* 2 *PI/16;
		float r = 0.5f;
		
		vec3 v1 = r * vec3(cos(angle1),sin(angle1),0.0f);
		vec3 v2 = r * vec3(cos(angle2),sin(angle2),0.0f);
		vec3 v3 = vec3(0.0f,0.0f,0.0f);
		normal = normalize(cross(v3-v1, v3-v2));
		
		gl_Position = pos + projection * view * model * vec4(v1,0.0f);
		EmitVertex();
		gl_Position = pos + projection * view * model * vec4(v2,0.0f);
		EmitVertex();
		gl_Position = pos + projection * view * model * vec4(v3,0.0f);
		EmitVertex();
	}
	
	EndPrimitive();
	
}
