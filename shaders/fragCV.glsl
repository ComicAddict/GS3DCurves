#version 330 core
out vec4 FragColor;

in vec3 cv;

void main()
{
	float PI = 3.14159265359f;
	float s = sqrt(cv.x * cv.x + cv.y * cv.y);
	float h = atan(cv.y/cv.x);
	if(cv.x < 0) {
		h += PI;
	} else if(cv.x > 0 && cv.y < 0) {
		h += 2.0f * PI;
	} 
	h /= PI / 3.0f;
	
	float v = cv.z;
	//float c = v * s;
	//float x = c * (1- abs(mod(h,2) - 1));
	//float m = v - c;
	//vec3 rgb = vec3(m,m,m);
	
	float cmax = v;
	float cmin = cmax - s * cmax;
	float delta = cmax - cmin;
	vec3 rgb = vec3(0,0,0);
	
	if( h < 1)
		rgb = vec3(cmax, cmin + h * delta, cmin);
	else if(h < 2)
		rgb = vec3(cmax - (h - 1) * delta, cmax, cmin);
	else if(h < 3)
		rgb = vec3(cmin, cmax, cmin + (h - 2) * delta);
	else if(h < 4)
		rgb = vec3(cmin, cmax - (h - 3) * delta, cmax);
	else if(h < 5)
		rgb = vec3(cmin + (h - 4) * delta, cmin, cmax);
	else if(h < 6)
		rgb = vec3(cmax, cmin, cmax - (h - 5) * delta);
	
		

	FragColor = vec4(rgb, 1.0f);
}

