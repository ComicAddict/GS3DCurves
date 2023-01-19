#version 330 core

out vec4 FragColor;

in vec3 col;
flat in int fmode;

const float PI = 3.14159265359f;
// standard illuminant D65
/*
const float XN = 95.0489f;
const float YN = 100.0f;
const float ZN = 108.8840f;
*/

// Open CV implementation
const float XN = 0.950456;
const float YN = 1.0;
const float ZN = 1.088754;

vec3 rgb2CIEXYZ(vec3 rgb){
	mat3x3 C = mat3x3(	0.4124564, 0.3575761, 0.1804375,
						0.2126729, 0.7151522, 0.0721750,
						0.0193339, 0.1191920, 0.9503041);
	vec3 xyz = C*rgb;
	return xyz;
}

vec3 CIEXYZ2rgb(vec3 xyz){
	mat3x3 C = mat3x3(	3.2404542, -1.5371385, -0.4985314,
						-0.9692660, 1.8760106, 0.0415560,
						0.0556434, -0.2040259, 1.0572252);
	vec3 rgb = C*xyz;
	return rgb;
}


float trFuncLab(float a){

	float delta = 6.0/29.0; 
	if(a > pow(delta,3))
		return pow(a,1.0/3.0);
	return 4.0/29.0 + a / (3*pow(delta,2));
	
	//openCV implementation
	/*
	if(a > 0.008856){
		return pow(a, 1.0/3.0);
	}
	return 7.787 * a + 16.0/116.0;
	*/
}

float invFuncLab(float a){
	float delta = 6.0/29.0; 
	if(a > pow(delta,3))
		return pow(a,3);
	return 3 * pow(delta,2) * (a - (4.0/29.0) );
	
	// openCV implementation
	/*
	if(a < 0.20689336){
		return pow(a, 3);
	}
	return (a - (16.0/116.0)) / 7.887;
	*/
}

vec3 CIEXYZ2CIELAB(vec3 xyz){
	vec3 lab;
	
	lab.x = 116 * trFuncLab(xyz.y/YN) - 16.0;
	lab.y = 500 * (trFuncLab(xyz.x/XN) - trFuncLab(xyz.y/YN));
	lab.z = 200 * (trFuncLab(xyz.y/YN) - trFuncLab(xyz.z/ZN));
	
	//openCV Implementation
	/*
	if(xyz.y > 0.008856){
		lab.x = 116.0 * pow(xyz.y, 1.0/3.0) - 16.0;
	} else {
		lab.x = 903.3 * xyz.y;
	}
	lab.y = 500 * (trFuncLab(xyz.x/XN) - trFuncLab(xyz.y/YN));
	lab.z = 200 * (trFuncLab(xyz.y/YN) - trFuncLab(xyz.z/ZN));
	*/
	return lab;
}

vec3 CIELAB2CIEXYZ(vec3 lab){
	// standard illuminant d65
	vec3 xyz;
	
	xyz.x = XN * invFuncLab(((lab.x + 16.0)/116.0) + (lab.y/500.0));
	xyz.y = YN * invFuncLab((lab.x + 16.0)/116.0);
	xyz.z = ZN * invFuncLab(((lab.x + 16.0)/116.0) - (lab.z/200.0));
	
	//openCV implementation
	/*
	if(lab.x > 8){
		xyz.y = pow((lab.x + 16.0) / 116.0f, 3.0);	
	} else {
		xyz.y = lab.x / 903.3f;	
	}
	
	xyz.x = XN * invFuncLab((lab.x + 16.0) / 116.0f + (lab.y/500.0));
	xyz.z = ZN * invFuncLab((lab.x + 16.0) / 116.0f - (lab.z/200.0));
	*/
	return xyz;
}

vec3 rgb2hsv(vec3 rgb){
	float cmax = max(rgb.x, rgb.y);
	cmax = max(cmax, rgb.z);
	float cmin = min(rgb.x, rgb.y);
	cmin = min(cmin, rgb.z);
	float del = cmax - cmin;
	vec3 hsv;

	if(del == 0){
		hsv.x = 0;
		hsv.y = 0;
	} else {
		hsv.y = del / cmax;
		if(cmax == rgb.x){
			hsv.x = mod((rgb.g - rgb.b)/del, 6);
		} else if(cmax == rgb.y){
			hsv.x = 2.0f + (rgb.b - rgb.r)/del;
		} else if(cmax == rgb.z){
			hsv.x = 4.0f + (rgb.r - rgb.g)/del;
		}
		hsv.x *= PI/3.0f;
	}
	hsv.z = cmax;
	return hsv;
}

vec3 hsv2cv(vec3 hsv){
	vec3 cv;
	cv.x = cos(hsv.x) * hsv.y;
	cv.y = sin(hsv.x) * hsv.y;
	cv.z = hsv.z;
	return cv;
}

vec3 rgb2cv(vec3 rgb){
	vec3 hsv = rgb2hsv(rgb);
	return hsv2cv(hsv);
}

vec3 cv2hsv(vec3 cv){
	float s = sqrt(cv.x * cv.x + cv.y * cv.y);
	float h = atan(cv.y/cv.x);
	if(cv.x < 0) {
		h += PI;
	} else if(cv.x > 0 && cv.y < 0) {
		h += 2.0f * PI;
	} 
	
	float v = cv.z;

	return vec3(h,s,v);
}

vec3 hsv2rgb(vec3 hsv){
	float s = hsv.y;
	float h = hsv.x;
	h /= PI / 3.0f;
	
	float v = hsv.z;
	
	float cmax = v;
	float cmin = cmax - s * cmax;
	float delta = cmax - cmin;
	vec3 rgb = vec3(0,0,0);
	
	if(h >= 0 && h < 1)
		rgb = vec3(cmax, cmin + h * delta, cmin);
	else if(h < 2)
		rgb = vec3(cmax - (h - 1) * delta, cmax, cmin);
	else if(h < 3)
		rgb = vec3(cmin, cmax, cmin + (h - 2) * delta);
	else if(h < 4)
		rgb = vec3(cmin, cmax - (h - 3) * delta, cmax);
	else if(h < 5)
		rgb = vec3(cmin + (h - 4) * delta, cmin, cmax);
	else if(h <= 6)
		rgb = vec3(cmax, cmin, cmax - (h - 5) * delta);
	return rgb;
}

vec3 srgb2rgb(vec3 srgb){
	vec3 rgb;
	if(srgb.x < 0.04045f){
		rgb.x = srgb.x/12.92f;
	} else{
		rgb.x = pow(((srgb.x+0.055f)/1.055f),2.4);
	}
	if(srgb.y < 0.04045f){
		rgb.y = srgb.y/12.92f;
	} else {
		rgb.y = pow(((srgb.y+0.055f)/1.055f),2.4);
	}
	if(srgb.z < 0.04045f){
		rgb.z = srgb.z/12.92f;
	} else {
		rgb.z = pow(((srgb.z+0.055f)/1.055f),2.4);
	}
	return rgb;
}

vec3 rgb2srgb(vec3 rgb){
	vec3 srgb;
	if(rgb.x < 0.0031308){
		srgb.x = rgb.x*12.92f;
	} else{
		srgb.x = pow(rgb.x,1.0/24.0)*1.055-0.055;
	}
	if(rgb.y < 0.0031308){
		srgb.y = rgb.y*12.92f;
	} else {
		srgb.y = pow(rgb.y,1.0/24.0)*1.055-0.055;
	}
	if(rgb.z < 0.0031308){
		srgb.z = rgb.z*12.92f;
	} else {
		srgb.z = pow(rgb.z,1.0/24.0)*1.055-0.055;
	}
	return srgb;
}

vec3 cv2rgb(vec3 cv){
	return hsv2rgb(cv2hsv(cv));
}

vec3 rbg2CIELAB(vec3 rgb){
	return CIEXYZ2CIELAB(rgb2CIEXYZ(rgb));
}

vec3 CIELAB2rgb(vec3 lab){
	return CIEXYZ2rgb(CIELAB2CIEXYZ(lab));
}


vec3 CIELAB2CIEHLC(vec3 lab){
	vec3 hlc;
	hlc.x = atan(lab.z/lab.y);
	if(lab.y < 0) {
		hlc.x += PI;
	} else if(lab.y > 0 && lab.z < 0) {
		hlc.x += 2.0f * PI;
	} 
	hlc.y = lab.x;
	hlc.z = length(lab.yz);
	return hlc;
}

vec3 CIEHLC2CIELAB(vec3 hlc){
	vec3 lab;
	lab.x = hlc.y;
	lab.y = hlc.z * cos(hlc.x);
	lab.z = hlc.z * sin(hlc.x);
	return lab;
}

vec3 rgb2CIEHLC(vec3 rgb){
	return CIELAB2CIEHLC(rbg2CIELAB(rgb));
}

vec3 CIEHLC2rgb(vec3 hlc){
	return CIELAB2rgb(CIEHLC2CIELAB(hlc));
}

vec3 CIEXYZ2CIELUV(vec3 xyz){
	vec3 luv;
	float t = 6.0/29.0;
	if(xyz.y/YN <= pow(t,3)){
		luv.x = pow(29.0/3.0,3) * xyz.y/YN;
	} else {
		luv.x = 116.0 * pow(xyz.y/YN, 1.0/3.0) - 16.0;
	}
	float ut = 4 * xyz.x / (xyz.x + 15 * xyz.y + 3 * xyz.z);
	float vt = 9 * xyz.y / (xyz.x + 15 * xyz.y + 3 * xyz.z);
	// for standard illuminant
	float un = 0.2009;
	float vn = 0.4610;
	luv.y = 13.0 * luv.x * (ut - un);
	luv.z = 13.0 * luv.x * (vt - vn);
	return luv;
}

vec3 CIELUV2CIEXYZ(vec3 luv){
	vec3 xyz;
	// for standard illuminant
	float un = 0.2009;
	float vn = 0.4610;
	float ut = un + luv.y/(13*luv.x);
	float vt = vn + luv.z/(13*luv.x);
	if(luv.x <= 8){
		xyz.y = YN * luv.x * pow(3.0/29.0,3);
	} else {
		xyz.y = YN * pow((luv.x+16)/116,3);
	}
	xyz.x = xyz.y * 9 * ut / (4 * vt);
	xyz.z = xyz.y * (12 - 3 * ut - 20 * vt)/(4 * vt);
	return xyz;
}

vec3 rgb2CIELUV(vec3 rgb){
	return CIEXYZ2CIELUV(rgb2CIEXYZ(rgb));
}

vec3 CIELUV2rgb(vec3 luv){
	return CIEXYZ2rgb(CIELUV2CIEXYZ(luv));
}

vec3 HSV2HSL(vec3 hsv){
	vec3 hsl;
	hsl.x = hsv.x;
	hsl.z = hsv.z * (1.0-(hsv.y/2));
	if(hsl.z == 0 || hsl.z == 1){
		hsl.y = 0;
	} else {
		hsl.y = (hsv.z - hsl.z)/min(hsl.z,1.0-hsl.z);
	}
	return hsl;
}

vec3 HSL2HSV(vec3 hsl){
	vec3 hsv;
	hsv.x = hsl.x;
	hsv.z = hsl.z + hsl.y * min(hsl.z,1.0-hsl.z);
	if(hsv.z == 0){
		hsv.y = 0;
	} else {
		hsv.y = 2*(1.0-(hsl.z/hsv.z));
	}
	return hsv;
}

vec3 rgb2HSL(vec3 rgb){
	return HSV2HSL(rgb2hsv(rgb));
}

vec3 HSL2rgb(vec3 hsl){
	return hsv2rgb(HSL2HSV(hsl));
}


void main()
{
	vec3 rgb;
	if(fmode == 1){ // RGB to sRGB
		rgb = srgb2rgb(col);
	} else if(fmode == 2){ // RGB to CV
		rgb = cv2rgb(col);
	} else if(fmode == 3){ // RGB to HSV
		rgb = hsv2rgb(col);
	} else if(fmode == 4){ // RGB to HSL
		rgb = HSL2rgb(col);
	} else if(fmode == 5){ // RGB to CIEXYZ
		rgb = CIEXYZ2rgb(col);
	} else if(fmode == 6){ // RGB to CIELAB
		rgb = CIELAB2rgb(col);
	} else if(fmode == 7){ // RGB to CIELUV
		rgb = CIELUV2rgb(col);
	} else if(fmode == 8){ // RGB to CIEHLC
		rgb = CIEHLC2rgb(col);
	} else { // no conversion, just RGB
		rgb = col;
	}

	FragColor = vec4(rgb, 1.0f);
}

