// basic STL streams
#include <iostream>

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

// global variables
int         windowWidth=1024, windowHeight=768;

GLuint      program3Dmodel,program;
GLuint      vao3Dmodel,vao;
GLuint      buffer3Dmodel,buffer;

GLuint		MV_uniform3D, MVP_uniform3D,Normal_uniform3D,MV_uniform ,Normal_uniform;
GLuint		MVP_uniform; 

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

glm::vec3 BaseColor = glm::vec3(1.0f,1.0f,1.0f);

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

unsigned int cubeMap;

unsigned int LoadMap(std::string* filenames)
{
	unsigned int tex;
	glGenTextures(1,&tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP,tex);

	for(int i=0;i<6;i++)
	{
		Texture	*cTexture = new Texture(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,filenames[i]);
		cTexture->loadTexture();
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
	return tex;
}

// Callback function called by GLFW when window size changes
void GLFWCALL WindowSizeCB(int width, int height)
{
    // Set OpenGL viewport and default camera
    glViewport(0, 0, width, height);

	float aspect = (GLfloat)width / (GLfloat)height;

	windowWidth = width;
	windowHeight = height;

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
	glGetProgramiv( program, GL_ACTIVE_UNIFORM_MAX_LENGTH,&maxLen);
	glGetProgramiv( program, GL_ACTIVE_UNIFORMS,&nUniforms);
	GLchar * name = (GLchar *) malloc( maxLen );
	GLint size, location;
	GLsizei written;
	GLenum type;

	printf(" Location | Name\n");
	printf("------------------------------------------------\n");

	for( int i = 0; i < nUniforms; ++i ) {

		glGetActiveUniform( program, i, maxLen, &written,&size, &type, name );
		location = glGetUniformLocation(program, name);
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

	std::string filename[6];
	filename[0]="./Textures/SkyTex/right.bmp";
	filename[1]="./Textures/SkyTex/left.bmp";
	filename[2]="./Textures/SkyTex/top.bmp";
	filename[3]="./Textures/SkyTex/bottom.bmp";
	filename[4]="./Textures/SkyTex/front.bmp";
	filename[5]="./Textures/SkyTex/back.bmp";

	cubeMap=LoadMap(filename);

    colorcube();
    
    // Load shaders and use the resulting shader program
    program = LoadShaders( "./Shaders/vshaderC1.vert", "./Shaders/fshaderCube2.frag" );
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
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

    glDrawArrays( GL_TRIANGLES, 0, NumVertices );

   
	if(timesc < 1)
		checkActiveUniforms();
	timesc++;


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
	m->loadMesh("./Models/box1/models/CargoCube01.dae");  
	//m->loadMesh("./Models/skull.obj");

    //Load shaders and use the resulting shader program
	program3Dmodel = LoadShaders( "./Shaders/vshader3D.vert", "./Shaders/fshader3D.frag" );

    glUseProgram( program3Dmodel );

	// Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
	glGenBuffers( 1, &buffer3Dmodel );
	glBindBuffer( GL_ARRAY_BUFFER, buffer3Dmodel );
	glBufferData(GL_ARRAY_BUFFER, (sizeof(m->Positions[0]) * m->Positions.size()) + (sizeof(m->Normals[0]) * m->Normals.size()) + (sizeof(m->TexCoords[0]) * m->TexCoords.size()), NULL, GL_STATIC_DRAW);
	glBufferSubData( GL_ARRAY_BUFFER, 0, (sizeof(m->Positions[0]) * m->Positions.size()), &m->Positions[0] );
	glBufferSubData( GL_ARRAY_BUFFER, (sizeof(m->Positions[0]) * m->Positions.size()), (sizeof(m->Normals[0]) * m->Normals.size()), &m->Normals[0] );
	glBufferSubData( GL_ARRAY_BUFFER, (sizeof(m->Positions[0]) * m->Positions.size()) + (sizeof(m->Normals[0]) * m->Normals.size()),(sizeof(m->TexCoords[0]) * m->TexCoords.size()), &m->TexCoords[0] );
	
	glGenBuffers( 1, &m_Buffers[0] );
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

void display3Dmodel(glm::mat4 tsl,glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv)
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
	MVP_uniform3D = glGetUniformLocation(program3Dmodel, "MVP_mat");
	Normal_uniform3D = glGetUniformLocation(program3Dmodel, "Normal_mat");

	//Calculation of ModelView Matrix
	glm::mat4 model_mat = tsl;
	glm::mat4 view_mat = glm::lookAt(positionv,positionv + directionv,upv);

	glm::mat4 MV_mat = view_mat * model_mat;
	glUniformMatrix4fv(MV_uniform3D,1, GL_FALSE, glm::value_ptr(MV_mat)); 

	glUniform3fv(glGetUniformLocation(program3Dmodel,"LightPosition"),1,glm::value_ptr(LightPos));
	glUniform3fv(glGetUniformLocation(program3Dmodel,"BaseColor"),1,glm::value_ptr(BaseColor));
	glUniform1f(glGetUniformLocation(program3Dmodel,"MixRatio"),MixRatio);

	//Calculation of Normal Matrix
	glm::mat3 Normal_mat = glm::transpose(glm::inverse(glm::mat3(MV_mat)));
	glUniformMatrix3fv(Normal_uniform3D,1, GL_FALSE, glm::value_ptr(Normal_mat));

	//Calculation of ModelViewProjection Matrix
	float aspect = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 projection_mat = glm::perspective(45.0f, aspect,0.1f,100.0f);
	glm::mat4 MVP_mat = projection_mat * MV_mat;
	glUniformMatrix4fv(MVP_uniform3D, 1, GL_FALSE, glm::value_ptr(MVP_mat));


	glUniform1i(glGetUniformLocation(program3Dmodel, "CubeMapTex"), 0);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	m->render(); 

	//if(timesc < 1)
		//checkActiveUniforms();
	//timesc++;
	glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);
}

int main (int argc, const char * argv[])
{

	TwBar *myBar;
	float bgColor[] = { 0.0f, 0.0f, 0.0f, 0.1f };

	// Current time
	double time = 0;

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
    glfwSetWindowTitle("Chapter-10");

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

	TwDefine("TweakBar label='Main TweakBar' alpha=0 help='Use this bar to control the objects of the scene.' ");

	// Add 'wire' to 'bar': it is a modifable variable of type TW_TYPE_BOOL32 (32 bits boolean). Its key shortcut is [w].
    TwAddVarRW(myBar, "wireframe", TW_TYPE_BOOL32, &wireFrame," label='Wireframe mode' key=w help='Toggle wireframe display mode.' ");

	// Add 'bgColor' to 'bar': it is a modifable variable of type TW_TYPE_COLOR3F (3 floats color)
    TwAddVarRW(myBar, "bgColor", TW_TYPE_COLOR3F, &bgColor, " label='Background color' ");

	TwAddVarRW(myBar, "Light Position", TW_TYPE_DIR3F, &LightPos," label='Light Position' ");

	TwAddVarRW(myBar, "Base Color", TW_TYPE_COLOR3F, &BaseColor," label='Base Color' ");

	TwAddVarRW(myBar, "Mix Ration", TW_TYPE_FLOAT, &MixRatio, "min=0.0 max=0.9 step=0.005 label='Mix Ratio' ");

	 // Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	initCube();
	//initPyramid();
	init3Dmodel();

	// Initialize time
    time = glfwGetTime();
	double currentTime;
	float lastTime = 0.0f;

	while (running) {

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

		glm::mat4 cube1 = glm::mat4();
		displayCube(cube1,cpos,pos,direction,up);

		//displayPyramid(pos,direction,up);
		display3Dmodel(cube1,pos,direction,up);
		
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
        
        glfwSwapBuffers();
        //check if ESC was pressed
        running=!glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);
    }

	//close OpenGL window and  terminate AntTweakBar and GLFW
    TwTerminate();
    glfwTerminate();
    
    
    exit(EXIT_SUCCESS);
    
}