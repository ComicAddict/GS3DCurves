# version 330 core
layout(lines) in;
layout(triangle_strip, max_vertices = 200) out;

out vec3 normal;
out vec3 fragPos;
out vec3 fCol;
//varying out vec3 fragPos;
const float PI = 3.1415926;

in vec3 vCol[];

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform float rad;

float atan2(in float y, in float x)
{
    bool s = (abs(x) > abs(y));
    return mix(PI/2.0 - atan(x,y), atan(y,x), s);
}

mat3 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c
                );
}

void main(){
	vec4 dir = normalize(gl_in[1].gl_Position - gl_in[0].gl_Position);
	vec3 ofdir = dir.xyz + vec3(0.1f,0.2f,0.0f);
	vec3 perp = normalize(cross(dir.xyz, ofdir));
	float az = atan2(dir.y, dir.x);
	float biz = atan2(dir.z, dir.x);
	vec4 pos1 = projection * view * model * gl_in[0].gl_Position;
	vec4 pos2 = projection * view * model * gl_in[1].gl_Position;
	fragPos = (model * gl_in[0].gl_Position).xyz;
	fCol = vCol[0];
	int res = 8;
	mat3 rot = rotationMatrix(dir.xyz, 2*PI/res);
	for(int i = 0; i < res; i++){
		
		vec3 v1 = rad * perp;
		perp = rot * perp;
		vec3 v2 = rad * perp;
		vec3 v3 = vec3(0.0f,0.0f,0.0f);
		
		normal = normalize(cross(v1-v2, v2-v3));

		gl_Position = pos1 + projection * view * model * vec4(v1,0.0f);
		EmitVertex();
		gl_Position = pos1 + projection * view * model * vec4(v2,0.0f);
		EmitVertex();
		gl_Position = pos1 + projection * view * model * vec4(v3,0.0f);
		EmitVertex();
		normal = normalize(cross(v1-v2, dir.xyz));
		gl_Position = pos1 + projection * view * model * vec4(v1,0.0f);
		EmitVertex();
		gl_Position = pos1 + projection * view * model * vec4(v2,0.0f);
		EmitVertex();
		gl_Position = pos2 + projection * view * model * vec4(v1,0.0f);
		EmitVertex();
		//normal = normalize(cross(v1-v2, dir.xyz));
		//gl_Position = pos2 + projection * view * model * vec4(v1,0.0f);
		//EmitVertex();
		//gl_Position = pos2 + projection * view * model * vec4(v2,0.0f);
		//EmitVertex();
		gl_Position = pos2 + projection * view * model * vec4(v2,0.0f);
		EmitVertex();
		normal = normalize(cross(v2-v1, v2-v3));
		gl_Position = pos2 + projection * view * model * vec4(v1,0.0f);
		EmitVertex();
		gl_Position = pos2 + projection * view * model * vec4(v2,0.0f);
		EmitVertex();
		gl_Position = pos2 + projection * view * model * vec4(v3,0.0f);
		EmitVertex();
	}
	EndPrimitive();
	
}
