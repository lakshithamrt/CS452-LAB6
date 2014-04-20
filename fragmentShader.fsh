#version 330

in vec3 fN;
in vec3 fL;

in vec3 fE;				// NEW!  Coming in from the vertex shader

in vec4 ShadowCoord;

out vec4 fColor;
uniform sampler2DShadow shadowMap;

void main () {

	vec4 color ;
	vec3 N = normalize(fN);
	vec3 L = normalize(fL);
	//vec4 basecolor = vec4(0.5,0.5,0.5,1);

	vec3 E = normalize(-fE);	// NEW!	Reverse E
	vec3 H = normalize(L + E);	// NEW! Create the half vector	

	// Diffuse component
	float diffuse_intensity = max(dot(N, L), 0.0);
	//vec4 diffuse_final = diffuse_intensity*vec4(0.1, 0.8, 0.1, 1.0);		// NEW!! Multiply the diffuse intensity times a color
	vec4 diffuse_final = diffuse_intensity*vec4(0.7, 0.2, 0.5, 1.0);

	// NEW! Specular component
	float spec_intensity = pow(max(dot(N, H), 0.0), 60);
	//vec4 spec_final = spec_intensity*vec4(0.7, 0.9, 0.7, 1.0);
	vec4 spec_final = spec_intensity*vec4(0.5, 0.5, 0.1, 1.0);
	
	color = diffuse_final + spec_final;

	float visibility = texture( shadowMap, vec3(ShadowCoord.xy, (ShadowCoord.z)/ShadowCoord.w)); 
	
	fColor = visibility * color;
	
	//fColor = color;	
}