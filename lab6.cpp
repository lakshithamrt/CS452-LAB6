#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <iostream>


#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform2.hpp"
//#include "glm/gtc/matrix_projection.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/vec3.hpp"
using namespace std;
using glm::vec3;

#define NUM_VERTICES 36
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#define PI 3.1415926

GLuint depthTexture;



GLfloat light[] = {-1.0f, 0.0f, 0.5f, 1.0f}; // NEW!


GLuint FramebufferName;
//GLuint depthTexture;
GLuint shaderProgramID, depthProgramID;
GLuint vao = 0;
GLuint vbo;
GLuint positionID, normalID, depthMatrixID, DepthBiasID, ShadowMapID, positionLoc; // NEW
GLuint lightID;
GLuint LocationMV, LocationMVP, LocationCT;
GLfloat theta;


//===========================================================================================================================

//==========================================================================================================================
#pragma region SHADER_FUNCTIONS
static char* readFile(const char* filename) {
	// Open the file
	FILE* fp = fopen (filename, "r");
	// Move the file pointer to the end of the file and determing the length
	fseek(fp, 0, SEEK_END);
	long file_length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* contents = new char[file_length+1];
	// zero out memory
	for (int i = 0; i < file_length+1; i++) {
		contents[i] = 0;
	}
	// Here's the actual read
	fread (contents, 1, file_length, fp);
	// This is how you denote the end of a string in C
	contents[file_length+1] = '\0';
	fclose(fp);
	return contents;
}

bool compiledStatus(GLint shaderID){
	GLint compiled = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compiled);
	if (compiled) {
		return true;
	}
	else {
		GLint logLength;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
		char* msgBuffer = new char[logLength];
		glGetShaderInfoLog(shaderID, logLength, NULL, msgBuffer);
		printf ("%s\n", msgBuffer);
		delete (msgBuffer);
		return false;
	}
}

GLuint makeVertexShader(const char* shaderSource) {
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource (vertexShaderID, 1, (const GLchar**)&shaderSource, NULL);
	glCompileShader(vertexShaderID);
	bool compiledCorrectly = compiledStatus(vertexShaderID);
	if (compiledCorrectly) {
		return vertexShaderID;
	}
	return -1;
}

GLuint makeFragmentShader(const char* shaderSource) {
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, (const GLchar**)&shaderSource, NULL);
	glCompileShader(fragmentShaderID);
	bool compiledCorrectly = compiledStatus(fragmentShaderID);
	if (compiledCorrectly) {
		return fragmentShaderID;
	}
	return -1;
}

GLuint makeShaderProgram (GLuint vertexShaderID, GLuint fragmentShaderID) {
	GLuint shaderID = glCreateProgram();
	glAttachShader(shaderID, vertexShaderID);
	glAttachShader(shaderID, fragmentShaderID);
	glLinkProgram(shaderID);
	return shaderID;
}
#pragma endregion SHADER_FUNCTIONS

//=========================================================================================================================


	void render() {

	//glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    
    //---------------------------------------------------------------------------------------------------------------------

    glUseProgram(depthProgramID);
    glm::vec3 lightInvDir = glm::vec3(-1.0f,0.0f,0.5f);

    // Compute the MVP matrix from the light's point of view
	glm::mat4 depthProjectionMatrix = glm::mat4(1.0f);
	//glm::mat4 depthProjectionMatrix = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,-10.0f,20.0f);
	glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));

	glm::mat4 depthModelMatrix = glm::mat4(1.0);
	glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
	


    glm::mat4 biasMatrix(
			0.5, 0.0, 0.0, 0.0, 
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0
		);

    glm::mat4 depthBiasMVP = biasMatrix*depthMVP;

	//-------------------------------------------------------------------------------
	glUseProgram(shaderProgramID);
	//theta = 0.01f;
	//scaleAmount = 0.8f; //sin(theta);

	// Important! Pass that data to the shader variables
	glm::mat4 rotate = glm::mat4(1.0f);

	glm::mat4 view = glm::lookAt(vec3(0.0f,0.0f,2.0f), vec3(0.0f,0.0f,0.0f), vec3(0.0f,1.0f,0.0f));
	glm::mat4 model = glm::mat4(1.0f);
	
	//model = glm::translate(vec3(0.0,0.0,1.0));
	model = glm::rotate( model,theta,vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 MV = view*model;
	glm::mat4 projection = glm::perspective(45.0f, (float)800/600, 1.0f, 300.0f);
	//glm::mat4 projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, 1.0f, 1000.0f);
    glm::mat4 MVP = projection*view*model;

    glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, glm::value_ptr(depthMVP));
    glUniformMatrix4fv(DepthBiasID, 1, GL_FALSE, glm::value_ptr(depthBiasMVP));
    glUniformMatrix4fv(LocationMV, 1, GL_FALSE, glm::value_ptr(MV));
	glUniformMatrix4fv(LocationMVP, 1, GL_FALSE, glm::value_ptr(MVP)); 
	glUniformMatrix4fv(LocationCT, 1, GL_FALSE, glm::value_ptr(model)); 
	glUniform4fv(lightID, 1, light);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glUniform1i(ShadowMapID, 1);

	glDrawArrays(GL_TRIANGLES, 0, NUM_VERTICES);

	glutSwapBuffers();
	glutPostRedisplay();		// This calls "render" again, allowing for animation!
}


	void changeViewport(int w, int h){
	glViewport(0, 0, w, h);
}



void keyboard(unsigned char key,int x,int y){
	switch(key){
		case'S':
		theta-=0.01f; break;
		case'D':
		theta+=0.01f; break;
		/*case'W':
		gama += 0.01f; break;
		case'A':
		gama -=0.01f; break;*/

	}
}



//==========================================================================================================================

int main (int argc, char** argv) {
	// Standard stuff...
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Particles");
	glutReshapeFunc(changeViewport);
	
	glewInit();
	

	GLfloat vertices[]= {
	-0.5f,0.5f,0.5f,  -0.5f,-0.5f,0.5f,  0.5f,-0.5f,0.5f,  -0.5f,0.5f,0.5f,  0.5f,-0.5f,0.5f,   0.5f,0.5f, 0.5f,
	 0.5f,0.5f,0.5f,   0.5f,-0.5f,0.5f,  0.5f,-0.5f,-0.5f,  0.5f,0.5f,0.5f,  0.5f,-0.5f,-0.5f,  0.5f,0.5f,-0.5f,
	 0.5f,-0.5f,0.5f, -0.5f,-0.5f,0.5f, -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,0.5f, -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,
	 0.5f,0.5f,-0.5f, -0.5f, 0.5f,-0.5f, -0.5f,0.5f,0.5f,   0.5f,0.5f,-0.5f,  -0.5f,0.5f,0.5f,   0.5f,0.5f,0.5f,
    -0.5f,-0.5f,-0.5f, -0.5f,0.5f,-0.5f,  0.5f,0.5f,-0.5f,  -0.5f,-0.5f,-0.5f, 0.5f,0.5f,-0.5f,  0.5f,-0.5f,-0.5f,
	-0.5f,0.5f,-0.5f,  -0.5f,-0.5f,-0.5f, -0.5f,-0.5f,0.5f, -0.5f,0.5f,-0.5f, -0.5f,-0.5f,0.5f,  -0.5f,0.5f,0.5f,


	};


	GLfloat normals[]={
	0.0f,0.0f,1.0f,  0.0f,0.0f,1.0f,  0.0f,0.0f,1.0f,  0.0f,0.0f,1.0f,  0.0f,0.0f,1.0f,  0.0f,0.0f,1.0f,
	1.0f,0.0f,0.0f,  1.0f,0.0f,0.0f,  1.0f,0.0f,0.0f,  1.0f,0.0f,0.0f,  1.0f,0.0f,0.0f,  1.0f,0.0f,0.0f,
	0.0f,-1.0f,0.0f, 0.0f,-1.0f,0.0f, 0.0f,-1.0f,0.0f, 0.0f,-1.0f,0.0f, 0.0f,-1.0f,0.0f, 0.0f,-1.0f,0.0f,
	0.0f,1.0f,0.0f,  0.0f,1.0f,0.0f,  0.0f,1.0f,0.0f,  0.0f,1.0f,0.0f,  0.0f,1.0f,0.0f,  0.0f,1.0f,0.0f,
	0.0f,0.0f,-1.0f, 0.0f,0.0f,-1.0f, 0.0f,0.0f,-1.0f, 0.0f,0.0f,-1.0f,  0.0f,0.0f,-1.0f, 0.0f,0.0f,-1.0f,
	-1.0f,0.0f,0.0f, -1.0f,0.0f,0.0f, -1.0f,0.0f,0.0f, -1.0f,0.0f,0.0f, -1.0f,0.0f,0.0f, -1.0f,0.0f,0.0f,
};
	
	// Make a shader
	char* vertexShaderSourceCode2 = readFile("DepthRTTv.glsl");
	char* fragmentShaderSourceCode2 = readFile("DepthRTTf.glsl");
	GLuint vertShaderID2 = makeVertexShader(vertexShaderSourceCode2);
	GLuint fragShaderID2 = makeFragmentShader(fragmentShaderSourceCode2);
	depthProgramID = makeShaderProgram(vertShaderID2, fragShaderID2);

	// Get a handle for our "MVP" uniform
	depthMatrixID = glGetUniformLocation(depthProgramID, "depthMVP");
	positionLoc = glGetAttribLocation(depthProgramID, "vertexPosition_modelspace");

 
 //============================ New code for shadow mapping ========================================================================


	
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT24, 1024, 1024, 0,GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		 
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		cout<<"FBO is created"<<endl; 

	glBindFramebufferEXT(GL_FRAMEBUFFER,0);

    
	// Make a shader
	char* vertexShaderSourceCode = readFile("vertexShader.vsh");
	char* fragmentShaderSourceCode = readFile("fragmentShader.fsh");
	GLuint vertShaderID = makeVertexShader(vertexShaderSourceCode);
	GLuint fragShaderID = makeFragmentShader(fragmentShaderSourceCode);
	shaderProgramID = makeShaderProgram(vertShaderID, fragShaderID);

	GLuint TextureID  = glGetUniformLocation(shaderProgramID, "myTextureSampler");


	DepthBiasID = glGetUniformLocation(shaderProgramID, "DepthBiasMVP");
	ShadowMapID = glGetUniformLocation(shaderProgramID, "shadowMap");


	// Create the "remember all"
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// Create the buffer, but don't load anything yet
	//glBufferData(GL_ARRAY_BUFFER, 7*NUM_VERTICES*sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, 9*NUM_VERTICES*sizeof(GLfloat), NULL, GL_STATIC_DRAW);		// NEW!! - We're only loading vertices and normals (6 elements, not 7)
	
	// Load the vertex points
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3*NUM_VERTICES*sizeof(GLfloat), vertices);
	// Load the colors right after that
	//glBufferSubData(GL_ARRAY_BUFFER, 3*NUM_VERTICES*sizeof(GLfloat),4*NUM_VERTICES*sizeof(GLfloat), colors);
	glBufferSubData(GL_ARRAY_BUFFER, 3*NUM_VERTICES*sizeof(GLfloat),3*NUM_VERTICES*sizeof(GLfloat), normals);
	glBufferSubData(GL_ARRAY_BUFFER, 6*NUM_VERTICES*sizeof(GLfloat),3*NUM_VERTICES*sizeof(GLfloat), vertices);

	// ============ New! glUniformLocation is how you pull IDs for uniform variables===============
	LocationMVP = glGetUniformLocation(shaderProgramID, "MVP");
	LocationMV = glGetUniformLocation(shaderProgramID, "MV");
	LocationCT = glGetUniformLocation(shaderProgramID, "CT");
	positionID = glGetAttribLocation(shaderProgramID, "s_vPosition");
	normalID = glGetAttribLocation(shaderProgramID, "s_vNormal");
	lightID = glGetUniformLocation(shaderProgramID, "vLight");	// NEW
	//lightID1 = glGetUniformLocation(shaderProgramID, "vLight1");	// NEW

	//=============================================================================================

	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices)));
	glVertexAttribPointer(normalID, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices)));
	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices)+sizeof(normals)));
	
	glUseProgram(shaderProgramID);
	glEnableVertexAttribArray(positionID);
	glEnableVertexAttribArray(normalID);
	glUseProgram(depthProgramID);
	glEnableVertexAttribArray(positionLoc);


	
	glEnable(GL_CULL_FACE);  // NEW! - we're doing real 3D now...  Cull (don't render) the backsides of triangles
	glCullFace(GL_BACK);	// Other options?  GL_FRONT and GL_FRONT_AND_BACK
	glEnable(GL_DEPTH_TEST);// Make sure the depth buffer is on.  As you draw a pixel, update the screen only if it's closer than previous ones


	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glutDisplayFunc(render);
	glutKeyboardFunc( keyboard );
	//glutIdleFunc(idler);
	glutMainLoop();
	
	return 0;


}
