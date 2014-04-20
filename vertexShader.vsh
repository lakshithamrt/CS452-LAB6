#version 330

in vec4 s_vPosition;	// Vertex in local coordinate system
in vec4 s_vNormal;		// Normal of this vertex

uniform mat4 MV;		// The matrix to convert into the world coordinate system
uniform mat4 MVP;		// The matrix to convert into the camera coordinate system
uniform mat4 CT;		// The perspective matrix for depth

uniform vec4 vLight;	   // The Light position
uniform mat4 DepthBiasMVP;


out vec3 fN;			// Because these are "out", they go to the fragment shader
out vec3 fL;			// as interpolated values (i.e. the pixel will have an interpolated normal)

out vec3 fE;			// NEW!! The vector between the camera and a pixel
out vec4 ShadowCoord;

void main () {
	
	fN = (CT*s_vNormal).xyz;	// Rotate the normal! only take the first 3 parts, since fN is a vec3
	fL = (vLight).xyz;					// Same here.  Note: if we're rotating the camera, we'd need to rotate the light direction as well.

	fE = (MV*s_vPosition).xyz;			// NEW!! where the pixel is relative to the camera. 
	
	// New way using matrix multiplication.  From local, to world, to camera, to NDCs
	gl_Position = MVP*s_vPosition;

	ShadowCoord = DepthBiasMVP * vec4(s_vPosition);

	

}