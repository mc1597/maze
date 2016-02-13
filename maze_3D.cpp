#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>


using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;
	GLuint TextureBuffer;
	GLuint TextureID;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
	GLuint TexMatrixID;
} Matrices;

GLuint programID, textureProgramID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

struct VAO* create3DTexturedObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* texture_buffer_data, GLuint textureID, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;
	vao->TextureID = textureID;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->TextureBuffer));  // VBO - textures

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->TextureBuffer); // Bind the VBO textures
	glBufferData (GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), texture_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			2,                  // attribute 2. Textures
			2,                  // size (s,t)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}


void draw3DTexturedObject (struct VAO* vao){
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Bind Textures using texture units
	glBindTexture(GL_TEXTURE_2D, vao->TextureID);

	// Enable Vertex Attribute 2 - Texture
	glEnableVertexAttribArray(2);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->TextureBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle

	// Unbind Textures to be safe
	glBindTexture(GL_TEXTURE_2D, 0);
}

/* Create an OpenGL Texture from an image */
GLuint createTexture (const char* filename){
	GLuint TextureID;
	// Generate Texture Buffer
	glGenTextures(1, &TextureID);
	// All upcoming GL_TEXTURE_2D operations now have effect on our texture buffer
	glBindTexture(GL_TEXTURE_2D, TextureID);
	// Set our texture parameters
	// Set texture wrapping to GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering (interpolation)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load image and create OpenGL texture
	int twidth, theight;
	unsigned char* image = SOIL_load_image(filename, &twidth, &theight, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D); // Generate MipMaps to use
	SOIL_free_image_data(image); // Free the data read from file after creating opengl texture
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess it up

	return TextureID;
}




float camera_rotation_angle = 90;
//glm::vec3 eye (7, 6, 12);
glm::vec3 eye (8, 8, 11);
glm::vec3 target (4, 2, 1);
glm::vec3 up (0, 1, 0);

glm::vec3 eye1 (4, 10, 5);
glm::vec3 target1 (4, 2, 1);
glm::vec3 up1 (0, 1, 0);

glm::vec3 eye2 (0, 1.5, 0.5);
glm::vec3 target2 (0, 1.5, 2);
glm::vec3 up2 (0, 1, 0);

glm::vec3 eye3 (0, 2.5, -1.5);
glm::vec3 target3 (0, 1.5, 3);
glm::vec3 up3 (0, 1, 0);

glm::vec3 eye4 (8, 8, 11);
glm::vec3 target4 (4, 2, 1);
glm::vec3 up4 (0, 1, 0);
//glm::vec3 eye (2, 3, 7 * sin(camera_rotation_angle * M_PI/180.0f));
// Target - Where is the camera looking at.  Don't change unless you are sure!!
//glm::vec3 target (1, 1, 1);
// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
//glm::vec3 up (0, 1, 0);
int view=0;
int choice=0;
float deltaTime=0;
bool jump=false;
bool now=false;
float eye4x = 8,eye4y = 8, eye4z = 11;
float target4x = 4,target4y = 8, target4z = 11;
class Background{
	public:

		VAO *axis;
		void createAxes(){

			static const GLfloat vertex_buffer_data [] = {
				-7,0,0,
				7,0,0,

				0,-7,0,
				0,7,0,

				0,0,-7,
				0,0,7,
			};

			static const GLfloat color_buffer_data [] = {
				1,0,0,
				1,0,0,

				0,1,0,
				0,1,0,

				0,0,1,
				0,0,1,
			};
			axis = create3DObject(GL_LINES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
		}

		void clean1(){

			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram (programID);
			glUseProgram(textureProgramID);
		}
		void draw(){
			glUseProgram (programID);
			if(view==3)
				Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			else if(view==4)
				Matrices.view = glm::lookAt( eye1, target1, up1 ); // Rotating Camera for 3D
			else if(view==1)
				Matrices.view = glm::lookAt( eye2, target2, up2 ); // Rotating Camera for 3D
			else if(view==2)
				Matrices.view = glm::lookAt( eye3, target3, up3 ); // Rotating Camera for 3D
			else if(view==0)
				Matrices.view = glm::lookAt( eye4, target4, up4 ); // Rotating Camera for 3D
			else
				Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;	// MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			//draw3DObject(axis);

		}


};

Background bg;

class Heart{

	public:
		VAO *hrt[3];
		float posx;
		float posy;
		float radius;

		Heart(){
			posx = 3.5;
			posy = 3.45;	
			radius = 0.062;
		}

		void createTriangle(int index){
			static const GLfloat vertex_buffer_data [] = {
				0,-0.062,0,
				-0.125,0.125,0,
				0.125,0.125,0,	
			};

			static const GLfloat color_buffer_data [] = {
				1,0,0, // color 0
				1,0,0, // color 0
				1,0,0, // color 0

			};
			hrt[index] = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);


		}

		void createLeft(int index){
			int numVertices = 190;
			GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
			for (int i=0; i<numVertices; i++) {
				vertex_buffer_data [3*i] = -0.062 + radius*cos(i*M_PI/180.0f);
				vertex_buffer_data [3*i + 1] = 0.125 + radius*sin(i*M_PI/180.0f);
				vertex_buffer_data [3*i + 2] = 0;
			}


			GLfloat* color_buffer_data = new GLfloat [3*numVertices];
			for (int i=0; i<numVertices; i++) {
				color_buffer_data [3*i] = 1;
				color_buffer_data [3*i + 1] = 0;
				color_buffer_data [3*i + 2] = 0;
			}


			// create3DObject creates and returns a handle to a VAO that can be used later
			hrt[index] = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

		}

		void createRight(int index){
			int numVertices = 190;
			GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
			for (int i=0; i<numVertices; i++) {
				vertex_buffer_data [3*i] = 0.062 + radius*cos(i*M_PI/180.0f);
				vertex_buffer_data [3*i + 1] = 0.125 + radius*sin(i*M_PI/180.0f);
				vertex_buffer_data [3*i + 2] = 0;
			}


			GLfloat* color_buffer_data = new GLfloat [3*numVertices];
			for (int i=0; i<numVertices; i++) {
				color_buffer_data [3*i] = 1;
				color_buffer_data [3*i + 1] = 0;
				color_buffer_data [3*i + 2] = 0;
			}


			// create3DObject creates and returns a handle to a VAO that can be used later
			hrt[index] = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

		}

		void draw(int index){
			glUseProgram (programID);
			Matrices.view = glm::lookAt(glm::vec3(1,3,4),glm::vec3(1,3,0),up);
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;  // MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);

			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateLt = glm::translate (glm::vec3(posx, posy, 0));
			glm::mat4 scaleLt = glm::scale(glm::vec3(2, 2, 0));
			Matrices.model *= (translateLt*scaleLt);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(hrt[index]);


		}


};

Heart heart[4];

class Bar{

	public:
		VAO *htb;
		float posx;
		float posy;
		float posz;
		Bar(){
			posx = 0;
			posy = 0;
			posz = 0;
		}
		void create(int i){
			if(i<5){
				static const GLfloat vertex_buffer_data [] = {
					1,1,0,
					-1,1,0,
					-1,-1,0,

					-1,-1,0,
					1,-1,0,
					1,1,0,
				};

				static const GLfloat color_buffer_data [] = {
					1,0,0, // color 0
					1,0,0, // color 0
					1,0,0, // color 0

					1,0,0, // color 0
					1,0,0, // color 0
					1,0,0, // color 0
				};
				htb = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
			}
			else{
				static const GLfloat vertex_buffer_data [] = {
					1,1,0,
					-1,1,0,
					-1,-1,0,

					-1,-1,0,
					1,-1,0,
					1,1,0,
				};

				static const GLfloat color_buffer_data [] = {
					0,1,0, // color 0
					0,1,0, // color 0
					0,1,0, // color 0

					0,1,0, // color 0
					0,1,0, // color 0
					0,1,0, // color 0
				};
				htb = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

			}

		}

		void draw(float posx,float posy,float scalex,float scaley){

			glUseProgram (programID);
			Matrices.view = glm::lookAt(glm::vec3(1,3,4),glm::vec3(1,3,0),up);
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;  // MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);

			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateHtb = glm::translate (glm::vec3(posx, posy, 0));
			glm::mat4 scaleHtb = glm::scale(glm::vec3(scalex, scaley, 0));
			Matrices.model *= (translateHtb*scaleHtb);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(htb);

		}

};

Bar bar[10];

class Brick{
	public:
		VAO *br,*gif[16],*top[2],*left[2],*right[2],*back[2],*bottom[2];
		float posx;
		float posy;
		float posz;
		float center[3];
		float radius;
		bool isThere;
		bool isMove;
		int index1;
		int index2;
		int count;
		int dir;
		Brick(){
			posx = 0;
			posy = 0;
			posz = 0;
			center[0] = posx;
			center[1] = posy;
			center[2] = posz;
			radius = 1;
			isThere=true;
			isMove=false;
			index1=0;
			index2=0;
			count=0;
			dir=1;
		}
		void create(){

			static const GLfloat vertex_buffer_data [] = {
				-0.5f,-1,-0.5f, // triangle 1 : begin
				-0.5f,-1, 0.5f,
				-0.5f, 1, 0.5f, // triangle 1 : end

				-0.5f, 1, 0.5f, // triangle 1 : begin
				-0.5f, 1,-0.5f,
				-0.5f,-1,-0.5f, // triangle 1 : end

				0.5f, 1, 0.5f,
				-0.5f,-1,-0.5f,
				-0.5f, 1,-0.5f,

				-0.5f, 1,-0.5f,
				0.5f,-1,-0.5f, 
				0.5f, 1, 0.5f,

				0.5f, 1,-0.5f,
				0.5f,-1,-0.5f, 
				0.5f,-1, 0.5f,

				0.5f,-1, 0.5f,
				0.5f, 1, 0.5f,	
				0.5f, 1,-0.5f,

				0.5f, 1, 0.5f,
				0.5f,-1, 0.5f,	
				-0.5f,-1, 0.5f,

				-0.5f,-1, 0.5f,
				-0.5f, 1, 0.5f,
				0.5f, 1, 0.5f,

				0.5f, 1, 0.5f,
				-0.5f, 1, 0.5f,
				-0.5f, 1,-0.5f,

				-0.5f, 1,-0.5f,
				0.5f, 1,-0.5f,
				0.5f, 1, 0.5f,

				0.5f,-1, 0.5f,
				-0.5f,-1, 0.5f,
				-0.5f,-1,-0.5f,

				-0.5f,-1,-0.5f,
				0.5f,-1,-0.5f,
				0.5f,-1, 0.5f,

			};

			static const GLfloat color_buffer_data [] = {
				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3

				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3

				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3

				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3

				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3

				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3

				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3

				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3

				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3

				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3

				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3

				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3
				0.2,0.2,1, // color 3

			};

			br = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);

		}


		void createFront(GLuint textureID,int i){
			static const GLfloat vertex_buffer_data [] = {
				0.5f, 1, 0.5f,
				0.5f,-1, 0.5f,	
				-0.5f,-1, 0.5f,

				-0.5f,-1, 0.5f,
				-0.5f, 1, 0.5f,
				0.5f, 1, 0.5f,
			};
			static const GLfloat color_buffer_data [] = {
				1,0,0, // color 1
				0,0,1, // color 2
				0,1,0, // color 3

				0,1,0, // color 3
				0.3,0.3,0.3, // color 4
				1,0,0  // color 1
			};

			// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
			static const GLfloat texture_buffer_data [] = {
				0,1, // TexCoord 1 - bot left
				1,1, // TexCoord 2 - bot right
				1,0, // TexCoord 3 - top right

				1,0, // TexCoord 3 - top right
				0,0, // TexCoord 4 - top left
				0,1  // TexCoord 1 - bot left
			};

			// create3DTexturedObject creates and returns a handle to a VAO that can be used later
			gif[i] = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);

		}

		void createUp(GLuint textureID,int i){
			static const GLfloat vertex_buffer_data [] = {
				0.5f, 1, 0.5f,
				-0.5f, 1, 0.5f,
				-0.5f, 1,-0.5f,

				-0.5f, 1,-0.5f,
				0.5f, 1,-0.5f,
				0.5f, 1, 0.5f,
			};
			static const GLfloat color_buffer_data [] = {
				1,0,0, // color 1
				0,0,1, // color 2
				0,1,0, // color 3

				0,1,0, // color 3
				0.3,0.3,0.3, // color 4
				1,0,0  // color 1
			};

			// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
			static const GLfloat texture_buffer_data [] = {
				0,1, // TexCoord 1 - bot left
				1,1, // TexCoord 2 - bot right
				1,0, // TexCoord 3 - top right

				1,0, // TexCoord 3 - top right
				0,0, // TexCoord 4 - top left
				0,1  // TexCoord 1 - bot left
			};

			// create3DTexturedObject creates and returns a handle to a VAO that can be used later
			top[i] = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);

		}

		void createDown(GLuint textureID,int i){
			static const GLfloat vertex_buffer_data [] = {
				0.5f, -1, 0.5f,
				-0.5f, -1, 0.5f,
				-0.5f, -1,-0.5f,

				-0.5f, -1,-0.5f,
				0.5f, -1,-0.5f,
				0.5f, -1, 0.5f,
			};
			static const GLfloat color_buffer_data [] = {
				1,0,0, // color 1
				0,0,1, // color 2
				0,1,0, // color 3

				0,1,0, // color 3
				0.3,0.3,0.3, // color 4
				1,0,0  // color 1
			};

			// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
			static const GLfloat texture_buffer_data [] = {
				0,1, // TexCoord 1 - bot left
				1,1, // TexCoord 2 - bot right
				1,0, // TexCoord 3 - top right

				1,0, // TexCoord 3 - top right
				0,0, // TexCoord 4 - top left
				0,1  // TexCoord 1 - bot left
			};

			// create3DTexturedObject creates and returns a handle to a VAO that can be used later
			bottom[i] = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);

		}


		void createBack(GLuint textureID,int i){
			static const GLfloat vertex_buffer_data [] = {
				-0.5f, -1, -0.5f,
				0.5f,-1, -0.5f,
				0.5f,1, -0.5f,

				0.5f,1, -0.5f,
				-0.5f, 1, -0.5f,
				-0.5f, -1, -0.5f,

			};
			static const GLfloat color_buffer_data [] = {
				1,0,0, // color 1
				0,0,1, // color 2
				0,1,0, // color 3

				0,1,0, // color 3
				0.3,0.3,0.3, // color 4
				1,0,0  // color 1
			};

			// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
			static const GLfloat texture_buffer_data [] = {
				0,1, // TexCoord 1 - bot left
				1,1, // TexCoord 2 - bot right
				1,0, // TexCoord 3 - top right

				1,0, // TexCoord 3 - top right
				0,0, // TexCoord 4 - top left
				0,1  // TexCoord 1 - bot left
			};

			// create3DTexturedObject creates and returns a handle to a VAO that can be used later
			back[i] = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);

		}

		void createLeft(GLuint textureID,int i){
			static const GLfloat vertex_buffer_data [] = {
				-0.5f,-1,-0.5f, // triangle 1 : begin
				-0.5f,-1, 0.5f,
				-0.5f, 1, 0.5f, // triangle 1 : end

				-0.5f, 1, 0.5f, // triangle 1 : begin
				-0.5f, 1,-0.5f,
				-0.5f,-1,-0.5f, // triangle 1 : end
			};
			static const GLfloat color_buffer_data [] = {
				1,0,0, // color 1
				0,0,1, // color 2
				0,1,0, // color 3

				0,1,0, // color 3
				0.3,0.3,0.3, // color 4
				1,0,0  // color 1
			};

			// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
			static const GLfloat texture_buffer_data [] = {
				0,1, // TexCoord 1 - bot left
				1,1, // TexCoord 2 - bot right
				1,0, // TexCoord 3 - top right

				1,0, // TexCoord 3 - top right
				0,0, // TexCoord 4 - top left
				0,1  // TexCoord 1 - bot left
			};

			// create3DTexturedObject creates and returns a handle to a VAO that can be used later
			left[i] = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);

		}
		void createRight(GLuint textureID,int i){
			static const GLfloat vertex_buffer_data [] = {
				0.5f, 1,-0.5f,
				0.5f,-1,-0.5f, 
				0.5f,-1, 0.5f,

				0.5f,-1, 0.5f,
				0.5f, 1, 0.5f,	
				0.5f, 1,-0.5f,
			};
			static const GLfloat color_buffer_data [] = {
				1,0,0, // color 1
				0,0,1, // color 2
				0,1,0, // color 3

				0,1,0, // color 3
				0.3,0.3,0.3, // color 4
				1,0,0  // color 1
			};

			// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
			static const GLfloat texture_buffer_data [] = {
				0,1, // TexCoord 1 - bot left
				1,1, // TexCoord 2 - bot right
				1,0, // TexCoord 3 - top right

				1,0, // TexCoord 3 - top right
				0,0, // TexCoord 4 - top left
				0,1  // TexCoord 1 - bot left
			};

			// create3DTexturedObject creates and returns a handle to a VAO that can be used later
			right[i] = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);

		}
		void draw(float i,float j){
			// Compute Camera matrix (view)
			glUseProgram (programID);
			if(view==3)
				Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			else if(view == 4)
				Matrices.view = glm::lookAt( eye1, target1, up1 ); // Rotating Camera for 3D
			else if(view==1)
				Matrices.view = glm::lookAt( eye2, target2, up2 ); // Rotating Camera for 3D
			else if(view==2)
				Matrices.view = glm::lookAt( eye3, target3, up3 ); // Rotating Camera for 3D
			else if(view==0)
				Matrices.view = glm::lookAt( eye4, target4, up4 ); // Rotating Camera for 3D
			else
				Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			//Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;	// MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);
			posz = i;
			posx = j;
			if(isMove){
				posy-=dir*(0.02 + i*0.002 + j*0.002);
				if(posy < -2.75 || posy > 2.55)
					dir = -1*dir;
			}
			glm::mat4 translateBrick = glm::translate(glm::vec3(posx, posy, posz));   
			Matrices.model *= translateBrick;
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(br);
		}

		void drawGif(float i,float j){
			glUseProgram(textureProgramID);
			if(view == 3)
				Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			else if(view == 4)
				Matrices.view = glm::lookAt( eye1, target1, up1 ); // Rotating Camera for 3D
			else if(view==1)
				Matrices.view = glm::lookAt( eye2, target2, up2 ); // Rotating Camera for 3D
			else if(view==2)
				Matrices.view = glm::lookAt( eye3, target3, up3 ); // Rotating Camera for 3D
			else if(view==0)
				Matrices.view = glm::lookAt( eye4, target4, up4 ); // Rotating Camera for 3D
			else
				Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			//Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;	// MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);
			posz = i;
			posx = j;
			glm::mat4 translateBrick = glm::translate(glm::vec3(posx, posy, posz));   
			Matrices.model *= translateBrick;
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			if(index1<16)
				draw3DTexturedObject(gif[index1]);
			else
				draw3DTexturedObject(gif[31-index1]);

			draw3DTexturedObject(top[index2]);
			draw3DTexturedObject(right[index2]);
			draw3DTexturedObject(left[index2]);
			draw3DTexturedObject(back[index2]);
			draw3DTexturedObject(bottom[index2]);


		}
};

Brick brick[100];
class Light{

	public:
		VAO *li;
		float posx;
		float posy;
		float posz;
		float center[3];
		float radius;
		bool show;
		bool pause;
		float angle;

		Light(){
			posx=4;
			posy=2;
			posz=8;
			radius=0.2;
			center[0] = posx;
			center[1] = posy;
			center[2] = posz;
			show = true;
			pause = false;
			angle = 0;
		}

		void create()
		{

			int numVertices = 360;
			GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
			for (int i=0; i<numVertices; i++) {
				vertex_buffer_data [3*i] = radius*cos(i*M_PI/180.0f);
				vertex_buffer_data [3*i + 1] = radius*sin(i*M_PI/180.0f);
				vertex_buffer_data [3*i + 2] = 0;
			}


			GLfloat* color_buffer_data = new GLfloat [3*numVertices];
			for (int i=0; i<numVertices; i++) {
				color_buffer_data [3*i] = 1;
				color_buffer_data [3*i + 1] = 1;
				color_buffer_data [3*i + 2] = 0;
			}


			li = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);
		}

		void draw(){
			glUseProgram (programID);
			if(view==3)
				Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			else if(view==4)
				Matrices.view = glm::lookAt( eye1, target1, up1 ); // Rotating Camera for 3D
			else if(view==1)
				Matrices.view = glm::lookAt( eye2, target2, up2 ); // Rotating Camera for 3D
			else if(view==2)
				Matrices.view = glm::lookAt( eye3, target3, up3 ); // Rotating Camera for 3D
			else if(view==0)
				Matrices.view = glm::lookAt( eye4, target4, up4 ); // Rotating Camera for 3D
			else
				Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			//Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;  // MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);
			glm::mat4 moveLt = glm::translate(glm::vec3(posx,posy,posz));
			glm::mat4 rotateLt = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,1,0));
			center[0] = posx;
			center[1] = posy;
			center[2] = posz;
			angle+=2;
			Matrices.model *= moveLt*rotateLt;
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(li);

		}
};

Light light[6];

class Obstacle{

	public:
		VAO *sp;
		float posx;
		float posy;
		float posz;
		float radius;
		float center[3];
		float angle;
		int count;
		int dir;
		float speed;
		Obstacle(){
			posx=0;
			posy=0;
			posz=0;
			radius=0.5;
			center[0]=posx;
			center[1]=posy;
			center[2]=posz;
			speed=0.07;
			angle=0;
			count=0;
			dir=1;
		}

		void create(int slices,int stacks){
			int n = 2 * (slices + 1) * stacks;
			int i = 0;
			GLfloat *points = new GLfloat[3*n];
			GLfloat *color = new GLfloat[3*n];
			for (float theta = -M_PI / 2; theta < M_PI / 2 - 0.0001; theta += M_PI / stacks) {
				for (float phi = -M_PI; phi <= M_PI + 0.0001; phi += 2 * M_PI / slices) {

					points[3*i] = radius*(cos(theta) * sin(phi));
					points[3*i + 1] = radius*(-sin(theta));
					points[3*i + 2] = radius*(cos(theta) * cos(phi));

					color[3*i] = 1;
					color[3*i + 1] = 0;
					color[3*i + 2] = 0;

					i++;

					points[3*i] = radius*(cos(theta + M_PI / stacks) * sin(phi));
					points[3*i + 1] = radius*(-sin(theta + M_PI / stacks));
					points[3*i + 2] = radius*(cos(theta + M_PI / stacks) * cos(phi));

					color[3*i] = 0.8;
					color[3*i + 1] = 0.8;
					color[3*i + 2] = 0.8;

					i++;
				}
			}

			sp = create3DObject(GL_TRIANGLE_STRIP, n, points, color, GL_FILL);

		}

		void draw(){

			glUseProgram (programID);
			if(view==3)
				Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			else if(view == 4)
				Matrices.view = glm::lookAt( eye1, target1, up1 ); // Rotating Camera for 3D
			else if(view==1)
				Matrices.view = glm::lookAt( eye2, target2, up2 ); // Rotating Camera for 3D
			else if(view==2)
				Matrices.view = glm::lookAt( eye3, target3, up3 ); // Rotating Camera for 3D
			else if(view==0)
				Matrices.view = glm::lookAt( eye4, target4, up4 ); // Rotating Camera for 3D
			else
				Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			//Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;  // MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);
			glm::mat4 moveSp = glm::translate(glm::vec3(posx,posy,posz));
			center[0] = posx;
			center[1] = posy;
			center[2] = posz;
			Matrices.model *= moveSp;
			angle+=2;
			count++;
			posy += dir*speed;
			if(center[1] < 2){
				dir = 1;
				center[1] = 2.1;
			}
			if(center[1] > 4){
				dir = -1;
				center[1] = 3.8;
			}
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(sp);

		}
};

Obstacle obstacle[3];

class Can{

	public:
		VAO *sh,*hs,*straw,*umb;
		float posx;
		float posy;
		float posz;
		float radius;
		bool show;
		float center[3];
		float angle;
		Can(){
			posx=5;
			posy=3;
			posz=8;
			radius=0.5;
			center[0]=posx;
			center[1]=posy;
			center[2]=posz;
			show = true;
			angle=0;
		}

		void create(){
			float factor=0.7;
			int numVertices = 360,i;
			float height=0.0007;
			int j;
			GLfloat* vertex_buffer_data = new GLfloat [3*numVertices*1500];
			for(j=0;j<1500;j++){
				for (i=0; i<numVertices; i++) {
					vertex_buffer_data [3*numVertices*j + 3*i] = factor*radius*cos(i*M_PI/180.0f);
					vertex_buffer_data [3*numVertices*j + 3*i + 1] = height*j;
					vertex_buffer_data [3*numVertices*j + 3*i + 2] = factor*radius*sin(i*M_PI/180.0f);
				}
				factor+=0.0003;

			}

			GLfloat* color_buffer_data = new GLfloat [3*numVertices*1500];

			for(j=0;j<1500;j++){
				for (i=0; i<numVertices; i++){
					if(j>=1493&&j<=1499){
						color_buffer_data [3*numVertices*j + 3*i] = 0;
						color_buffer_data [3*numVertices*j + 3*i + 1] = 0.78;
						color_buffer_data [3*numVertices*j + 3*i + 2] = 0.9;
					}
					else{
						color_buffer_data [3*numVertices*j + 3*i] = 0.01;
						color_buffer_data [3*numVertices*j + 3*i + 1] = 0.13;
						color_buffer_data [3*numVertices*j + 3*i + 2] = 0.4;
					}
				}
			}

			sh = create3DObject(GL_TRIANGLE_FAN, 1500*numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

		}

		void createStraw(){
			float factor=0.1;
			int numVertices = 360,i;
			float height=0.001;
			int j;
			GLfloat* vertex_buffer_data = new GLfloat [3*numVertices*1500];
			for(j=0;j<1500;j++){
				for (i=0; i<numVertices; i++) {
					vertex_buffer_data [3*numVertices*j + 3*i] = factor*radius*cos(i*M_PI/180.0f);
					vertex_buffer_data [3*numVertices*j + 3*i + 1] = height*j;
					vertex_buffer_data [3*numVertices*j + 3*i + 2] = factor*radius*sin(i*M_PI/180.0f);
				}

			}

			GLfloat* color_buffer_data = new GLfloat [3*numVertices*1500];

			for(j=0;j<1500;j++){
				for (i=0; i<numVertices; i++){
					color_buffer_data [3*numVertices*j + 3*i] = 1;
					color_buffer_data [3*numVertices*j + 3*i + 1] = 1;
					color_buffer_data [3*numVertices*j + 3*i + 2] = 1;
				}
			}

			straw = create3DObject(GL_TRIANGLE_FAN, 1500*numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

		}

		void createUmb(int slices,int stacks){
			int n = 2 * (slices + 1) * stacks;
			int i = 0;
			GLfloat *points = new GLfloat[3*n];
			GLfloat *color = new GLfloat[3*n];
			for (float theta = -M_PI / 2; theta < M_PI/2 - 0.0001; theta += M_PI / stacks) {
				for (float phi = -M_PI; phi <= 0.0001; phi += M_PI / slices) {

					points[3*i] = radius*(cos(theta) * sin(phi));
					points[3*i + 1] = radius*(-sin(theta));
					points[3*i + 2] = radius*(cos(theta) * cos(phi));

					color[3*i] = 1;
					color[3*i + 1] = 0.2;
					color[3*i + 2] = 0.6;

					i++;

					points[3*i] = radius*(cos(theta + M_PI / stacks) * sin(phi));
					points[3*i + 1] = radius*(-sin(theta + M_PI / stacks));
					points[3*i + 2] = radius*(cos(theta + M_PI / stacks) * cos(phi));

					color[3*i] = 1;
					color[3*i + 1] = 0.2;
					color[3*i + 2] = 0.6;

					i++;
				}
			}

			umb = create3DObject(GL_TRIANGLE_STRIP, n, points, color, GL_FILL);

		}


		void draw(){

			glUseProgram (programID);
			if(view==3)
				Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			else if(view==4)
				Matrices.view = glm::lookAt( eye1, target1, up1 ); // Rotating Camera for 3D
			else if(view==1)
				Matrices.view = glm::lookAt( eye2, target2, up2 ); // Rotating Camera for 3D
			else if(view==2)
				Matrices.view = glm::lookAt( eye3, target3, up3 ); // Rotating Camera for 3D
			else if(view==0)
				Matrices.view = glm::lookAt( eye4, target4, up4 ); // Rotating Camera for 3D
			else
				Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			//Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;  // MVP = Projection * View * Model
			angle=15;
			Matrices.model = glm::mat4(1.0f);
			glm::mat4 moveSt = glm::translate(glm::vec3(posx-0.15,posy,posz));
			glm::mat4 rotateSt = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,0,1));
			center[0] = posx;
			center[1] = posy;
			center[2] = posz;
			Matrices.model *= (moveSt*rotateSt);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(straw);

			Matrices.model = glm::mat4(1.0f);
			glm::mat4 moveUmb = glm::translate(glm::vec3(0.1,1.35,0));
			angle=-90;
			glm::mat4 rotateUmb1 = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,0,1));
			angle=20;
			glm::mat4 rotateUmb2 = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,1,0));
			Matrices.model *= (moveSt * rotateSt * moveUmb * rotateUmb1 * rotateUmb2);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(umb);

			Matrices.model = glm::mat4(1.0f);
			glm::mat4 moveSh = glm::translate(glm::vec3(posx,posy,posz));
			//glm::mat4 rotateSh = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,1,0));
			Matrices.model *= (moveSh);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(sh);

		}
};

Can can;

int count=0;
class Person{
	public:
		VAO *per,*limb[4],*head;
		float posx;
		float posy;
		float posz;
		bool isMoving;
		bool levitate;
		bool onMTile;
		bool onMTileJump;
		bool move1;
		int dir;
		float t;
		int hitno;
		float center[3];
		float vel;
		float radius;
		int lives;
		int coins;
		int score;
		int speed;
		float beforeht;
		float beforeht1;
		Person(){
			isMoving=false;
			onMTile=false;
			onMTileJump=false;
			move1=false;
			posx=0;
			posy=2.5;
			posz=0;
			beforeht = posy;
			beforeht1 = posy;
			dir=0;
			t=0;
			coins=0;
			hitno=0;
			vel=1;
			levitate = false;
			center[0] = posx;
			center[1] = posy;
			center[2] = posz;
			lives = 3;
			score = 0;
			radius = sqrt(3)/2;
			speed = 10;
		}

		void create(){
			static const GLfloat vertex_buffer_data [] = {
				-0.5f,-0.5f,-0.5f, // triangle 1 : begin
				-0.5f,-0.5f, 0.5f,
				-0.5f, 0.5f, 0.5f, // triangle 1 : end

				0.5f, 0.5f,-0.5f, // triangle 2 : begin
				-0.5f,-0.5f,-0.5f,
				-0.5f, 0.5f,-0.5f, // triangle 2 : end

				0.5f,-0.5f, 0.5f,
				-0.5f,-0.5f,-0.5f,
				0.5f,-0.5f,-0.5f,

				0.5f, 0.5f,-0.5f,
				0.5f,-0.5f,-0.5f,
				-0.5f,-0.5f,-0.5f,

				-0.5f,-0.5f,-0.5f,
				-0.5f, 0.5f, 0.5f,
				-0.5f, 0.5f,-0.5f,

				0.5f,-0.5f, 0.5f,
				-0.5f,-0.5f, 0.5f,
				-0.5f,-0.5f,-0.5f,

				-0.5f, 0.5f, 0.5f,
				-0.5f,-0.5f, 0.5f,
				0.5f,-0.5f, 0.5f,

				0.5f, 0.5f, 0.5f,
				0.5f,-0.5f,-0.5f,
				0.5f, 0.5f,-0.5f,

				0.5f,-0.5f,-0.5f,
				0.5f, 0.5f, 0.5f,
				0.5f,-0.5f, 0.5f,

				0.5f, 0.5f, 0.5f,
				0.5f, 0.5f,-0.5f,
				-0.5f, 0.5f,-0.5f,

				0.5f, 0.5f, 0.5f,
				-0.5f, 0.5f,-0.5f,
				-0.5f, 0.5f, 0.5f,

				0.5f, 0.5f, 0.5f,
				-0.5f, 0.5f, 0.5f,
				0.5f,-0.5f, 0.5f
			};

			static const GLfloat color_buffer_data [] = {
				1,1,0, // color 1
				0,1,1, // color 2
				1,1,0.4, // color 3

				1,1,0.4, // color 3
				0.5,0.5,0.5, // color 4
				1,1,0,  // color 1

				1,1,0, // color 1
				0,1,1, // color 2
				1,1,0.4, // color 3

				1,1,0.4, // color 3
				0.5,0.5,0.5, // color 4
				1,1,0,  // color 1

				1,1,0, // color 1
				0,1,1, // color 2
				1,1,0.4, // color 3

				1,1,0.4, // color 3
				0.5,0.5,0.5, // color 4
				1,1,0,  // color 1

				1,1,0, // color 1
				0,1,1, // color 2
				1,1,0.4, // color 3

				1,1,0.4, // color 3
				0.5,0.5,0.5, // color 4
				1,1,0,  // color 1

				1,1,0, // color 1
				0,1,1, // color 2
				1,1,0.4, // color 3

				1,1,0.4, // color 3
				0.5,0.5,0.5, // color 4
				1,1,0,  // color 1

				1,1,0, // color 1
				0,1,1, // color 2
				1,1,0.4, // color 3

				1,1,0.4, // color 3
				0.5,0.5,0.5, // color 4
				1,1,0,  // color 1
			};

			per = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);

		}	

		void createHead(int slices,int stacks){
			int n = 2 * (slices + 1) * stacks;
			int i = 0;
			float radius = 0.375; 
			GLfloat *points = new GLfloat[3*n];
			GLfloat *color = new GLfloat[3*n];
			for (float theta = -M_PI / 2; theta < M_PI / 2 - 0.0001; theta += M_PI / stacks) {
				for (float phi = -M_PI; phi <= M_PI + 0.0001; phi += 2 * M_PI / slices) {

					points[3*i] = radius*(cos(theta) * sin(phi));
					points[3*i + 1] = radius*(-sin(theta));
					points[3*i + 2] = radius*(cos(theta) * cos(phi));

					color[3*i] = 1;
					color[3*i + 1] = 1;
					color[3*i + 2] = 0;

					/*if(theta> -90*M_PI/180.0f && theta< -35*M_PI/180.0f){
						color[3*i] = 0;
						color[3*i + 1] = 0;
						color[3*i + 2] = 0;
					}*/

					i++;

					points[3*i] = radius*(cos(theta + M_PI / stacks) * sin(phi));
					points[3*i + 1] = radius*(-sin(theta + M_PI / stacks));
					points[3*i + 2] = radius*(cos(theta + M_PI / stacks) * cos(phi));

					color[3*i] = 1;
					color[3*i + 1] = 1;
					color[3*i + 2] = 0;

					/*if(theta > -90*M_PI/180.0f && theta< -35*M_PI/180.0f){
						color[3*i] = 0;
						color[3*i + 1] = 0;
						color[3*i + 2] = 0;
					}*/

					i++;
				}
			}

			head = create3DObject(GL_TRIANGLE_STRIP, n, points, color, GL_FILL);

		}


		void createLimb(int i){
			static const GLfloat vertex_buffer_data [] = {
				-0.1f,-0.5f,-0.1f, // triangle 1 : begin
				-0.1f,-0.5f, 0.1f,
				-0.1f, 0.5f, 0.1f, // triangle 1 : end

				0.1f, 0.5f,-0.1f, // triangle 2 : begin
				-0.1f,-0.5f,-0.1f,
				-0.1f, 0.5f,-0.1f, // triangle 2 : end

				0.1f,-0.5f, 0.1f,
				-0.1f,-0.5f,-0.1f,
				0.1f,-0.5f,-0.1f,

				0.1f, 0.5f,-0.1f,
				0.1f,-0.5f,-0.1f,
				-0.1f,-0.5f,-0.1f,

				-0.1f,-0.5f,-0.1f,
				-0.1f, 0.5f, 0.1f,
				-0.1f, 0.5f,-0.1f,

				0.1f,-0.5f, 0.1f,
				-0.1f,-0.5f, 0.1f,
				-0.1f,-0.5f,-0.1f,

				-0.1f, 0.5f, 0.1f,
				-0.1f,-0.5f, 0.1f,
				0.1f,-0.5f, 0.1f,

				0.1f, 0.5f, 0.1f,
				0.1f,-0.5f,-0.1f,
				0.1f, 0.5f,-0.1f,

				0.1f,-0.5f,-0.1f,
				0.1f, 0.5f, 0.1f,
				0.1f,-0.5f, 0.1f,

				0.1f, 0.5f, 0.1f,
				0.1f, 0.5f,-0.1f,
				-0.1f, 0.5f,-0.1f,

				0.1f, 0.5f, 0.1f,
				-0.1f, 0.5f,-0.1f,
				-0.1f, 0.5f, 0.1f,

				0.1f, 0.5f, 0.1f,
				-0.1f, 0.5f, 0.1f,
				0.1f,-0.5f, 0.1f
			};

			static const GLfloat color_buffer_data [] = {
				1,1,0, // color 1
				0,1,1, // color 2
				1,1,0.4, // color 3

				1,1,0.4, // color 3
				0.5,0.5,0.5, // color 4
				1,1,0,  // color 1

				1,1,0, // color 1
				0,1,1, // color 2
				1,1,0.4, // color 3

				1,1,0.4, // color 3
				0.5,0.5,0.5, // color 4
				1,1,0,  // color 1

				1,1,0, // color 1
				0,1,1, // color 2
				1,1,0.4, // color 3

				1,1,0.4, // color 3
				0.5,0.5,0.5, // color 4
				1,1,0,  // color 1

				1,1,0, // color 1
				0,1,1, // color 2
				1,1,0.4, // color 3

				1,1,0.4, // color 3
				0.5,0.5,0.5, // color 4
				1,1,0,  // color 1

				1,1,0, // color 1
				0,1,1, // color 2
				1,1,0.4, // color 3

				1,1,0.4, // color 3
				0.5,0.5,0.5, // color 4
				1,1,0,  // color 1

				1,1,0, // color 1
				0,1,1, // color 2
				1,1,0.4, // color 3

				1,1,0.4, // color 3
				0.5,0.5,0.5, // color 4
				1,1,0,  // color 1
			};

			limb[i] = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);

		}	


		void draw(){

			glUseProgram (programID);
			if(view==3)
				Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			else if(view==4)
				Matrices.view = glm::lookAt( eye1, target1, up1 ); // Rotating Camera for 3D
			else if(view==1)
				Matrices.view = glm::lookAt( eye2, target2, up2 ); // Rotating Camera for 3D
			else if(view==2)
				Matrices.view = glm::lookAt( eye3, target3, up3 ); // Rotating Camera for 3D
			else if(view==0)
				Matrices.view = glm::lookAt( eye4, target4, up4 ); // Rotating Camera for 3D
			else
				Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			//Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;  // MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);
			glm::mat4 moveBody = glm::translate(glm::vec3(posx,posy,posz));
			center[0]=posx;
			center[1]=posy;
			center[2]=posz;
			Matrices.model *= moveBody;
			if(!jump)
				beforeht = posy;
			if(!onMTile)
				beforeht1 = posy;
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(per);

			Matrices.model = glm::mat4(1.0f);
			glm::mat4 moveLimb = glm::translate(glm::vec3(posx+0.2,posy-1,posz));
			Matrices.model *= moveLimb;
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(limb[0]);

			Matrices.model = glm::mat4(1.0f);
			glm::mat4 moveLimb2 = glm::translate(glm::vec3(posx-0.2,posy-1,posz));
			Matrices.model *= moveLimb2;
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(limb[1]);

			Matrices.model = glm::mat4(1.0f);
			glm::mat4 moveLimb3 = glm::translate(glm::vec3(posx-0.5,posy+0.2,posz));
			glm::mat4 rotateLimb3 =  glm::rotate((float)(-70*M_PI/180.0f), glm::vec3(0,0,1));
			Matrices.model *= (moveLimb3*rotateLimb3);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(limb[2]);

			Matrices.model = glm::mat4(1.0f);
			glm::mat4 moveLimb4 = glm::translate(glm::vec3(posx+0.5,posy+0.2,posz));
			glm::mat4 rotateLimb4 =  glm::rotate((float)(70*M_PI/180.0f), glm::vec3(0,0,1));
			Matrices.model *= (moveLimb4*rotateLimb4);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(limb[3]);

			Matrices.model = glm::mat4(1.0f);
			glm::mat4 moveHead = glm::translate(glm::vec3(posx,posy+0.875,posz));
			//glm::mat4 rotateLimb4 =  glm::rotate((float)(70*M_PI/180.0f), glm::vec3(0,0,1));
			Matrices.model *= (moveHead);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(head);

		}

		void move(){
			if(posy>=2.5){
				if(dir==1)
					posx++;
				if(dir==2)
					posz--;
				if(dir==3)
					posx--;
				if(dir==4)
					posz++;
			}
		}

		void back(){

			if(dir==1)
				posx--;
			if(dir==2)
				posz++;
			if(dir==3)
				posx++;
			if(dir==4)
				posz--;
			posy=2.5-hitno*0.1;
			center[0]=posx;
			center[1]=posy;
			center[2]=posz;
			score--;
			speed=10;
			dir=0;
			//if(hitno>=10){
			//	fall();
				/*posx=0;
				posy=2.5;
				posz=0;
				lives--;
				hitno=0;*/
			//}
			
		}
		
		void checkObstacle(int i){
			if(sqrt((pow(center[0] - obstacle[i].center[0],2)) +(pow(center[1] - obstacle[i].center[1],2)) + (pow(center[2] - obstacle[i].center[2],2))) <= radius + obstacle[i].radius){
				back();
				hitno++;
			}
		}


		void checkCan(){

			if(posx==can.posx && posz == can.posz && can.show){
				can.show=false;
				posy = 4.5;
				levitate=true;
			}
		}
		void checkBelow(){
			float x,z;
			float index;
			int ind1;
			x = posx;
			z = posz;
			index = (10*z + x);
			ind1 = index;
			if(!brick[ind1].isThere && posy <= 2.5){
				//cout << "index: " << index << endl;
				fall();		
			}

		}

		void checkBelowMoving(){
			float x,z;
			float index;
			int ind1;
			x = posx;
			z = posz;
			index = (10*z + x);
			ind1 = index;
			if(brick[ind1].isMove && !jump && !levitate){
				onMTile=true;
				posy = brick[ind1].posy + 2.5;		
			}
			else{
				onMTile=false;
				if(posy>=2.5){
					posy = beforeht1;
					if(!jump && !levitate)
						posy = 2.5;
				}
			}
		}
		void checkBoundary(){
			float x,z;
			float index;
			int ind1;
			x = posx;
			z = posz;
			index = (10*z + x);
			ind1 = index;
			if(posx < 0 || posx > 9 || posz < 0 || posz > 9)
				fall();		

		}
		void collectCoin(int i){
			if(posx==light[i].posx&&posz == light[i].posz&&light[i].show){
				light[i].show=false;
				score+=10;
				coins++;
			}
		}

		void checkHealth(){
		if(hitno>=10)
			fall();
		}
		void fall(){
			posy = -1;
			lives--;
			hitno=0;
			posx=0;
			posy=2.5;
			posz=0;
			count=0;
			speed=10;
			dir=0;

		}

		void leap(){
			if(jump){
				posy += vel*deltaTime - (0.5*5*deltaTime*deltaTime);
				if(!levitate && posy > 4.62)
					hitno++;
				if(posy<beforeht){
					//cout << beforeht << endl;
					posy=beforeht;
					jump=false;
				}
				if(dir==1){
					posx+=vel*deltaTime*0.25;
				}
				if(dir==2){
					posz-=vel*deltaTime*0.25;
				}
				if(dir==3){
					posx-=vel*deltaTime*0.25;
				}
				if(dir==4){
					posz+=vel*deltaTime*0.25;
				}
			}

		}

};

Person person;


/**************************
 * Customizable functions *
 **************************/

void changeCam(int choice){
	if(choice==0){
		eye4 = glm::vec3(8,8,11);
		target4 = glm::vec3(4,2,1);

		eye4x = 8;
		eye4y = 8;
		eye4z = 11;

		target4x = 4;	
		target4y = 2;	
		target4z = 1;	

	}
	else if(choice==1){
		eye4 = glm::vec3(8,8,-11);
		target4 = glm::vec3(4,2,-1);

		eye4x = 8;
		eye4y = 8;
		eye4z = -11;

		target4x = 4;	
		target4y = 2;	
		target4z = -1;	

	}

	else if(choice==2){
		eye4 = glm::vec3(-8,8,-11);
		target4 = glm::vec3(-4,2,-1);

		eye4x = -8;
		eye4y = 8;
		eye4z = -11;

		target4x = -4;	
		target4y = 2;	
		target4z = -1;	

	}

	else if(choice==3){
		eye4 = glm::vec3(-8,8,11);
		target4 = glm::vec3(-4,2,1);

		eye4x = -8;
		eye4y = 8;
		eye4z = 11;

		target4x = -4;	
		target4y = 2;	
		target4z = 1;	

	}

}




void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_T:
				view=(view+1)%5;
				break;
			case GLFW_KEY_SPACE:
				jump=false;
				deltaTime=0;
				person.t=0;
				person.dir=0;
				person.posx = (int)person.posx;
				person.posy = person.beforeht;
				if(person.onMTileJump){
					person.posy=2.5;
					person.onMTileJump=false;
				}
				person.posz = (int)person.posz;
				break;
			case GLFW_KEY_F:
				person.speed-=1;
				if(person.speed<=2){
					person.speed=2;
				}
				break;
			case GLFW_KEY_S:
				person.speed+=1;
				break;
			case GLFW_KEY_P:
				person.dir=0;
				break;
			case GLFW_KEY_C:
				choice=(choice+1)%4;
				changeCam(choice);
				break;
			case GLFW_KEY_UP:
				person.move1=false;
				break;
			case GLFW_KEY_DOWN:
				person.move1=false;
				break;
			case GLFW_KEY_LEFT:
				person.move1=false;
				break;
			case GLFW_KEY_RIGHT:
				person.move1=false;
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			case GLFW_KEY_SPACE:
				if(person.onMTile && person.posy < 2.5){
					person.dir=0;
					person.onMTileJump=true;
				}
				//else	
				jump=true;	
				break;
			case GLFW_KEY_UP:
				person.move1=true;
				if(person.onMTile && person.posy < 2.5);
				else if(view==1||view==2||choice==1||choice==2){
					person.posz+=1;
					person.dir=4;
				}	
				else{		
					person.posz-=1;
					person.dir=2;
				}
				break;
			case GLFW_KEY_DOWN:
				person.move1=true;
				if(person.onMTile && person.posy < 2.5);
				else if(view==1||view==2||choice==1||choice==2){
					person.posz-=1;
					person.dir=2;
				}

				else{		
					person.posz+=1;
					person.dir=4;
				}
				break;
			case GLFW_KEY_LEFT:
				person.move1=true;
				if(person.onMTile && person.posy < 2.5);
				else if(view==1||view==2||choice==1||choice==2){
					person.posx+=1;
					person.dir=1;
				}

				else{
					person.posx-=1;
					person.dir=3;
				}
				break;
			case GLFW_KEY_RIGHT:
				person.move1=true;
				if(person.onMTile && person.posy < 2.5);
				else if(view==1||view==2||choice==1||choice==2){
					person.posx-=1;
					person.dir=3;
				}

				else{
					person.posx+=1;
					person.dir=1;
				}
				break;
			default:
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		case 'A':
		case 'a':
			break;		
		case 'W':
		case 'w':	
			break;
		case 'D':
		case 'd':

			break;
		case 'S':
		case 's':

			break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
bool pressNext=false;
bool pressMove=false;
int moves;
void mouse_callback(GLFWwindow* window,double x,double y){
	if(pressNext){
		target4=glm::vec3(x/75-4,1,y/75-4);
	}

}

void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_PRESS){
				pressNext=true;
			}
			if (action == GLFW_RELEASE){
				pressNext=false;
			}
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_PRESS){
				pressMove=true;
				person.move1=true;
			}
			if (action == GLFW_RELEASE){
				pressMove=false;
				person.move1=false;
				moves=0;
			}
			break;
		default:
			break;
	}
}

float zoom_x,zoom_y,zoom_z;
void scroll(GLFWwindow* window,double x,double y){
	if(pressNext){
	if(choice==0)
		eye4 = glm::vec3(eye4x+zoom_x,eye4y+zoom_y,eye4z+zoom_z);	
	else if(choice==1)
		eye4 = glm::vec3(eye4x+zoom_x,eye4y+zoom_y,eye4z-zoom_z);	
	else if(choice==2)
		eye4 = glm::vec3(eye4x-zoom_x,eye4y+zoom_y,eye4z-zoom_z);	
	else if(choice==3)
		eye4 = glm::vec3(eye4x-zoom_x,eye4y+zoom_y,eye4z+zoom_z);	
	zoom_x += -y*0.5;
	zoom_y += -y*0.5;
	zoom_z += -y*0.5;
	}
	if(pressMove && moves%person.speed==0){
		moves++;
		if(x==1){
				person.move1=true;
				if(person.onMTile && person.posy < 2.5);
				else if(view==1||view==2||choice==1||choice==2){
					person.posx+=1;
					person.dir=1;
				}

				else{
					person.posx-=1;
					person.dir=3;
				}

		}
		else if(x==-1){

				person.move1=true;
				if(person.onMTile && person.posy < 2.5);
				else if(view==1||view==2||choice==1||choice==2){
					person.posx-=1;
					person.dir=3;
				}

				else{
					person.posx+=1;
					person.dir=1;
				}

		}
		else if(y==1){
				
				person.move1=true;
				if(person.onMTile && person.posy < 2.5);
				else if(view==1||view==2||choice==1||choice==2){
					person.posz+=1;
					person.dir=4;
				}	
				else{		
					person.posz-=1;
					person.dir=2;
				}


		}
		
		else if(y==-1){
				person.move1=true;
				if(person.onMTile && person.posy < 2.5);
				else if(view==1||view==2||choice==1||choice==2){
					person.posz-=1;
					person.dir=2;
				}

				else{		
					person.posz+=1;
					person.dir=4;
				}


		}	
	}

}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	//  Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}




/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
	glfwSetScrollCallback(window, scroll);
	glfwSetCursorPosCallback(window,mouse_callback);

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	int i;
	int x=0,z=0;
	int num;
	pressNext=false;
	for(i=0;i<8;i++){
		num = rand()%100;
		brick[num].isThere = false;
	}
	for(i=0;i<100;i++){
		brick[i].create();
	}
	for(i=0;i<5;i++){
		num = rand()%100;
		brick[num].isMove = true;
		brick[num].isThere = true;
	}
	brick[0].isThere = true;
	brick[1].isThere = true;
	brick[99].isThere = true;
	brick[0].isMove = false;
	brick[1].isMove = false;
	brick[99].isMove = false;
	bg.createAxes();
	person.create();
	person.createLimb(0);
	person.createLimb(1);
	person.createLimb(2);
	person.createLimb(3);
	person.createHead(30,30);
	for(i=0;i<10;i++)
		bar[i].create(i);
	for(i=0;i<3;i++){
		obstacle[i].posx = rand()%9 + 1;
		obstacle[i].posy = 2;
		obstacle[i].posz = rand()%9 + 1;
		obstacle[i].speed = (((float)(rand()%50))/1000) + 0.04;
		obstacle[i].create(30,30);
		//cout << obstacle[i].speed << endl;
	}
	can.posx = rand()%5 + 3;
	can.posz = rand()%5 + 3;
	can.create();
	can.createStraw();
	can.createUmb(30,30);
	brick[86].isThere = false;
	for(i=0;i<6;i++){
		light[i].posx = rand()%10;
		light[i].posy = 2;
		light[i].posz = rand()%10;

		light[i].create();
	}
	heart[3].posx = 1.90 + 3;
	heart[3].posy = 3.45 + 5;
	heart[2].posx = 2.40 + 3;
	heart[2].posy = 3.45 + 5;
	heart[1].posx = 2.95 + 3;
	heart[1].posy = 3.45 + 5;
	heart[0].posx = 3.50 + 3;
	heart[0].posy = 3.45 + 5;
	for(i=0;i<4;i++){
		heart[i].createTriangle(0);
		heart[i].createLeft(1);
		heart[i].createRight(2);
	}

	glActiveTexture(GL_TEXTURE0);

	GLuint textureID1 = createTexture("frame-001.png");
	GLuint textureID2 = createTexture("frame-002.png");
	GLuint textureID3 = createTexture("frame-003.png");
	GLuint textureID4 = createTexture("frame-004.png");
	GLuint textureID5 = createTexture("frame-005.png");
	GLuint textureID6 = createTexture("frame-006.png");
	GLuint textureID7 = createTexture("frame-007.png");
	GLuint textureID8 = createTexture("frame-008.png");
	GLuint textureID9 = createTexture("frame-009.png");
	GLuint textureID10 = createTexture("frame-010.png");
	GLuint textureID11 = createTexture("frame-011.png");
	GLuint textureID12 = createTexture("frame-012.png");
	GLuint textureID13 = createTexture("frame-013.png");
	GLuint textureID14 = createTexture("frame-014.png");
	GLuint textureID15 = createTexture("frame-015.png");
	GLuint textureID16 = createTexture("frame-016.png");	
	GLuint textureID17 = createTexture("sand2.png");	
	GLuint textureID18 = createTexture("sand2.png");	
	GLuint textureID19 = createTexture("win.png");	


	textureProgramID = LoadShaders( "TextureRender.vert", "TextureRender.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");


	/* Objects should be created before any other gl function and shaders */
	// Create the models
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	for(i=0;i<100;i++){
		brick[i].createFront(textureID1,0);
		brick[i].createFront(textureID2,1);
		brick[i].createFront(textureID3,2);
		brick[i].createFront(textureID4,3);
		brick[i].createFront(textureID5,4);
		brick[i].createFront(textureID6,5);
		brick[i].createFront(textureID7,6);
		brick[i].createFront(textureID8,7);
		brick[i].createFront(textureID9,8);
		brick[i].createFront(textureID10,9);
		brick[i].createFront(textureID11,10);
		brick[i].createFront(textureID12,11);
		brick[i].createFront(textureID13,12);
		brick[i].createFront(textureID14,13);
		brick[i].createFront(textureID15,14);
		brick[i].createFront(textureID16,15);
		brick[i].createUp(textureID17,0);
		brick[i].createUp(textureID18,1);
		brick[i].createDown(textureID17,0);
		brick[i].createDown(textureID18,1);
		brick[i].createRight(textureID17,0);
		brick[i].createRight(textureID18,1);
		brick[i].createLeft(textureID17,0);
		brick[i].createLeft(textureID18,1);
		brick[i].createBack(textureID17,0);
		brick[i].createBack(textureID18,1);
	}
		brick[99].createUp(textureID19,0);
		brick[99].createUp(textureID19,1);
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.7f, 0.5f, 1.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;
	int counter=0,move_count=0;
	stringstream ss1;
	string convStr1,concatStr;
	GLFWwindow* window = initGLFW(width, height);
	initGL (window, width, height);
	double last_update_time = glfwGetTime(), current_time;
	while (!glfwWindowShouldClose(window)) {
		int i,j;
		bg.clean1();
		bg.draw();
		ss1.str("");
		ss1 << person.score;
		convStr1 = ss1.str();
		concatStr = "Waterfall Maze!!!\t\t\t\t\t Score: " + convStr1;
		const char *gameTitle = concatStr.c_str();
		glfwSetWindowTitle(window,gameTitle);

		for(i=0;i<10;i++)
		{
			for(j=0;j<10;j++){
				if(brick[(10*i)+j].isThere){
					brick[(10*i)+j].draw(i,j);
					brick[(10*i)+j].drawGif(i,j);
				}
			}
		}

		for(i=0;i<person.lives;i++){
			heart[i].draw(0);
			heart[i].draw(1);
			heart[i].draw(2);
		}
		if(can.show)
			can.draw();
		for(i=0;i<6;i++){
			if(light[i].show)
				light[i].draw();
			person.collectCoin(i);
		}

		person.draw();
		for(i=0;i<10-person.hitno;i++)
			bar[i].draw(6,6+0.2*i,0.25,0.1);
		for(i=0;i<3;i++)
			obstacle[i].draw();
		person.checkBelow();
		person.checkBelowMoving();
		person.checkCan();
		for(i=0;i<3;i++){
			person.checkObstacle(i);
		}

		person.checkBoundary();
		person.checkHealth();
		person.leap();

		eye2=glm::vec3(person.posx,person.posy,person.posz+0.5);
		target2=glm::vec3(person.posx,person.posy,person.posz+2);

		eye3=glm::vec3(person.posx,person.posy+1,person.posz-1.5);
		target3=glm::vec3(person.posx,person.posy,person.posz+2);

		glfwSwapBuffers(window);
		glfwPollEvents();
		if(person.lives==0){
			cout << "Score: " << person.score << endl;
			quit(window);
		}
		if(person.coins==6&&person.posx==9&&person.posz==9){
			cout << "You won!!! Score: " << person.score << endl;
			quit(window);
		}
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.025) { // atleast 0.5s elapsed since last frame
			last_update_time = current_time;
			move_count++;
			if(person.move1 && move_count%person.speed == 0)
				person.move();
			if(jump){
				deltaTime+=0.025;
				//cout << "time: " << deltaTime << endl;
				person.t+=0.025;
			}
			if(move_count%240==0){
				for(i=0;i<3;i++){
					obstacle[i].posx = rand()%9 + 1;
					obstacle[i].posy = 2;
					obstacle[i].posz = rand()%9 + 1;
					obstacle[i].speed = (((float)(rand()%50))/1000) + 0.04;
					obstacle[i].dir = 1;
				}
			}
			if(person.levitate){
				counter++;
				if(counter==320){
					counter=0;
					person.posy=2.5;
					person.levitate=false;
				}
			}
			for(i=0;i<100;i++)
				brick[i].index1 = (brick[i].index1 + 1)%16;	
			brick[i].index2 = (brick[i].index2 + 1)%2;	
		}
	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
