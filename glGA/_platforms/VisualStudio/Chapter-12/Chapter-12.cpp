// basic STL streams
#include <iostream>
#include <sstream>

// AntWeak lib
#include <AntTweakBar.h>


// GLEW lib
// http://glew.sourceforge.net/basic.html
#include <../_thirdPartyLibs/glew-1.9.0/include/GL/glew.h>

// Here we decide which of the two versions we want to use
// If your systems supports both, choose to uncomment USE_OPENGL32
// otherwise choose to uncomment USE_OPENGL21
// GLView cna also help you decide before running this program:
// 
#define USE_OPENGL32
#include <../_thirdPartyLibs/GL3/gl3.h>

//#define USE_OPENGL21

// GLFW lib
// http://www.glfw.org/documentation.html
#ifdef USE_OPENGL32
#define GLFW_INCLUDE_GL3
#define USE_GL3
#define GLFW_NO_GLU
#define GL3_PROTOTYPES 1 
#endif
#define GLFW_DLL //use GLFW as a dynamically linked library
#include <../_thirdPartyLibs/glfw-2.7.6/include/GL/glfw.h>

// GLM lib
// http://glm.g-truc.net/api/modules.html
#define GLM_SWIZZLE
#define GLM_FORCE_INLINE
#include <../_thirdPartyLibs/glm-0.9.3.4/glm/glm.hpp>
#include <../_thirdPartyLibs/glm-0.9.3.4/glm/gtx/string_cast.hpp>
#include <../_thirdPartyLibs/glm-0.9.3.4/glm/gtc/matrix_transform.hpp>
#include <../_thirdPartyLibs/glm-0.9.3.4/glm/gtc/quaternion.hpp>
#include <../_thirdPartyLibs/glm-0.9.3.4/glm/gtc/type_ptr.hpp>
#include <../_thirdPartyLibs/glm-0.9.3.4/glm/gtc/random.hpp>

//local
#include "glGAHelper.h"
#include "glGAMesh.h"
#include "HDRI_frame.h"

// global variables
int         windowWidth=1024, windowHeight=768;

GLuint      program3Dmodel,program,programQuad;
GLuint      vao3Dmodel,vao,vaoQuad;
GLuint      buffer3Dmodel,buffer,bufferQuad;

GLuint		MV_uniform3D, MVP_uniform3D,Normal_uniform3D,MV_uniform ,Normal_uniform;
GLuint		MVP_uniform,MVP_uniformQuad,MVTR_uniform3D,uniformDr; 

GLuint		gSampler;
GLuint		texID;

Texture	    *pTexture = NULL;
Mesh		*m		  = NULL;

const       int         NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
int                     Index = 0;

bool        wireFrame = false;
bool		camera    = true;

typedef     glm::vec4   color4;
typedef     glm::vec4   point4;

//Modelling arrays
point4      points[NumVertices];
glm::vec2	tex_coords[NumVertices];
color4      colors[NumVertices];
glm::vec3   normals[NumVertices];

glm::vec3 pos = glm::vec3( 0.0f, 0.0f , 10.0f );

const glm::vec3 cpos = glm::vec3(0.0f,0.0f,0.0f);

float horizAngle = 3.14f;
float verticAngle = 0.0f;


float speedo = 3.0f;
float mouseSpeedo = 0.005f;

int	  xpos = 0,ypos = 0;

glm::vec3 LightPos  = glm::vec3(2.0f,5.0f,10.0f);

glm::vec3 BaseColor = glm::vec3(0.6f,0.6f,0.6f);

float	  MixRatio  = 0.2f;

int timesc = 0;
// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4( -20.5, -20.5, 20.5, 1.0 ),
    point4( -20.5,  20.5, 20.5, 1.0 ),
    point4( 20.5,  20.5, 20.5, 1.0 ),
    point4( 20.5, -20.5, 20.5, 1.0 ),
    point4( -20.5, -20.5, -20.5, 1.0 ),
    point4( -20.5,  20.5, -20.5, 1.0 ),
    point4( 20.5,  20.5, -20.5, 1.0 ),
    point4( 20.5, -20.5, -20.5, 1.0 )
};


// RGBA colors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};

point4 QuadVertices[] = {
	point4(-1.0f, -1.0f, 0.0f,1.0f),
	point4( 1.0f, -1.0f, 0.0f,1.0f),
	point4(-1.0f,  1.0f, 0.0f,1.0f),
	point4(-1.0f,  1.0f, 0.0f,1.0f),
	point4( 1.0f, -1.0f, 0.0f,1.0f),
	point4( 1.0f,  1.0f, 0.0f,1.0f)
};

color4 QuadColors[] = {
	color4(1.0f,0.5f,1.0f,1.0f),
	color4(1.0f,0.5f,1.0f,1.0f),
	color4(1.0f,0.5f,1.0f,1.0f),
	color4(1.0f,0.5f,1.0f,1.0f),
	color4(1.0f,0.5f,1.0f,1.0f),
	color4(1.0f,0.5f,1.0f,1.0f)
};

bool initializeFBO();
void setupFBO();

float Exposure = 0.0f;
float Gamma = 1.0f/2.2f;
float Defog = 0.0f;
glm::vec3 FogColor = glm::vec3(1.0f,1.0f,1.0f);

float SpecularPercent = 1.0;
float DiffusePercent  = 1.0;

unsigned int cubeMap,diffMap;

/************** FBO ********************/
GLuint FBO;
GLuint DepthBuffer;

/**************************************/

HDRI_frame	hdrImg;

GLuint TextureIDs[3];

const char * hdr_map = "./Textures/St.Peter's_Basilica/stpeters_cross.hdr";
const char * hdr_diff_map = "./Textures/St.Peter's_Basilica/stpeters_diffuse_cross.hdr";

const char * hdr_map_fiz = "./Textures/Uffizi_Gallery/uffizi_cross.hdr";
const char * hdr_diff_map_fiz = "./Textures/Uffizi_Gallery/uffizi_cross_diffuse.hdr";

const char * hdr_map_gc = "./Textures/Grand_Canyon/Grand_Canyon.hdr";
const char * hdr_diff_map_gc = "./Textures/Grand_Canyon/Grand_Canyon_diff.hdr";

const char * hdr_map_gr = "./Textures/Washington-Gold-Room/Gold_Room.hdr";
const char * hdr_diff_map_gr = "./Textures/Washington-Gold-Room/Gold_Room_diff.hdr";

struct Map{

	enum HdrMap {st_Peter , ufizi , grandc , gr};
	HdrMap map_val;
};


void LoadMap(const char * hpmap,GLuint tex,int i)
{
	std::cout<<"\ntex : "<<tex<<" i : "<<i<<"\n";
	hdrImg.SetTextureUnit(i);
	hdrImg.SetTextureImage(tex);
	hdrImg.ReadHDR(hpmap);
	hdrImg.CreateCubeMapFaces();
	hdrImg.BindCubeMapTexture();
}

// Callback function called by GLFW when window size changes
void GLFWCALL WindowSizeCB(int width, int height)
{
    // Set OpenGL viewport and default camera
    glViewport(0, 0, width, height);

	float aspect = (GLfloat)width / (GLfloat)height;

	windowWidth = width;
	windowHeight = height;
	initializeFBO();
	setupFBO();
	// Send the new window size to AntTweakBar
    TwWindowSize(width, height);
}

// Callback function called by GLFW when a mouse button is clicked
void GLFWCALL OnMouseButton(int glfwButton, int glfwAction)
{
    // Do something if needed.
}


// Callback function called by GLFW when mouse has moved
void GLFWCALL OnMousePos(int mouseX, int mouseY)
{
    // Do something if needed.
}


// Callback function called by GLFW on mouse wheel event
void GLFWCALL OnMouseWheel(int pos)
{
    // Do something if needed.
}


// Callback function called by GLFW on key event
void GLFWCALL OnKey(int glfwKey, int glfwAction)
{
    
    if( glfwKey==GLFW_KEY_ESC && glfwAction==GLFW_PRESS ) // Want to quit?
	{
        glfwCloseWindow();
	}
	else if( glfwGetKey(GLFW_KEY_SPACE) == GLFW_PRESS )
	{
		camera = false;
	}
    else
    {
        // Event has not been handled
        // Do something if needed.
    }
}

// Callback function called by GLFW on char event
void GLFWCALL OnChar(int glfwChar, int glfwAction)
{
    // Do something if needed.
}

void checkActiveUniforms()
{
	GLint nUniforms, maxLen;
	glGetProgramiv( program3Dmodel, GL_ACTIVE_UNIFORM_MAX_LENGTH,&maxLen);
	glGetProgramiv( program3Dmodel, GL_ACTIVE_UNIFORMS,&nUniforms);
	GLchar * name = (GLchar *) malloc( maxLen );
	GLint size, location;
	GLsizei written;
	GLenum type;

	printf(" Location | Name\n");
	printf("------------------------------------------------\n");

	for( int i = 0; i < nUniforms; ++i ) {

		glGetActiveUniform( program3Dmodel, i, maxLen, &written,&size, &type, name );
		location = glGetUniformLocation(program3Dmodel, name);
		printf(" %-8d | %s\n", location, name);
	}
	free(name);
}

// quad generates two triangles for each face and assigns colors
//    to the vertices
void quad( int a, int b, int c, int d )
{
    //specify temporary vectors along each quad's edge in order to compute the face
    // normal using the cross product rule
    glm::vec3 u = (vertices[b]-vertices[a]).xyz();
    glm::vec3 v = (vertices[c]-vertices[b]).xyz();
    glm::vec3 norm = glm::cross(u, v);
    glm::vec3 normal= glm::normalize(norm);
    
	normals[Index]=normal;colors[Index] = vertex_colors[a]; points[Index] = vertices[a];tex_coords[Index] = glm::vec2( 0.0 , 0.0); Index++;
    normals[Index]=normal;colors[Index] = vertex_colors[b]; points[Index] = vertices[b];tex_coords[Index] = glm::vec2( 0.0 , 0.5); Index++;
    normals[Index]=normal;colors[Index] = vertex_colors[c]; points[Index] = vertices[c];tex_coords[Index] = glm::vec2( 0.5 , 0.0); Index++;
    normals[Index]=normal;colors[Index] = vertex_colors[a]; points[Index] = vertices[a];tex_coords[Index] = glm::vec2( 0.0 , 0.5); Index++;
    normals[Index]=normal;colors[Index] = vertex_colors[c]; points[Index] = vertices[c];tex_coords[Index] = glm::vec2( 0.5 , 0.5); Index++;
    normals[Index]=normal;colors[Index] = vertex_colors[d]; points[Index] = vertices[d];tex_coords[Index] = glm::vec2( 0.5 , 0.0); Index++;
}

// generate 12 triangles: 36 vertices and 36 colors
void colorcube()
{
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
	
}

void initCube()
{
    //generate and bind a VAO for the 3D axes
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    colorcube();
    
    // Load shaders and use the resulting shader program
    program = LoadShaders( "./Shaders/vshaderSkybox.vert", "./Shaders/fshaderSkybox.frag" );
    glUseProgram( program );
    
    // Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors) + sizeof(normals) + sizeof(tex_coords),NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals), sizeof(colors) , colors );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals) + sizeof(colors) ,sizeof(tex_coords) , tex_coords );

    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );

	GLuint vNormal = glGetAttribLocation( program, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(points)) );
    
    GLuint vColor = glGetAttribLocation( program, "vColor" ); 
    glEnableVertexAttribArray( vColor );
	glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(points) + sizeof(normals)) );

	GLuint vText = glGetAttribLocation( program, "vTexture" );
    glEnableVertexAttribArray( vText );
	glVertexAttribPointer( vText, 2, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(points) + sizeof(normals) + sizeof(colors)) );
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
    
    
    // only one VAO can be bound at a time, so disable it to avoid altering it accidentally
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void displayCube(glm::mat4 tsl,glm::vec3 constpos,glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv)
{
    glUseProgram(program);
    glBindVertexArray(vao);
    
    glDisable(GL_CULL_FACE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if (wireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	MV_uniform = glGetUniformLocation(program, "MV_mat");
	MVP_uniform = glGetUniformLocation(program, "MVP_mat");
	Normal_uniform = glGetUniformLocation(program, "Normal_mat");
	
	//Calculation of ModelView Matrix
	glm::mat4 model_mat = tsl;
	glm::mat4 view_mat = glm::lookAt(constpos,directionv,upv);
	glm::mat4 MV_mat = view_mat * model_mat;
	glUniformMatrix4fv(MV_uniform,1, GL_FALSE, glm::value_ptr(MV_mat));

	//Calculation of Normal Matrix
	glm::mat3 Normal_mat = glm::transpose(glm::inverse(glm::mat3(MV_mat)));
	glUniformMatrix3fv(Normal_uniform,1, GL_FALSE, glm::value_ptr(Normal_mat));

	//Calculation of ModelViewProjection Matrix
	float aspect = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 projection_mat = glm::perspective(45.0f, aspect,0.1f,100.0f);
	glm::mat4 MVP_mat = projection_mat * MV_mat;
	glUniformMatrix4fv(MVP_uniform, 1, GL_FALSE, glm::value_ptr(MVP_mat));


	glUniform1i(glGetUniformLocation(program, "CubeMapTex"), 0);
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, TextureIDs[2]);

    glDrawArrays( GL_TRIANGLES, 0, NumVertices );


    glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);

}

void init3Dmodel()
{

	glGenVertexArrays(1, &vao3Dmodel);
    glBindVertexArray(vao3Dmodel);
	GLuint m_Buffers[4];
	m = new Mesh();
	//m->loadMesh("./Models/box1/models/CargoCube01.dae");  
	m->loadMesh("./Models/sphere.dae");

    //Load shaders and use the resulting shader program
	program3Dmodel = LoadShaders( "./Shaders/vshader3D.vert", "./Shaders/fshader3D.frag" );

    glUseProgram( program3Dmodel );

	std::cout<<"this is Position size :  "<<m->Positions.size()<<" and this is Normal size : "<<m->Normals.size()<<" and this is TexCoords size : "<<m->TexCoords.size()<<std::endl;
	std::cout<<"this is numVertices "<<m->numVertices<<std::endl;
	std::cout<<"this is numSamplers "<<m->m_TextureSamplers.size()<<std::endl;
	std::cout<<"this is numTextures "<<m->m_Textures.size()<<std::endl;

	// Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
	glGenBuffers( 1, &buffer3Dmodel );
	glBindBuffer( GL_ARRAY_BUFFER, buffer3Dmodel );
	glBufferData( GL_ARRAY_BUFFER,(sizeof(m->Positions[0]) * m->Positions.size()) + (sizeof(m->Normals[0]) * m->Normals.size()) + (sizeof(m->TexCoords[0]) * m->TexCoords.size()),NULL, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, (sizeof(m->Positions[0]) * m->Positions.size()), &m->Positions[0] );
	glBufferSubData( GL_ARRAY_BUFFER, (sizeof(m->Positions[0]) * m->Positions.size()), (sizeof(m->Normals[0]) * m->Normals.size()), &m->Normals[0] );
	glBufferSubData( GL_ARRAY_BUFFER, (sizeof(m->Positions[0]) * m->Positions.size()) + (sizeof(m->Normals[0]) * m->Normals.size()),(sizeof(m->TexCoords[0]) * m->TexCoords.size()), &m->TexCoords[0] );
	
	glGenBuffers(1, &m_Buffers[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m->Indices[0]) * m->Indices.size(), &m->Indices[0], GL_STATIC_DRAW );

	// set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program3Dmodel, "MCVertex" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );

	GLuint vNormal = glGetAttribLocation( program3Dmodel, "MCNormal" ); 
    glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET((sizeof(m->Positions[0]) * m->Positions.size())) );

	GLuint vText = glGetAttribLocation( program3Dmodel, "MCCoords" );
    glEnableVertexAttribArray( vText );
	glVertexAttribPointer( vText, 2, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET((sizeof(m->Positions[0]) * m->Positions.size()) + (sizeof(m->Normals[0]) * m->Normals.size()) ));
	
	glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void display3Dmodel(glm::mat4 tsl,glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv,int im)
{
	glUseProgram(program3Dmodel);
	glBindVertexArray(vao3Dmodel);

	glDisable(GL_CULL_FACE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if (wireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	MV_uniform3D = glGetUniformLocation(program3Dmodel, "MV_mat");
	MVTR_uniform3D = glGetUniformLocation(program3Dmodel, "MV_matTR");
	MVP_uniform3D = glGetUniformLocation(program3Dmodel, "MVP_mat");
	Normal_uniform3D = glGetUniformLocation(program3Dmodel, "Normal_mat");

	//Calculation of ModelView Matrix
	glm::mat4 model_mat = tsl;
	glm::mat4 view_mat = glm::lookAt(positionv,positionv + directionv,upv);

	glm::mat4 MV_mat = view_mat * model_mat;
	glUniformMatrix4fv(MV_uniform3D,1, GL_FALSE, glm::value_ptr(MV_mat)); 

	glm::mat4 MV_matTR = glm::transpose(MV_mat);
	glUniformMatrix4fv(MVTR_uniform3D,1, GL_FALSE, glm::value_ptr(MV_matTR)); 


	glUniform3fv(glGetUniformLocation(program3Dmodel,"BaseColor"),1,glm::value_ptr(BaseColor));
	glUniform1f(glGetUniformLocation(program3Dmodel,"SpecularPercent"),SpecularPercent);
	glUniform1f(glGetUniformLocation(program3Dmodel,"DiffusePercent"),DiffusePercent);

	glUniform1i(glGetUniformLocation(program3Dmodel,"Mode"),im);

	//Calculation of Normal Matrix
	glm::mat3 Normal_mat = glm::transpose(glm::inverse(glm::mat3(MV_mat)));
	glUniformMatrix3fv(Normal_uniform3D,1, GL_FALSE, glm::value_ptr(Normal_mat));

	//Calculation of ModelViewProjection Matrix
	float aspect = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 projection_mat = glm::perspective(45.0f, aspect,0.1f,100.0f);
	glm::mat4 MVP_mat = projection_mat * MV_mat;
	glUniformMatrix4fv(MVP_uniform3D, 1, GL_FALSE, glm::value_ptr(MVP_mat));


	glUniform1i(glGetUniformLocation(program3Dmodel, "SpecularEnvMap"), 0);

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, TextureIDs[0]);

	glUniform1i(glGetUniformLocation(program3Dmodel, "DiffuseEnvMap"), 1);

	glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, TextureIDs[1]);


	m->render(); 

	if(timesc < 1)
	{
		checkActiveUniforms();
	}
	timesc++;

	glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);
}

void initQuad()
{
	//generate and bind a VAO for the 3D axes
    glGenVertexArrays(1, &vaoQuad);
    glBindVertexArray(vaoQuad);

	// Load shaders and use the resulting shader program
    programQuad = LoadShaders( "./Shaders/quad.vert", "./Shaders/quad.frag" );
    glUseProgram( programQuad );

	// Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
    glGenBuffers( 1, &bufferQuad );
    glBindBuffer( GL_ARRAY_BUFFER, bufferQuad );
    glBufferData( GL_ARRAY_BUFFER, sizeof(QuadVertices) + sizeof(QuadColors),NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(QuadVertices), QuadVertices );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(QuadVertices), sizeof(QuadColors), QuadColors );
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( programQuad, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );
    
    GLuint vColor = glGetAttribLocation( programQuad, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(QuadVertices)) );
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
    
    // only one VAO can be bound at a time, so disable it to avoid altering it accidentally
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void displayQuad(glm::mat4 md,glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv)
{
	glUseProgram(programQuad);
	glBindVertexArray(vaoQuad);

	glDisable(GL_CULL_FACE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (wireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	MVP_uniformQuad = glGetUniformLocation(programQuad, "MVP_mat");

	// Calculation of ModelView Matrix
	glm::mat4 model_matq = md;
	glm::mat4 view_matq = glm::lookAt(positionv,positionv + directionv,upv);

	glm::mat4 MV_matq = view_matq * model_matq;

	// Calculation of ModelViewProjection Matrix
	float aspect = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 projection_matq = glm::perspective(45.0f, aspect,0.1f,100.0f);
	glm::mat4 MVP_matq = projection_matq * MV_matq;
	glUniformMatrix4fv(MVP_uniformQuad, 1, GL_FALSE, glm::value_ptr(MVP_matq));

	glUniform1f(glGetUniformLocation(programQuad,"exposure"),Exposure);
	glUniform1f(glGetUniformLocation(programQuad,"Gamma"),Gamma);
	glUniform1f(glGetUniformLocation(programQuad,"Defog"),Defog);

	glUniform3fv(glGetUniformLocation(programQuad,"FogColor"),1,glm::value_ptr(FogColor));

	texID =  glGetUniformLocationARB(programQuad, "texID");
	glUniform1i(texID, 2);

	glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, TextureIDs[2]);
    glGenerateMipmap(GL_TEXTURE_2D);

    glDrawArrays( GL_TRIANGLES, 0, 6);
    
    glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);
}

bool initializeFBO()
{
	//generate fbo objects
  glGenFramebuffers(1, &FBO);

  glBindFramebuffer(GL_FRAMEBUFFER, FBO);

  //death depthbuffer
  glGenRenderbuffers(1, &DepthBuffer);

   //bind renderbuffer to depthbuffer variabler
  glBindRenderbuffer(GL_RENDERBUFFER, DepthBuffer);

  //select what type of renderbuffer to use
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);

  //attach depth to framebuffer
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthBuffer);

  //check status of frame buffer
  GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FBO error, status: 0x%x\n", Status);
        return false;
    }
  else
  {
	  return true;
  }

}

void setupFBO()
{
	glBindTexture(GL_TEXTURE_2D, TextureIDs[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureIDs[2], 0);
}

int main (int argc, const char * argv[])
{

	TwBar *myBar;
	float bgColor[] = { 0.0f, 0.0f, 0.0f, 0.1f };

	Map *x;
	x = new Map();
	x->map_val = Map::st_Peter;

	// Current time
	double time = 0;

	double FT = 0;
	double FPS = 0;

	int rate = 0;
	int fr = 0;

	double starting = 0.0;
	double ending   = 0.0;

	 // initialise GLFW
    int running = GL_TRUE;
    
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    
    //only for OpenGL 2.1
#ifdef USE_OPENGL21
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);
#endif
    
    //Only for OpenGL 3.2
#ifdef USE_OPENGL32
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
    glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#endif

	GLFWvidmode mode;
    glfwGetDesktopMode(&mode);
    if( !glfwOpenWindow(windowWidth, windowHeight, mode.RedBits, mode.GreenBits, mode.BlueBits, 0, 32, 0, GLFW_WINDOW /* or GLFW_FULLSCREEN */) )
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwEnable(GLFW_MOUSE_CURSOR);
    glfwEnable(GLFW_KEY_REPEAT);
    // Ensure we can capture the escape key being pressed below
	glfwEnable( GLFW_STICKY_KEYS );
	glfwSetMousePos(windowWidth/2, windowHeight/2);
    glfwSetWindowTitle("Chapter-12");

	// Initialize AntTweakBar
    if ( !TwInit(TW_OPENGL_CORE, NULL))
	{
		fprintf(stderr,"AntweakBar initialiazation failed: %s\n",TwGetLastError());
		exit(1);
	}

    // Create a tweak bar
	myBar = TwNewBar("TweakBar");

    //init GLEW and basic OpenGL information
    // VERY IMPORTANT OTHERWISE GLEW CANNOT HANDLE GL3
#ifdef USE_OPENGL32
    glewExperimental = true; 
#endif
    glewInit();
    std::cout<<"\nUsing GLEW "<<glewGetString(GLEW_VERSION)<<std::endl;
    if (GLEW_VERSION_2_1)
    {
        std::cout<<"\nYay! OpenGL 2.1 is supported and GLSL 1.2!\n"<<std::endl;
    }
    if (GLEW_VERSION_3_2)
    {
        std::cout<<"Yay! OpenGL 3.2 is supported and GLSL 1.5!\n"<<std::endl;
    }
    
    /*
     This extension defines an interface that allows various types of data
     (especially vertex array data) to be cached in high-performance
     graphics memory on the server, thereby increasing the rate of data
     transfers.
     Chunks of data are encapsulated within "buffer objects", which
     conceptually are nothing more than arrays of bytes, just like any
     chunk of memory.  An API is provided whereby applications can read
     from or write to buffers, either via the GL itself (glBufferData,
     glBufferSubData, glGetBufferSubData) or via a pointer to the memory.
     */
	if (glewIsSupported("GL_ARB_vertex_buffer_object"))
		std::cout<<"ARB VBO's are supported"<<std::endl;
    else if (glewIsSupported("GL_APPLE_vertex_buffer_object"))
		std::cout<<"APPLE VBO's are supported"<<std::endl;
	else
		std::cout<<"VBO's are not supported,program will not run!!!"<<std::endl; 
    
    /* 
     This extension introduces named vertex array objects which encapsulate
     vertex array state on the client side. The main purpose of these 
     objects is to keep pointers to static vertex data and provide a name 
     for different sets of static vertex data.  
     By extending vertex array range functionality this extension allows multiple
     vertex array ranges to exist at one time, including their complete sets of
     state, in manner analogous to texture objects. 
     GenVertexArraysAPPLE creates a list of n number of vertex array object
     names.  After creating a name, BindVertexArrayAPPLE associates the name with
     a vertex array object and selects this vertex array and its associated
     state as current.  To get back to the default vertex array and its
     associated state the client should bind to vertex array named 0.
     */
    
	if (glewIsSupported("GL_ARB_vertex_array_object"))
        std::cout<<"ARB VAO's are supported\n"<<std::endl;
    else if (glewIsSupported("GL_APPLE_vertex_array_object"))//this is the name of the extension for GL2.1 in MacOSX
		std::cout<<"APPLE VAO's are supported\n"<<std::endl;
	else
		std::cout<<"VAO's are not supported, program will not run!!!\n"<<std::endl;
    
    
    std::cout<<"Vendor: "<<glGetString (GL_VENDOR)<<std::endl;
    std::cout<<"Renderer: "<<glGetString (GL_RENDERER)<<std::endl;
    std::cout<<"Version: "<<glGetString (GL_VERSION)<<std::endl;
    

    // Set GLFW event callbacks
    // - Redirect window size changes to the callback function WindowSizeCB
    glfwSetWindowSizeCallback(WindowSizeCB);
    
    // - Directly redirect GLFW mouse button events to AntTweakBar
    glfwSetMouseButtonCallback((GLFWmousebuttonfun)TwEventMouseButtonGLFW);
    
    // - Directly redirect GLFW mouse position events to AntTweakBar
    glfwSetMousePosCallback((GLFWmouseposfun)TwEventMousePosGLFW);
    
    // - Directly redirect GLFW mouse wheel events to AntTweakBar
    glfwSetMouseWheelCallback((GLFWmousewheelfun)TwEventMouseWheelGLFW);
    
    // - Directly redirect GLFW key events to AntTweakBar
    glfwSetKeyCallback((GLFWkeyfun)TwEventKeyGLFW);
    
    // - Directly redirect GLFW char events to AntTweakBar
    glfwSetCharCallback((GLFWcharfun)TwEventCharGLFW);

	TwDefine("TweakBar label='Main TweakBar' alpha=0 position='1 1' help='Use this bar to control the objects of the scene.' ");

	TwAddVarRW(myBar, "Base Color", TW_TYPE_COLOR3F, &BaseColor," label='Base Color' group='IBL control' ");

	TwAddVarRW(myBar, "SpecularPercent", TW_TYPE_FLOAT, &SpecularPercent, "min=0.1 max=5.0 step=0.1 group='IBL control' ");

	TwAddVarRW(myBar, "DiffusePercent", TW_TYPE_FLOAT, &DiffusePercent, "min=0.1 max=5.0 step=0.1 group='IBL control' ");

	TwAddVarRW(myBar, "MS per 1 Frame" , TW_TYPE_DOUBLE, &FPS, "label='MS per 1 Frame' readonly=true group='Frame Rate'");

	TwAddVarRW(myBar, "Frames Per Second" , TW_TYPE_INT32, &rate, "label='FPS' readonly=true group='Frame Rate'");

	TwAddVarRW(myBar, "Exposure" , TW_TYPE_FLOAT, &Exposure, "label='Exposure' min=-10.0 max=10.0 step=0.1 group='Exposure & Gamma Control'");

	TwAddVarRW(myBar, "Gamma" , TW_TYPE_FLOAT, &Gamma, "label='Gamma' min=0.0 max=1.0 step=0.01 group='Exposure & Gamma Control'");

	TwAddVarRW(myBar, "Defog" , TW_TYPE_FLOAT, &Defog, "label='Defog' min=0.0 max=0.1 step=0.0001 group='Exposure & Gamma Control'");

	TwAddVarRW(myBar, "FogColor" , TW_TYPE_COLOR3F, &FogColor, "label='FogColor'  group='Exposure & Gamma Control'");

	// Define a new enum type for the tweak bar
    TwEnumVal modeEV[] =
    {
		{ Map::st_Peter,     "St.Peter Basilica"     }, 
		{ Map::ufizi   ,	  "Uffizi Gallery"		 },
		{ Map::grandc  ,	   "Grand Canyon"		 },
		{ Map::gr	   ,	"Washington G.Room"		 }
					
    };
    TwType modeType = TwDefineEnum("Mode", modeEV, 4);  // create a new TwType associated to the enum defined by the modeEV array

	TwAddVarRW(myBar, "HDR-MAP" ,modeType, &x->map_val, "label='Texture' group='Scene Management'");

	//TwAddVarRW(myBar, "vSYNC" , TW_TYPE_BOOL8, &SYNC, "label='vSync' readonly=true group='Frame Rate'");

	 // Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	glEnable(GLU_CULLING);
	initializeFBO();

	glGenTextures(3, &TextureIDs[0]);

	LoadMap(hdr_map,TextureIDs[0],0);
	LoadMap(hdr_diff_map,TextureIDs[1],1);

	initCube();
	init3Dmodel();
	initQuad();

	setupFBO();

	int init_map = 0;
	float spee = 1.0;
	bool go = TRUE;
	// Initialize time
    time = glfwGetTime();
	double currentTime;
	float lastTime = 0.0f;

	int Frames = 0;
	double LT = glfwGetTime();
	starting = glfwGetTime();

	while (running) {

		if(x->map_val == 0 && (init_map == 1 || init_map == 2 || init_map == 3))
		{
			std::cout<<"\nChanges applied!\n";
			LoadMap(hdr_map,TextureIDs[0],0);
			LoadMap(hdr_diff_map,TextureIDs[1],1);
			init_map = 0;
		}
		else if(x->map_val == 1 && (init_map == 0 || init_map == 2 || init_map == 3))
		{
			std::cout<<"\nChanges applied!\n";
			LoadMap(hdr_map_fiz,TextureIDs[0],0);
			LoadMap(hdr_diff_map_fiz,TextureIDs[1],1);
			init_map = 1;
		}
		else if(x->map_val == 2 && (init_map == 0 || init_map == 1 || init_map == 3))
		{
			std::cout<<"\nChanges applied!\n";
			LoadMap(hdr_map_gc,TextureIDs[0],0);
			LoadMap(hdr_diff_map_gc,TextureIDs[1],1);
			init_map = 2;
		}
		else if(x->map_val == 3 && (init_map == 0 || init_map == 1 || init_map == 2))
		{
			std::cout<<"\nChanges applied!\n";
			LoadMap(hdr_map_gr,TextureIDs[0],0);
			LoadMap(hdr_diff_map_gr,TextureIDs[1],1);
			init_map = 3;
		}
		


		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
        glClearColor( bgColor[0], bgColor[1], bgColor[2], bgColor[3]); //black color

		if(camera == true)
		{
			glfwGetMousePos(&xpos,&ypos);
			glfwSetMousePos(windowWidth/2, windowHeight/2);
		
			horizAngle  += mouseSpeedo * float(windowWidth/2 - xpos );
			verticAngle += mouseSpeedo * float( windowHeight/2 - ypos );
		}

		glm::vec3 direction(cos(verticAngle) * sin(horizAngle),sin(verticAngle),cos(verticAngle) * cos(horizAngle));

		glm::vec3 right = glm::vec3(sin(horizAngle - 3.14f/2.0f),0,cos(horizAngle - 3.14f/2.0f));

		glm::vec3 up = glm::cross( right, direction );

		currentTime = glfwGetTime();
		float dTime = float(currentTime - lastTime);
		lastTime = (float)currentTime;

		// Move forward
		if (glfwGetKey( GLFW_KEY_UP ) == GLFW_PRESS){
			pos += direction * dTime* speedo;
		}
		// Move backward
		if (glfwGetKey( GLFW_KEY_DOWN ) == GLFW_PRESS){
			pos -= direction * dTime * speedo;
		}
		// Strafe right
		if (glfwGetKey( GLFW_KEY_RIGHT ) == GLFW_PRESS){
			pos += right * dTime * speedo;
		}
		//Strafe left
		if (glfwGetKey( GLFW_KEY_LEFT ) == GLFW_PRESS){
				pos -= right * dTime * speedo;
		}

		if (glfwGetKey( GLFW_KEY_F1 ) == GLFW_PRESS && go == TRUE){
				go = FALSE;
		}
		else if(glfwGetKey( GLFW_KEY_F1 ) == GLFW_PRESS && go == FALSE)
		{
				go = TRUE;
		}
		
		if (glfwGetKey(GLFW_KEY_SPACE) == GLFW_PRESS){

			if(camera == false)
			{
				camera=true;
				glfwSetMousePos(windowWidth/2, windowHeight/2);
				glfwGetMousePos(&xpos,&ypos);
			}
			else
			{
				camera=false;
				glfwSetMousePos(windowWidth/2, windowHeight/2);
				glfwGetMousePos(&xpos,&ypos);
			}
		}

		glm::mat4 cube1,cube2,cube3 = glm::mat4();
		
		displayCube(cube1,cpos,pos,direction,up);

		//cube1=glm::scale(cube1,glm::vec3(0.1,0.1,0.1));
		//cube2=glm::scale(cube2,glm::vec3(0.1,0.1,0.1));
		//cube3=glm::scale(cube3,glm::vec3(0.1,0.1,0.1));

		cube2=glm::translate(cube2,glm::vec3(3.0,0.0,-3.0));
		cube3=glm::translate(cube3,glm::vec3(-3.0,0.0,-3.0));

		//cube1=glm::rotate(cube1,spee,glm::vec3(0.0,1.0,0.0));
		//cube2=glm::rotate(cube2,spee,glm::vec3(0.0,1.0,0.0));
		//cube3=glm::rotate(cube3,spee,glm::vec3(0.0,1.0,0.0));

		if(go == TRUE)
		{
			spee += 0.1;
		}

		display3Dmodel(cube1,pos,direction,up,0);
		display3Dmodel(cube2,pos,direction,up,1);
		display3Dmodel(cube3,pos,direction,up,2);

		//draw to screen instead of fbo
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	    glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 mat = glm::mat4();

		displayQuad(mat,pos,direction,up);
		
		// drawing the AntWeakBar
		if (wireFrame)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			TwDraw();
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			TwDraw();
		}
        
		fr++;
		ending = glfwGetTime();

		if(ending - starting >= 1)
		{
			rate = fr;
			fr = 0;
			starting = glfwGetTime();
		}

		double CT = glfwGetTime();
		Frames++;
		if(CT -LT >= 1.0)
		{
			FPS = 1000.0 / (double)Frames;
			Frames = 0;
			LT += 1.0f;
		}

        glfwSwapBuffers();
        //check if ESC was pressed
        running=!glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);
    }

	//close OpenGL window and  terminate AntTweakBar and GLFW
    TwTerminate();
    glfwTerminate();
    
    
    exit(EXIT_SUCCESS);
    
}