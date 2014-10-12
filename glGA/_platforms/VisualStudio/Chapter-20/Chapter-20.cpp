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

// number of Squares for Plane
#define NumOfSQ 20

/**********************WATER DEFINES**************************/

#define WMR 128 // water mesh resolution

#define WMS 10.0f // water mesh size

#define MAXY 0.0625f // waves amplitude

#define MAX_WAVES 32

#define M_PI 3.14159265358979323846

#define WNBMTR 512 // water normal bump map texture resolution, must be >= WMR

class CWave
{
public:
	float StartTime, Speed, MaxY, FrequencyMPIM2;
	glm::vec2 Position;

public:
	CWave();
	~CWave();
};

float WaterLevel = 0.5f;

CWave Waves[MAX_WAVES];

int Wave = 0;
int WaterVerticesCount;

GLuint programWater,programBumpMap,programNormalMap;
GLuint vaoWater,vaoBumpMap,vaoNormalMap;
GLuint bufferWater,bufferBumpMap,bufferNormalMap;

GLuint WaterNormalBumpMapTexture;
GLuint ReflectionTexture,RefractionTexture,DepthTexture;

GLuint MVP_uniformWater;

GLuint FBO;

glm::vec3 directionIntrs,upIntrs;

glm::mat4 BiasMatrixInverse = glm::mat4(2.0f,  0.0f,  0.0f, 0.0f, 
										0.0f,  2.0f,  0.0f, 0.0f, 
										0.0f,  0.0f,  2.0f, 0.0f, 
									   -1.0f, -1.0f, -1.0f, 1.0f);

glm::mat4 ProjectionBiasMatrixInverse;

glm::vec4 QuadVertices[] = {
	glm::vec4(-1.0f, -1.0f, 0.0f,1.0f),  // a
	glm::vec4( 1.0f, -1.0f, 0.0f,1.0f),  // b
	glm::vec4( 1.0f,  1.0f, 0.0f,1.0f),  // c
	glm::vec4(-1.0f, -1.0f, 0.0f,1.0f),  // a
	glm::vec4( 1.0f,  1.0f, 0.0f,1.0f),  // c
	glm::vec4(-1.0f,  1.0f, 0.0f,1.0f)   // d
};

struct Lighting
{
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	glm::vec4 position;

	GLfloat constantAttenuation;  
    GLfloat linearAttenuation;     
    GLfloat quadraticAttenuation;
};

Lighting *SceneLight = NULL;

/******************************END****************************/

// global variables
int         windowWidth=1024, windowHeight=768;

GLuint      program3Dmodel;
GLuint      vao3Dmodel;
GLuint      buffer3Dmodel;

GLuint		M_uniform3D      , MVP_uniform3D     , Normal_uniform3D;

GLuint		TextureMatrix_Uniform;
int			timesc = 0;
GLuint		sample;

Texture	    *pTexture		= NULL;
Mesh		*m				= NULL;

const		int			NumVerticesSQ = ( (NumOfSQ) * (NumOfSQ)) * (2) * (3);
const       int         NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

bool        wireFrame = false;
bool		camera    = true;

bool		SYNC	  = true;

typedef     glm::vec4   color4;
typedef     glm::vec4   point4;

int			Index = 0,IndexSQ1 = 0,IndexCube = 0;

//Modelling arrays
point4      points[NumVertices];
glm::vec2	tex_coords[NumVertices];

glm::vec3 pos = glm::vec3( 0.0f, 1.5f , 5.0f );

float Scale = 0.2f;
float Time  = 0.0f;

float angle = 0.0f;

float horizAngle = 3.14f;
float verticAngle = 0.0f;


float speedo = 3.0f;
float mouseSpeedo = 0.005f;

int	  xpos = 0,ypos = 0;

float zNear;
float zFar;
float FOV;
float initialFoV = 45.0f;

// Scene orientation (stored as a quaternion)
float Rotation[] = { 0.0f, 0.0f, 0.0f, 1.0f };

//Plane
point4		planeVertices[NumVerticesSQ];
color4		planeColor[NumVerticesSQ];

void setWaveInShader(int Wave);
void addWaveOnClick();
void initWater();
void displayWater(glm::mat4 md,glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv);
void initBumpMap();
void displayBumpMap();
void initNormalMap();
void displayNormalMap();


void setVSync(bool sync)
{	
	// Function pointer for the wgl extention function we need to enable/disable
	// vsync
	typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALPROC)( int );
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;

	//const char *extensions = (char*)glGetString( GL_EXTENSIONS );

	//if( strstr( extensions, "WGL_EXT_swap_control" ) == 0 )
	//if(glewIsSupported("WGL_EXT_swap_control"))
	//{
	//	std::cout<<"\nWGL_EXT_swap_control Extension is not supported.\n";
	//	return;
	//}
	//else
	//{
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress( "wglSwapIntervalEXT" );

		if( wglSwapIntervalEXT )
			wglSwapIntervalEXT(sync);

		std::cout<<"\nDONE :: "<<sync<<"\n";
	//}
}

// Callback function called by GLFW when window size changes
void GLFWCALL WindowSizeCB(int width, int height)
{
    // Set OpenGL viewport and default camera
    glViewport(0, 0, width, height);

	float aspect = (GLfloat)width / (GLfloat)height;

	windowWidth = width;
	windowHeight = height;

	glBindTexture(GL_TEXTURE_2D, ReflectionTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, RefractionTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, DepthTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Send the new window size to AntTweakBar
    TwWindowSize(width, height);
}

// Callback function called by GLFW when a mouse button is clicked
void GLFWCALL OnMouseButton(int glfwButton, int glfwAction)
{
	if(!TwEventMouseButtonGLFW(glfwButton,glfwAction))
	{
		if(glfwButton == GLFW_MOUSE_BUTTON_1 && glfwAction == GLFW_PRESS)
		{
			std::cout<<"\nLeft Mouse Button pressed!\n";
			addWaveOnClick();
			return;
		}
	}
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

void TW_CALL ResetView(void * a)
{
	Rotation[0] = 0.0f;Rotation[1] = 0.0f;Rotation[2] = 0.0f;Rotation[3] = 1.0f;

	pos = glm::vec3( 0.0f, 1.5f , 5.0f );

	zNear = 0.1f;
	zFar  = 100.0f;
	FOV	  = 45.0f;
	glfwSetMouseWheel(0);

	horizAngle = 3.14f;
	verticAngle = 0.0f;
}

CWave::CWave()
{
	Speed = 1.0f;
	MaxY = 0.03125f;
	FrequencyMPIM2 = 4.0f * (float)M_PI * 2.0;
}

CWave::~CWave()
{
}

// Routine to convert a quaternion to a 4x4 matrix
glm::mat4 ConvertQuaternionToMatrix(float *quat,glm::mat4 mat)
{
    float yy2 = 2.0f * quat[1] * quat[1];
    float xy2 = 2.0f * quat[0] * quat[1];
    float xz2 = 2.0f * quat[0] * quat[2];
    float yz2 = 2.0f * quat[1] * quat[2];
    float zz2 = 2.0f * quat[2] * quat[2];
    float wz2 = 2.0f * quat[3] * quat[2];
    float wy2 = 2.0f * quat[3] * quat[1];
    float wx2 = 2.0f * quat[3] * quat[0];
    float xx2 = 2.0f * quat[0] * quat[0];
    mat[0][0] = - yy2 - zz2 + 1.0f;
    mat[0][1] = xy2 + wz2;
    mat[0][2] = xz2 - wy2;
    mat[0][3] = 0;
    mat[1][0] = xy2 - wz2;
    mat[1][1] = - xx2 - zz2 + 1.0f;
    mat[1][2] = yz2 + wx2;
    mat[1][3] = 0;
    mat[2][0] = xz2 + wy2;
    mat[2][1] = yz2 - wx2;
    mat[2][2] = - xx2 - yy2 + 1.0f;
    mat[2][3] = 0;
    mat[3][0] = mat[3][1] = mat[3][2] = 0;
    mat[3][3] = 1;

	return mat;
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

void addWaveOnClick()
{
	glfwGetMousePos(&xpos,&ypos);

	float s = (float)xpos / (float)(windowWidth - 1);
	float t = 1.0f - (float)ypos / (float)(windowHeight - 1);

	glm::mat4 ViewMatrix = glm::lookAt(pos,pos + directionIntrs,upIntrs);

	float aspect_Intrs = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 ProjectionMatrix = glm::perspective(FOV, aspect_Intrs,zNear,zFar);

	ProjectionBiasMatrixInverse = glm::inverse(ProjectionMatrix) * BiasMatrixInverse;

	glm::vec4 Position = glm::inverse(ViewMatrix) * (ProjectionBiasMatrixInverse * glm::vec4(s, t, 0.5f, 1.0f));
	Position /= Position.w;

	glm::vec3 Ray = glm::normalize(*(glm::vec3*)&Position - pos);

	glm::vec3 Normal = glm::vec3(0.0f, 1.0f, 0.0f);
	float D = -glm::dot(Normal, glm::vec3(0.0f, WaterLevel, 0.0f));

	float NdotR = -glm::dot(Normal, Ray);

	if(NdotR != 0.0f)
	{
		float Distance = (glm::dot(Normal, pos) + D) / NdotR;

		if(Distance > 0.0)
		{
			glm::vec3 Position = Ray * Distance + pos;

			float WMSD2 = WMS / 2.0f;

			if(Position.x >= -WMSD2 && Position.x <= WMSD2 && Position.z >= -WMSD2 && Position.z <= WMSD2)
			{
				Waves[Wave].Position = glm::vec2(Position.x, Position.z);
				Waves[Wave].StartTime = (float)GetTickCount() * 0.001f;
				Waves[Wave].MaxY = MAXY;

				setWaveInShader(Wave++);

				Wave %= MAX_WAVES;
			}
		}
	}
}

void setWaveInShader(int Wave)
{
	std::cout<<"\nWave setted!\n";

	std::string strONE    = "", strTWO = "", strTHREE = "", strFOUR = "", strFIVE = "";
	std::string strWa     = "Waves[";
	std::string strWSTime = "].StartTime";
	std::string strWSpeed = "].Speed";
	std::string strWMaxY  = "].MaxY";
	std::string strWfrency= "].FrequencyMPIM2";
	std::string stwWPos   = "].Position";

	std::string strNum = static_cast<std::ostringstream*>( &(std::ostringstream() << Wave) )->str();
	strONE	 += strWa;
	strONE	 += strNum;
	strONE	 += strWSTime;

	strTWO	 += strWa;
	strTWO	 += strNum;
	strTWO	 += strWSpeed;

	strTHREE += strWa;
	strTHREE += strNum;
	strTHREE += strWMaxY;

	strFOUR += strWa;
	strFOUR += strNum;
	strFOUR += strWfrency;

	strFIVE += strWa;
	strFIVE += strNum;
	strFIVE += stwWPos;

	const GLchar *one   = strONE.c_str();
	const GLchar *two   = strTWO.c_str();
	const GLchar *three = strTHREE.c_str();
	const GLchar *four  = strFOUR.c_str();
	const GLchar *five  = strFIVE.c_str();

	glUseProgram(programBumpMap);
	glUniform1f(glGetUniformLocation(programBumpMap,one), Waves[Wave].StartTime);
	glUniform1f(glGetUniformLocation(programBumpMap,two), Waves[Wave].Speed);
	glUniform1f(glGetUniformLocation(programBumpMap,three), Waves[Wave].MaxY);
	glUniform1f(glGetUniformLocation(programBumpMap,four), Waves[Wave].FrequencyMPIM2);
	glUniform2fv(glGetUniformLocation(programBumpMap,five), 1,glm::value_ptr(Waves[Wave].Position));
	glUseProgram(0);
}

void initWater()
{
	WaterVerticesCount = WMR * WMR * 6;

	glm::vec2 *WaterVertices = new glm::vec2[WaterVerticesCount];

	float MWMSD2 = -WMS / 2.0f, WMSDWMR = WMS / (float)WMR;

	int i = 0;
	
	for(int y = 0; y < WMR; y++)
	{
		for(int x = 0; x < WMR; x++)
		{
			WaterVertices[i].x = MWMSD2 + x * WMSDWMR;				// a
			WaterVertices[i++].y = MWMSD2 + y * WMSDWMR;

			WaterVertices[i].x = MWMSD2 + (x + 1) * WMSDWMR;		// b
			WaterVertices[i++].y = MWMSD2 + y * WMSDWMR;

			WaterVertices[i].x = MWMSD2 + (x + 1) * WMSDWMR;		// c
			WaterVertices[i++].y = MWMSD2 + (y + 1) * WMSDWMR;


			WaterVertices[i].x = MWMSD2 + x * WMSDWMR;				// a
			WaterVertices[i++].y = MWMSD2 + y * WMSDWMR;

			WaterVertices[i].x = MWMSD2 + (x + 1) * WMSDWMR;		// c
			WaterVertices[i++].y = MWMSD2 + (y + 1) * WMSDWMR;

			WaterVertices[i].x = MWMSD2 + x * WMSDWMR;				// d
			WaterVertices[i++].y = MWMSD2 + (y + 1) * WMSDWMR;
		}
	}

	glGenVertexArrays(1, &vaoWater);
	glBindVertexArray(vaoWater);

	//Load shaders and use the resulting shader program
	programWater = LoadShaders( "./Shaders/vshaderWater.vert", "./Shaders/fshaderWater.frag" );

    glUseProgram( programWater );

	glGenBuffers( 1, &bufferWater );
	glBindBuffer( GL_ARRAY_BUFFER, bufferWater );
	glBufferData( GL_ARRAY_BUFFER, (WaterVerticesCount * 2 * 4),WaterVertices, GL_STATIC_DRAW );
	
	// set up vertex arrays
    GLuint vPosition = glGetAttribLocation( programWater, "MCVertex" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );

	glUniform1f(glGetUniformLocation(programWater, "ODWMS"), 1.0f / WMS);

	glUniform4fv(glGetUniformLocation(programWater, "SceneLight.specular"),1,glm::value_ptr(SceneLight->specular));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	delete [] WaterVertices;
}

void displayWater(glm::mat4 md,glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, WaterNormalBumpMapTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ReflectionTexture);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, RefractionTexture);

	glUseProgram(programWater);
	glBindVertexArray(vaoWater);

	glEnable(GL_DEPTH_TEST);
    glPushAttrib(GL_ALL_ATTRIB_BITS);	
	
    if (wireFrame)
	{
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	MVP_uniformWater = glGetUniformLocation(programWater, "MVPMatrix");

	// Calculation of ModelView Matrix
	glm::mat4 model_mat_water =  glm::mat4();
	glm::mat4 view_mat_water = glm::lookAt(positionv,positionv + directionv,upv);
	glm::mat4 MV_mat_water = view_mat_water * model_mat_water;

	// Calculation of ModelViewProjection Matrix
	float aspect_water = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 projection_mat_water = glm::perspective(FOV, aspect_water,zNear,zFar);
	glm::mat4 MVP_mat_water = projection_mat_water * MV_mat_water;
	glUniformMatrix4fv(MVP_uniformWater, 1, GL_FALSE, glm::value_ptr(MVP_mat_water));

	glUniform1i(glGetUniformLocation(programWater, "WaterNormalBumpMapTexture"), 0);
	glUniform1i(glGetUniformLocation(programWater, "ReflectionTexture"), 1);
	glUniform1i(glGetUniformLocation(programWater, "RefractionTexture"), 2);

	glUniform1f(glGetUniformLocation(programWater, "WaterLevel"),WaterLevel);
	glUniform3fv(glGetUniformLocation(programWater, "CameraPosition"),1,glm::value_ptr(pos));
	glUniformMatrix4fv(glGetUniformLocation(programWater, "ViewProjectionBiasInverse"), 1, GL_FALSE, glm::value_ptr(glm::inverse(view_mat_water) * ProjectionBiasMatrixInverse));

	glUniform4fv(glGetUniformLocation(programWater, "SceneLight.position"),1,glm::value_ptr(SceneLight->position));
	glUniform4fv(glGetUniformLocation(programWater, "SceneLight.ambient"),1,glm::value_ptr(SceneLight->ambient));
	glUniform4fv(glGetUniformLocation(programWater, "SceneLight.diffuse"),1,glm::value_ptr(SceneLight->diffuse));
	glUniform1f(glGetUniformLocation(programWater, "SceneLight.constantAttenuation"),SceneLight->constantAttenuation);
	glUniform1f(glGetUniformLocation(programWater, "SceneLight.linearAttenuation"),SceneLight->linearAttenuation);
	glUniform1f(glGetUniformLocation(programWater, "SceneLight.quadraticAttenuation"),SceneLight->quadraticAttenuation);

	glDrawArrays( GL_TRIANGLES, 0, WaterVerticesCount);
    
	glDisable(GL_DEPTH_TEST);
    glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void initBumpMap()
{
	//generate and bind a VAO for the 3D axes
	glGenVertexArrays(1, &vaoBumpMap);
    glBindVertexArray(vaoBumpMap);

	// Load shaders and use the resulting shader program
	programBumpMap = LoadShaders( "./Shaders/vshaderBump.vert", "./Shaders/fshaderBump.frag" );
    glUseProgram( programBumpMap );

	// Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
	glGenBuffers( 1, &bufferBumpMap );
    glBindBuffer( GL_ARRAY_BUFFER, bufferBumpMap );
    glBufferData( GL_ARRAY_BUFFER, sizeof(QuadVertices),NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(QuadVertices), QuadVertices );
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( programBumpMap, "MCvertex" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );

	std::string strONE    = "", strTWO = "", strTHREE = "", strFOUR = "", strFIVE = "";
	std::string strWa     = "Waves[";
	std::string strWSTime = "].StartTime";
	std::string strWSpeed = "].Speed";
	std::string strWMaxY  = "].MaxY";
	std::string strWfrency= "].FrequencyMPIM2";
	std::string stwWPos   = "].Position";

	for(int Wave = 0;Wave < MAX_WAVES; Wave++)
	{
		std::string strNum = static_cast<std::ostringstream*>( &(std::ostringstream() << Wave) )->str();
		strONE	 += strWa;
		strONE	 += strNum;
		strONE	 += strWSTime;

		strTWO	 += strWa;
		strTWO	 += strNum;
		strTWO	 += strWSpeed;

		strTHREE += strWa;
		strTHREE += strNum;
		strTHREE += strWMaxY;

		strFOUR += strWa;
		strFOUR += strNum;
		strFOUR += strWfrency;

		strFIVE += strWa;
		strFIVE += strNum;
		strFIVE += stwWPos;

		const GLchar *one   = strONE.c_str();
		const GLchar *two   = strTWO.c_str();
		const GLchar *three = strTHREE.c_str();
		const GLchar *four  = strFOUR.c_str();
		const GLchar *five  = strFIVE.c_str();

		glUniform1f(glGetUniformLocation(programBumpMap,one), Waves[Wave].StartTime);
		glUniform1f(glGetUniformLocation(programBumpMap,two), Waves[Wave].Speed);
		glUniform1f(glGetUniformLocation(programBumpMap,three), Waves[Wave].MaxY);
		glUniform1f(glGetUniformLocation(programBumpMap,four), Waves[Wave].FrequencyMPIM2);
		glUniform2fv(glGetUniformLocation(programBumpMap,five), 1,glm::value_ptr(Waves[Wave].Position));

		strONE   = "";
		strTWO   = "";
		strTHREE = "";
		strFOUR  = "";
		strFIVE  = "";
	}

	glUniform1f(glGetUniformLocation(programBumpMap, "WMS"), WMS);				// Water Mesh Size
	glUniform1f(glGetUniformLocation(programBumpMap, "WMSD2"), WMS / 2.0f);		// Water Mesh Size Divided by 2.0f
    
    // only one VAO can be bound at a time, so disable it to avoid altering it accidentally
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void displayBumpMap()
{
	glUseProgram(programBumpMap);
	glBindVertexArray(vaoBumpMap);

	glDisable(GL_CULL_FACE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

	glUniform1f(glGetUniformLocation(programBumpMap,"Time"),(float)GetTickCount() * 0.001f);


	glDrawArrays( GL_TRIANGLES, 0, 6);
    
    glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);
}

void initNormalMap()
{
	//generate and bind a VAO for the 3D axes
	glGenVertexArrays(1, &vaoNormalMap);
    glBindVertexArray(vaoNormalMap);

	glGenTextures(1, &WaterNormalBumpMapTexture);

	glBindTexture(GL_TEXTURE_2D, WaterNormalBumpMapTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, WNBMTR, WNBMTR, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Load shaders and use the resulting shader program
	programNormalMap = LoadShaders( "./Shaders/vshaderNormal.vert", "./Shaders/fshaderNormal.frag" );
    glUseProgram( programNormalMap );

	// Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
	glGenBuffers( 1, &bufferNormalMap );
    glBindBuffer( GL_ARRAY_BUFFER, bufferNormalMap );
    glBufferData( GL_ARRAY_BUFFER, sizeof(QuadVertices),NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(QuadVertices), QuadVertices );
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( programNormalMap, "MCvertex" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );

	glUniform1f(glGetUniformLocation(programNormalMap, "ODWNBMTR"), 1.0f / WNBMTR);
	glUniform1f(glGetUniformLocation(programNormalMap, "WMSDWNBMTRM2"), WMS / WNBMTR * 2.0f);
    
    // only one VAO can be bound at a time, so disable it to avoid altering it accidentally
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void displayNormalMap()
{
	glBindTexture(GL_TEXTURE_2D, WaterNormalBumpMapTexture);
	glUseProgram(programNormalMap);
	glBindVertexArray(vaoNormalMap);

	glDisable(GL_CULL_FACE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDrawArrays( GL_TRIANGLES, 0, 6);
    
    glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void init3Dmodel()
{

	glGenVertexArrays(1, &vao3Dmodel);
    glBindVertexArray(vao3Dmodel);
	GLuint m_Buffers[4];
	m = new Mesh();
	m->loadMesh("./Models/Cruisership2012/Cruiser 2012.obj");

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
	glBufferSubData( GL_ARRAY_BUFFER, (sizeof(m->Positions[0]) * m->Positions.size()) + (sizeof(m->Normals[0]) * m->Normals.size()), (sizeof(m->TexCoords[0]) * m->TexCoords.size()), &m->TexCoords[0] );
	
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

	GLuint vTexCoord = glGetAttribLocation( program3Dmodel, "TexCoord0" ); 
    glEnableVertexAttribArray( vTexCoord );
	glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET((sizeof(m->Positions[0]) * m->Positions.size()) + (sizeof(m->Normals[0]) * m->Normals.size())) );

	glUniform1i(glGetUniformLocation(program3Dmodel, "Texture"), 0);
	glUniform1i(glGetUniformLocation(program3Dmodel, "WaterNormalBumpMapTexture"), 1);
	glUniform1f(glGetUniformLocation(program3Dmodel, "ODWMS"), 1.0f / WMS);

	glUniform4fv(glGetUniformLocation(program3Dmodel, "SceneLight.position"),1,glm::value_ptr(SceneLight->position));

	glUniform4fv(glGetUniformLocation(program3Dmodel, "SceneLight.ambient"),1,glm::value_ptr(SceneLight->ambient));
	glUniform4fv(glGetUniformLocation(program3Dmodel, "SceneLight.diffuse"),1,glm::value_ptr(SceneLight->diffuse));
	glUniform4fv(glGetUniformLocation(program3Dmodel, "SceneLight.specular"),1,glm::value_ptr(SceneLight->specular));

	glUniform1f(glGetUniformLocation(program3Dmodel, "SceneLight.constantAttenuation"), SceneLight->constantAttenuation);
	glUniform1f(glGetUniformLocation(program3Dmodel, "SceneLight.linearAttenuation"), SceneLight->linearAttenuation);
	glUniform1f(glGetUniformLocation(program3Dmodel, "SceneLight.quadraticAttenuation"), SceneLight->quadraticAttenuation);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void display3Dmodel(glm::mat4 tsl,glm::mat4 Notsl,glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv,int ClipType)
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, WaterNormalBumpMapTexture);

	glUseProgram(program3Dmodel);
	glBindVertexArray(vao3Dmodel);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if (wireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	MVP_uniform3D    = glGetUniformLocation(program3Dmodel, "MVPMatrix");
	M_uniform3D     = glGetUniformLocation(program3Dmodel, "MMatrix" );
	Normal_uniform3D = glGetUniformLocation(program3Dmodel, "NormalMatrix");

	//Calculation of ModelView Matrix
	glm::mat4 model_mat = tsl;
	glm::mat4 view_mat = glm::lookAt(positionv,positionv + directionv,upv);

	glm::mat4 MV_mat;

	if(Notsl != glm::mat4())
		MV_mat = view_mat * Notsl;
	else
	    MV_mat = view_mat * model_mat;

	glUniformMatrix4fv(M_uniform3D,1, GL_FALSE, glm::value_ptr(model_mat)); 

	//Calculation of Normal Matrix
	glm::mat3 Normal_mat = glm::transpose(glm::inverse(glm::mat3(MV_mat)));
	glUniformMatrix3fv(Normal_uniform3D,1, GL_FALSE, glm::value_ptr(Normal_mat));

	//Calculation of ModelViewProjection Matrix
	float aspect = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 projection_mat = glm::perspective(FOV, aspect,0.1f,100.0f);
	glm::mat4 MVP_mat = projection_mat * MV_mat;
	glUniformMatrix4fv(MVP_uniform3D, 1, GL_FALSE, glm::value_ptr(MVP_mat));

	glUniform1i(glGetUniformLocation(program3Dmodel, "ClipType"),ClipType);
	glUniform1f(glGetUniformLocation(program3Dmodel, "WaterLevel"),WaterLevel);

	glUniform4fv(glGetUniformLocation(program3Dmodel, "SceneLight.position"),1,glm::value_ptr(SceneLight->position));

	m->render(); 

	if(timesc < 1)
	{
		checkActiveUniforms();
	}
	timesc++;

	glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void initSceneLighting()
{
	SceneLight = new Lighting();

	SceneLight->position = glm::vec4(-1.88f , 3.74f,  4.18f, 1.0f);

	SceneLight->ambient  = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
	SceneLight->diffuse  = glm::vec4(0.75f, 0.75f, 0.75f, 1.0f);
	SceneLight->specular = glm::vec4(1.0f , 1.0f , 1.0f , 1.0f);

	SceneLight->constantAttenuation  = 1.0f;
	SceneLight->linearAttenuation    = 1.0f / 128.0f;
	SceneLight->quadraticAttenuation = 1.0f / 256.0f;
}

int main (int argc, const char * argv[])
{

	TwBar *myBar;
	float bgColor[] = { 0.0f, 0.0f, 0.0f, 0.1f };

	glm::mat4 mat;
	float axis[] = { 0.7f, 0.7f, 0.7f }; // initial model rotation
    float angle = 0.8f;

	double FT  = 0;
	double FPS = 0;

	double starting = 0.0;
	double ending   = 0.0;
	int rate = 0;
	int fr = 0;

	zNear = 0.1f;
	zFar  = 100.0f;
	FOV   = 45.0f; 

	// Current time
	double time = 0;

	initSceneLighting();

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
    glfwSetWindowTitle("Chapter-20");

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
   
	std::ostringstream stream1,stream2;

	stream1 << glGetString(GL_VENDOR);
	stream2 << glGetString(GL_RENDERER);

	std::string vendor ="Title : Chapter-20   Vendor : " + stream1.str() + "   Renderer : " +stream2.str();

	const char *tit = vendor.c_str();
	glfwSetWindowTitle(tit);
    // Set GLFW event callbacks
    // - Redirect window size changes to the callback function WindowSizeCB
    glfwSetWindowSizeCallback(WindowSizeCB);
    
    // - Directly redirect GLFW mouse button events to AntTweakBar
    glfwSetMouseButtonCallback(OnMouseButton);
    
    // - Directly redirect GLFW mouse position events to AntTweakBar
    glfwSetMousePosCallback((GLFWmouseposfun)TwEventMousePosGLFW);
    
    // - Directly redirect GLFW mouse wheel events to AntTweakBar
    glfwSetMouseWheelCallback((GLFWmousewheelfun)TwEventMouseWheelGLFW);
    
    // - Directly redirect GLFW key events to AntTweakBar
    glfwSetKeyCallback((GLFWkeyfun)TwEventKeyGLFW);
    
    // - Directly redirect GLFW char events to AntTweakBar
    glfwSetCharCallback((GLFWcharfun)TwEventCharGLFW);


	TwDefine("TweakBar label='Main TweakBar' alpha=0 help='Use this bar to control the objects of the scene.' ");

	// Add 'wire' to 'myBar': it is a modifable variable of type TW_TYPE_BOOL32 (32 bits boolean). Its key shortcut is [w].
    TwAddVarRW(myBar, "wireframe mode", TW_TYPE_BOOL32, &wireFrame," label='Wireframe mode' key=w help='Toggle wireframe display mode.' ");

	// Add 'bgColor' to 'myBar': it is a modifable variable of type TW_TYPE_COLOR3F (3 floats color)
    TwAddVarRW(myBar, "bgColor", TW_TYPE_COLOR3F, &bgColor, " label='Background color' ");

	// Add 'Rotation' to 'myBar': this is a variable of type TW_TYPE_QUAT4F which defines the scene's orientation
    TwAddVarRW(myBar, "SceneRotation", TW_TYPE_QUAT4F, &Rotation," label='Scene rotation' opened=true help='Change the scenes orientation.' ");

	TwAddButton(myBar, "Reset", ResetView,NULL," label='Reset View' ");

	TwAddVarRW(myBar, "Near Clip Plane", TW_TYPE_FLOAT, &zNear,"min=0.5 max=100 step=0.5 label='Near Clip' group='Projection Properties'");

	TwAddVarRW(myBar, "Far Clip Plane", TW_TYPE_FLOAT, &zFar," min=0.5 max=1000 step=0.5 label='Far Clip' group='Projection Properties'");

	TwAddVarRW(myBar, "Field of View", TW_TYPE_FLOAT, &FOV," label='FoV' readonly=true group='Projection Properties'");

	TwAddVarRW(myBar, "MS per 1 Frame" , TW_TYPE_DOUBLE, &FPS, "label='MS per 1 Frame' readonly=true group='Frame Rate'");

	TwAddVarRW(myBar, "Frames Per Second" , TW_TYPE_INT32, &rate, "label='FPS' readonly=true group='Frame Rate'");

	TwAddVarRW(myBar, "vSYNC" , TW_TYPE_BOOL8, &SYNC, "label='vSync' readonly=true group='Frame Rate'");

	TwAddVarRW(myBar, "LightDir", TW_TYPE_DIR3F, &(SceneLight->position),"label='LightDir' ");

	
	glGenTextures(1, &ReflectionTexture);
	glGenTextures(1, &RefractionTexture);
	glGenTextures(1, &DepthTexture);

	glBindTexture(GL_TEXTURE_2D, ReflectionTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, RefractionTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, DepthTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	initWater();

	initBumpMap();

	initNormalMap();

	init3Dmodel();

	glGenFramebuffersEXT(1, &FBO);

	GLfloat rat = 0.001f;

	if(SYNC == false)
	{
		rat = 0.001f;
	}
	else
	{
		rat = 0.01f;
	}

	// Initialize time
    time = glfwGetTime();
	double currentTime;
	float lastTime = 0.0f;

	int Frames = 0;
	double LT = glfwGetTime();
	starting = glfwGetTime();

	setVSync(SYNC);

	while (running) {
		
		glViewport(0, 0, WNBMTR, WNBMTR);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, WaterNormalBumpMapTexture, 0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);

		displayBumpMap();
		displayNormalMap();

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		FOV = initialFoV - 5 * glfwGetMouseWheel();

		if(camera == true)
		{
			glfwGetMousePos(&xpos,&ypos);
			glfwSetMousePos(windowWidth/2, windowHeight/2);
		
			horizAngle  += mouseSpeedo * float(windowWidth/2 - xpos );
			verticAngle += mouseSpeedo * float( windowHeight/2 - ypos );
		}

		glm::vec3 direction(cos(verticAngle) * sin(horizAngle),sin(verticAngle),cos(verticAngle) * cos(horizAngle));

		directionIntrs = direction;

		glm::vec3 right = glm::vec3(sin(horizAngle - 3.14f/2.0f),0,cos(horizAngle - 3.14f/2.0f));

		glm::vec3 up = glm::cross( right, direction );

		upIntrs = up;		
		
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

		mat = ConvertQuaternionToMatrix(Rotation, mat);

		glm::mat4 boattr = glm::translate(glm::mat4(),glm::vec3(0.0,0.4,2.2));

		glm::mat4 scaledown = glm::scale(mat,glm::vec3(0.007,0.007,0.007));

		scaledown = glm::rotate(scaledown,45.0f,glm::vec3(0.0,1.0,0.0));

		boattr *= scaledown;

		glViewport(0, 0, windowWidth, windowHeight);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, ReflectionTexture, 0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, DepthTexture, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		display3Dmodel(boattr,glm::mat4(),pos,direction,up,1);

		glm::mat4 Nboattr  = glm::scale(boattr,glm::vec3(1.0, -1.0, 1.0));

		glCullFace(GL_FRONT);

		display3Dmodel(boattr,Nboattr,pos,direction,up,1);

		glCullFace(GL_BACK);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		// render scene / refraction texture --------------------------------------------------------------------------------------

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		display3Dmodel(boattr,glm::mat4(),pos,direction,up,0);
		
		glBindTexture(GL_TEXTURE_2D, RefractionTexture);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, windowWidth, windowHeight);
		glBindTexture(GL_TEXTURE_2D, 0);

		displayWater(mat,pos,direction,up);

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
