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

// number of Squares for Plane
#define NumOfSQ 50

// global variables
int         windowWidth=1024, windowHeight=768;

GLuint      programX, programY, programZ, programPlane, program3Dmodel, programPyramid;
GLuint      vao, vaoX, vaoY, vaoZ, vaoPlane, vao3Dmodel, vaoPyramid;
GLuint      bufferX, bufferY, bufferZ, bufferPlane, buffer3Dmodel, bufferPyramid;

GLuint      eyes_uniform;
GLuint		MV_uniformX       , MVP_uniformX;
GLuint		MV_uniformY       , MVP_uniformY;
GLuint		MV_uniformZ       , MVP_uniformZ;
GLuint		MV_uniformPlane   , MVP_uniformPlane   , Normal_uniformPlane, V_uniformPlane;
GLuint		MV_uniform3D      , MVP_uniform3D      , Normal_uniform3D   , V_uniform3D;
GLuint		MV_uniformPyramid , MVP_uniform_Pyramid; 
GLuint		MVlight;
GLuint		TextureMatrix_Uniform;
int timesc = 0;
GLuint		gSampler1,gSampler;

Texture	    *pTexture = NULL;
Mesh		*m		  = NULL;

const		int			NumVerticesl = 2;
const		int			NumVerticesSQ = ( (NumOfSQ) * (NumOfSQ)) * (2) * (3);

bool        wireFrame = false;
bool		camera    = true;

typedef     glm::vec4   color4;
typedef     glm::vec4   point4;

int			IndexSQ = 0;
int			IndexPyramid = 0;

//Modelling arrays
point4		pointPyramid[18];
color4		colorPyramid[18];
point4		pointsq[NumVerticesSQ];
color4		colorsq[NumVerticesSQ];
glm::vec3	normalsq[NumVerticesSQ];
glm::vec4	tex_coords[NumVerticesSQ];

glm::vec3 pos = glm::vec3( 3.0f, 4.0f , 10.0f );

float horizAngle = 3.14f;
float verticAngle = 0.0f;


float speedo = 3.0f;
float mouseSpeedo = 0.005f;

int	  xpos = 0,ypos = 0;

struct Lights
{
	GLint active;

	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat position[4];

	glm::vec3 spotDirection;  
    GLfloat spotExponent;        
    GLfloat spotCutoff;                                     
                              // (range: [0.0,90.0], 180.0)   
    GLfloat spotCosCutoff;      // Derived: cos(Crli)                 
                              // (range: [1.0,0.0],-1.0)

    GLfloat constantAttenuation;  
    GLfloat linearAttenuation;     
    GLfloat quadraticAttenuation;
};

struct MaterialProperties
{

   GLfloat emission[4];   
   GLfloat ambient[4];   
   GLfloat diffuse[4];   
   GLfloat specular[4];   
   GLfloat shininess; 
};

struct MaterialProperties *frontMaterial,*backMaterial;

struct FogProperties
{
	int   enable;
	float density;
	float start;
	float end;

	glm::vec4 color;

	enum equation{FOG_EQUATION_LINEAR , FOG_EQUATION_EXP, FOG_EQUATION_EXP2};
	equation pick;
	float scale;
};

struct FogProperties *Fog;

int				    maxLights = 0;                  // maximum number of dynamic lights allowed by the graphic card
Lights			   *lights = NULL;						// array of lights
MaterialProperties *materials = NULL;
TwBar			   *light_Material_Bar = NULL,*fog_properties_bar = NULL; 

//Plane
point4		planeVertices[NumVerticesSQ];
color4		planeColor[NumVerticesSQ];

//Vertices of a XYZ axis
// X - axis
point4 Xvertices[2] = {
	point4( 0.0, 0.0 , 0.0 , 1.0),
	point4( 5.0, 0.0 , 0.0 , 1.0)  
};

color4 Xvertex_color[2] = {
	color4( 1.0, 0.0, 0.0, 1.0),
	color4( 1.0, 0.0, 0.0, 1.0)
};

// Y - axis
point4 Yvertices[2] = {
	point4( 0.0, 0.0, 0.0, 1.0),
	point4( 0.0, 5.0, 0.0, 1.0)
};

color4 Yvertex_color[2] = {
	color4( 0.0, 1.0, 0.0, 1.0),
	color4( 0.0, 1.0, 0.0, 1.0)
};

// Z - axis
point4 Zvertices[2] = {
	point4( 0.0, 0.0, 0.0, 1.0),
	point4( 0.0, 0.0, 5.0, 1.0)
};

color4 Zvertex_color[2] = {
	color4( 0.0, 0.0, 1.0, 1.0),
	color4( 0.0, 0.0, 1.0, 1.0)
};

point4 Pyramid[5] = {
	point4(-1.0, 6.0, 1.0, 1.0),
	point4(-1.0, 6.0,-1.0, 1.0),
	point4( 1.0, 6.0,-1.0, 1.0),
	point4( 1.0, 6.0, 1.0, 1.0),
	point4( 0.0, 9.0, 0.0, 1.0)
};

color4 PyramidsColor[5] = {
	color4(0.1, 1.0, 0.1, 1.0),
	color4(0.1, 1.0, 0.1, 1.0),
	color4(0.1, 1.0, 0.1, 1.0),
	color4(0.1, 1.0, 0.1, 1.0),
	color4(0.1, 1.0, 0.1, 1.0)
};

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

int getActiveLights(Lights *ptr)
{
	int active = 0;

	if(ptr == NULL)
	{
		return active;
	}
	else
	{
		for(int i=0;i<maxLights;i++)
		{
			if(ptr[i].active == 1)
			{
				active++;
			}
		}

		return active;
	}
}

void create_Light_Material_bar()
{
	// Create a new tweak bar and change its label, position and transparency
	light_Material_Bar = TwNewBar("Lights-Materials");
    TwDefine(" Lights-Materials label='Lights-Materials TweakBar' position='16 380' alpha=0 help='Use this bar to edit the lights and materials in the scene.' ");

	// Define a new struct type: light variables are embedded in this structure
    TwStructMember lightMembers[] = // array used to describe tweakable variables of the Light structure
    {
		{ "Active",    TW_TYPE_INT32  , offsetof(Lights, active),    "min = 0 max = 1 help='Enable/disable the light.' " },   // Lights::active is a C++ integer value
        { "Ambient",   TW_TYPE_COLOR4F, offsetof(Lights, ambient),   " noalpha help='Ambient color.' " },      // Lights::ambient is represented by 4 floats, but alpha channel should be ignored
		{ "Diffuse",   TW_TYPE_COLOR4F, offsetof(Lights, diffuse),   " noalpha help='Diffuse color.' " },      // Lights::diffuse is represented by 4 floats, but alpha channel should be ignored
		{ "Specular",  TW_TYPE_COLOR4F, offsetof(Lights, specular),  " noalpha help='Specular color.' " },     // Lights::specular is represented by 4 floats, but alpha channel should be ignored
		{ "Position",  TW_TYPE_DIR3F,   offsetof(Lights, position),  " help='Lights position.' " },
		{ "SpotCutOff",TW_TYPE_FLOAT,   offsetof(Lights, spotCutoff),"min = 0 max = 180 help='SpotCutoff. ' " },
		{ "SpotCosCutoff",TW_TYPE_FLOAT,offsetof(Lights, spotCosCutoff),"help='SpotCosCutoff. ' "}
	};
	TwType lightType = TwDefineStruct("Light", lightMembers, 7, sizeof(Lights), NULL, NULL);  // create a new TwType associated to the struct defined by the lightMembers array
	
	TwAddVarRW(light_Material_Bar,"Light #1", lightType, &lights[0], " group='Edit lights' "); // Add a lightType variable and group it into the 'Edit lights' group
	TwAddVarRW(light_Material_Bar,"Light #2", lightType, &lights[1], " group='Edit lights' ");
	TwAddVarRW(light_Material_Bar,"Light #3", lightType, &lights[2], " group='Edit lights' ");
	TwAddVarRW(light_Material_Bar,"Light #4", lightType, &lights[3], " group='Edit lights' ");
	TwAddVarRW(light_Material_Bar,"Light #5", lightType, &lights[4], " group='Edit lights' ");

	// Define a new struct type: FrontMaterial variables are embedded in this structure
    TwStructMember MaterialMembers[] = // array used to describe tweakable variables of the Material structure
    {
		{ "Ambient",   TW_TYPE_COLOR4F, offsetof(MaterialProperties, ambient),   " noalpha help='Ambient color.' " },      // MaterialProperties::ambient is represented by 4 floats, but alpha channel should be ignored
		{ "Diffuse",   TW_TYPE_COLOR4F, offsetof(MaterialProperties, diffuse),   " noalpha help='Diffuse color.' " },      // MaterialProperties::diffuse is represented by 4 floats, but alpha channel should be ignored
		{ "Specular",  TW_TYPE_COLOR4F, offsetof(MaterialProperties, specular),  " noalpha help='Specular color.' " },     // MaterialProperties::specular is represented by 4 floats, but alpha channel should be ignored
		{ "Emission",  TW_TYPE_COLOR4F, offsetof(MaterialProperties, emission),  " noalpha help='Emission color.' " },     // MaterialProperties::emission is represented by 4 floats, but alpha channel should be ignored
		{ "Shininess", TW_TYPE_FLOAT  , offsetof(MaterialProperties, shininess), " min=0 max=100 help='Shininess value.' " } // MaterialProperties::shininnes is represented by 1 float
	};
	TwType MaterialType = TwDefineStruct("Material", MaterialMembers, 5, sizeof(MaterialProperties), NULL, NULL);  // create a new TwType associated to the struct defined by the MaterialMembers array

	TwAddVarRW(light_Material_Bar,"FrontMaterial", MaterialType, frontMaterial," group='Materials' "); // Add a frontMaterialType variable and group it into the 'Material' group
	TwAddVarRW(light_Material_Bar,"BackMaterial", MaterialType, backMaterial," group='Materials' "); // Add a backMaterialType variable and group it into the 'Material' group
}

void initLights()
{
	// Get the max number of lights allowed by the graphic card
    glGetIntegerv(GL_MAX_LIGHTS, &maxLights);

	if(maxLights > 8)
	{
		maxLights = 5;
	}

	lights = new Lights[maxLights];

	for(int i=0;i<maxLights;i++)
	{
		lights[i].active = 0;
	}

	//#Light-1- Directional Light
	lights[0].active = 0;
	lights[0].ambient[0] = 0.3f;lights[0].ambient[1] = 0.3f;lights[0].ambient[2] = 0.3f;lights[0].ambient[3] = 1.0f;
	lights[0].diffuse[0] = 1.0f;lights[0].diffuse[1] = 1.0f;lights[0].diffuse[2] = 1.0f;lights[0].diffuse[3] = 1.0f;
	lights[0].specular[0] = 1.0f;lights[0].specular[1] = 1.0f;lights[0].specular[2] = 1.0f;lights[0].specular[3] = 1.0f;
	lights[0].position[0] = 10.0f;lights[0].position[1] = 1.0f;lights[0].position[2] = 0.0f;lights[0].position[3] = 0.0f;

	//#Light-2- Point Light
	lights[1].active = 0;
	lights[1].ambient[0] = 0.3f;lights[1].ambient[1] = 0.3f;lights[1].ambient[2] = 0.3f;lights[1].ambient[3] = 1.0f;
	lights[1].diffuse[0] = 1.0f;lights[1].diffuse[1] = 1.0f;lights[1].diffuse[2] = 1.0f;lights[1].diffuse[3] = 1.0f;
	lights[1].specular[0] = 1.0f;lights[1].specular[1] = 1.0f;lights[1].specular[2] = 1.0f;lights[1].specular[3] = 1.0f;
	lights[1].position[0] = -4.5f;lights[1].position[1] = 1.0f;lights[1].position[2] = 4.0f;lights[1].position[3] = 1.0f;
	lights[1].constantAttenuation = 1.0f;lights[1].linearAttenuation = 0.0f;lights[1].quadraticAttenuation = 0.0f;
	lights[1].spotCutoff = 180.0f;

	//#lights-2- Point Light
	lights[2].active = 0;
	lights[2].ambient[0] = 0.3f;lights[2].ambient[1] = 0.3f;lights[2].ambient[2] = 0.3f;lights[2].ambient[3] = 1.0f;
	lights[2].diffuse[0] = 1.0f;lights[2].diffuse[1] = 1.0f;lights[2].diffuse[2] = 1.0f;lights[2].diffuse[3] = 1.0f;
	lights[2].specular[0] = 1.0f;lights[2].specular[1] = 1.0f;lights[2].specular[2] = 1.0f;lights[2].specular[3] = 1.0f;
	lights[2].position[0] = -4.5f;lights[2].position[1] = 1.0f;lights[2].position[2] = -4.0f;lights[2].position[3] = 1.0f;
	lights[2].constantAttenuation = 1.0f;lights[2].linearAttenuation = 0.0f;lights[2].quadraticAttenuation = 0.0f;
	lights[2].spotCutoff = 180.0f;

	//#lights-3- Spot Light
	lights[3].active = 0;
	lights[3].ambient[0] = 0.3f;lights[3].ambient[1] = 0.3f;lights[3].ambient[2] = 0.3f;lights[3].ambient[3] = 1.0f;
	lights[3].diffuse[0] = 1.0f;lights[3].diffuse[1] = 1.0f;lights[3].diffuse[2] = 1.0f;lights[3].diffuse[3] = 1.0f;
	lights[3].specular[0] = 1.0f;lights[3].specular[1] = 1.0f;lights[3].specular[2] = 1.0f;lights[3].specular[3] = 1.0f;
	lights[3].position[0] = -4.5f;lights[3].position[1] = 1.0f;lights[3].position[2] = 4.0f;lights[3].position[3] = 1.0f;
	lights[3].constantAttenuation = 1.0f;lights[3].linearAttenuation = 0.0f;lights[3].quadraticAttenuation = 0.0f;
	lights[3].spotCutoff = 45.0f;
	lights[3].spotDirection = glm::vec3(-1.0f,-1.0f,1.0f);
	lights[3].spotExponent = 15.0f;
	lights[3].spotCosCutoff = glm::cos(glm::radians(glm::clamp(lights[3].spotCutoff,0.0f,90.0f)));

	//#lights-4- Spot Light
	lights[4].active = 0;
	lights[4].ambient[0] = 0.3f;lights[4].ambient[1] = 0.3f;lights[4].ambient[2] = 0.3f;lights[4].ambient[3] = 1.0f;
	lights[4].diffuse[0] = 1.0f;lights[4].diffuse[1] = 1.0f;lights[4].diffuse[2] = 1.0f;lights[4].diffuse[3] = 1.0f;
	lights[4].specular[0] = 1.0f;lights[4].specular[1] = 1.0f;lights[4].specular[2] = 1.0f;lights[4].specular[3] = 1.0f;
	lights[4].position[0] = -4.5f;lights[4].position[1] = 1.0f;lights[4].position[2] = -4.0f;lights[4].position[3] = 1.0f;
	lights[4].constantAttenuation = 1.0f;lights[4].linearAttenuation = 0.0f;lights[4].quadraticAttenuation = 0.0f;
	lights[4].spotCutoff = 45.0f;
	lights[4].spotDirection = glm::vec3(-1.0f,-1.0f,1.0f);
	lights[4].spotExponent = 40.0f;
	lights[4].spotCosCutoff = glm::cos(glm::radians(glm::clamp(lights[4].spotCutoff,0.0f,90.0f)));
	
}

void initMaterials()
{
	frontMaterial = new MaterialProperties;

	frontMaterial->ambient[0] = 0.2f;frontMaterial->ambient[1] = 0.2f;frontMaterial->ambient[2] = 0.2f;frontMaterial->ambient[3] = 1.0f;
	frontMaterial->diffuse[0] = 1.0f;frontMaterial->diffuse[1] = 0.8f;frontMaterial->diffuse[2] = 0.8f;frontMaterial->diffuse[3] = 1.0f;
	frontMaterial->specular[0] = 1.0f;frontMaterial->specular[1] = 1.0f;frontMaterial->specular[2] = 1.0f;frontMaterial->specular[3] = 1.0f;
	frontMaterial->emission[0] = 0.0f;frontMaterial->emission[1] = 0.0f;frontMaterial->emission[2] = 0.0f;frontMaterial->emission[3] = 1.0f;
	frontMaterial->shininess = 5.0f;

	backMaterial = new MaterialProperties;

	backMaterial->ambient[0] = 0.2f;backMaterial->ambient[1] = 0.2f;backMaterial->ambient[2] = 0.2f;backMaterial->ambient[3] = 1.0f;
	backMaterial->diffuse[0] = 0.0f;backMaterial->diffuse[1] = 0.0f;backMaterial->diffuse[2] = 1.0f;backMaterial->diffuse[3] = 1.0f;
	backMaterial->specular[0] = 1.0f;backMaterial->specular[1] = 1.0f;backMaterial->specular[2] = 1.0f;backMaterial->specular[3] = 1.0f;
	backMaterial->emission[0] = 0.0f;backMaterial->emission[1] = 0.0f;backMaterial->emission[2] = 0.0f;backMaterial->emission[3] = 1.0f;
	backMaterial->shininess = 5.0f;
}

void initFog()
{
	Fog = new FogProperties;

	Fog->enable = 0;

	Fog->density = 0.15f;
	Fog->start   = 10.0f;
	Fog->end		= 75.0f;

	Fog->color = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);

	Fog->pick = (FogProperties::equation)(FogProperties::FOG_EQUATION_LINEAR);

	Fog->scale = 1.0f / (Fog->end - Fog->start); 
	
}

void LoadUniforms(GLuint program)
{
	//Load Lights
	//Directional Light -1-
	
	glUniform1iv( glGetUniformLocation(program, "lights[0].active"),1,&(lights[0].active));
	glUniform4fv( glGetUniformLocation(program, "lights[0].ambient"),1,lights[0].ambient);
	glUniform4fv( glGetUniformLocation(program, "lights[0].diffuse"),1,lights[0].diffuse);
	glUniform4fv( glGetUniformLocation(program, "lights[0].specular"),1,lights[0].specular);
	glUniform4fv( glGetUniformLocation(program, "lights[0].position"),1,lights[0].position);
	
	//Point Light -2-
	
	glUniform1iv( glGetUniformLocation(program, "lights[1].active"),1,&(lights[1].active));
	glUniform4fv( glGetUniformLocation(program, "lights[1].ambient"),1,lights[1].ambient);
	glUniform4fv( glGetUniformLocation(program, "lights[1].diffuse"),1,lights[1].diffuse);
	glUniform4fv( glGetUniformLocation(program, "lights[1].specular"),1,lights[1].specular);
	glUniform4fv( glGetUniformLocation(program, "lights[1].position"),1,lights[1].position);
	glUniform1fv( glGetUniformLocation(program, "lights[1].constantAttenuation"),1,&(lights[1].constantAttenuation));
	glUniform1fv( glGetUniformLocation(program, "lights[1].linearAttenuation"),1,&(lights[1].linearAttenuation));
	glUniform1fv( glGetUniformLocation(program, "lights[1].quadraticAttenuation"),1,&(lights[1].quadraticAttenuation));
	glUniform1fv( glGetUniformLocation(program, "lights[1].spotCutoff"),1,&(lights[1].spotCutoff));
	
	//Point Light -3-
	
	glUniform1iv( glGetUniformLocation(program, "lights[2].active"),1,&(lights[2].active));
	glUniform4fv( glGetUniformLocation(program, "lights[2].ambient"),1,lights[2].ambient);
	glUniform4fv( glGetUniformLocation(program, "lights[2].diffuse"),1,lights[2].diffuse);
	glUniform4fv( glGetUniformLocation(program, "lights[2].specular"),1,lights[2].specular);
	glUniform4fv( glGetUniformLocation(program, "lights[2].position"),1,lights[2].position);
	glUniform1fv( glGetUniformLocation(program, "lights[2].constantAttenuation"),1,&(lights[2].constantAttenuation));
	glUniform1fv( glGetUniformLocation(program, "lights[2].linearAttenuation"),1,&(lights[2].linearAttenuation));
	glUniform1fv( glGetUniformLocation(program, "lights[2].quadraticAttenuation"),1,&(lights[2].quadraticAttenuation));
	glUniform1fv( glGetUniformLocation(program, "lights[2].spotCutoff"),1,&(lights[2].spotCutoff));
	
	//Spot Light -4-
	
	glUniform1iv( glGetUniformLocation(program, "lights[3].active"),1,&(lights[3].active));
	glUniform4fv( glGetUniformLocation(program, "lights[3].ambient"),1,lights[3].ambient);
	glUniform4fv( glGetUniformLocation(program, "lights[3].diffuse"),1,lights[3].diffuse);
	glUniform4fv( glGetUniformLocation(program, "lights[3].specular"),1,lights[3].specular);
	glUniform4fv( glGetUniformLocation(program, "lights[3].position"),1,lights[3].position);
	glUniform1fv( glGetUniformLocation(program, "lights[3].constantAttenuation"),1,&(lights[3].constantAttenuation));
	glUniform1fv( glGetUniformLocation(program, "lights[3].linearAttenuation"),1,&(lights[3].linearAttenuation));
	glUniform1fv( glGetUniformLocation(program, "lights[3].quadraticAttenuation"),1,&(lights[3].quadraticAttenuation));
	glUniform1fv( glGetUniformLocation(program, "lights[3].spotCutoff"),1,&(lights[3].spotCutoff));
	glUniform3fv( glGetUniformLocation(program, "lights[3].spotDirection"),1,glm::value_ptr(lights[3].spotDirection));
	glUniform1fv( glGetUniformLocation(program, "lights[3].spotExponent"),1,&(lights[3].spotExponent));
	lights[3].spotCosCutoff = glm::cos(glm::radians(glm::clamp(lights[3].spotCutoff,0.0f,90.0f)));
	glUniform1fv( glGetUniformLocation(program, "lights[3].spotCosCutoff"),1,&(lights[3].spotCosCutoff));
	
	//Spot Light -5-
	
	//glUniform1iv( glGetUniformLocation(program, "lights[4].active"),1,&(lights[4].active));
	//glUniform4fv( glGetUniformLocation(program, "lights[4].ambient"),1,lights[4].ambient);
	//glUniform4fv( glGetUniformLocation(program, "lights[4].diffuse"),1,lights[4].diffuse);
	//glUniform4fv( glGetUniformLocation(program, "lights[4].specular"),1,lights[4].specular);
	//glUniform4fv( glGetUniformLocation(program, "lights[4].position"),1,lights[4].position);
	//glUniform1fv( glGetUniformLocation(program, "lights[4].constantAttenuation"),1,&(lights[4].constantAttenuation));
	//glUniform1fv( glGetUniformLocation(program, "lights[4].linearAttenuation"),1,&(lights[4].linearAttenuation));
	//glUniform1fv( glGetUniformLocation(program, "lights[4].quadraticAttenuation"),1,&(lights[4].quadraticAttenuation));
	//glUniform1fv( glGetUniformLocation(program, "lights[4].spotCutoff"),1,&(lights[4].spotCutoff));
	//glUniform3fv( glGetUniformLocation(program, "lights[4].spotDirection"),1,glm::value_ptr(lights[4].spotDirection));
	//glUniform1fv( glGetUniformLocation(program, "lights[4].spotExponent"),1,&(lights[4].spotExponent));
	//glUniform1fv( glGetUniformLocation(program, "lights[4].spotCosCutoff"),1,&(lights[4].spotCosCutoff));
	
	//Load Material Properties
	glUniform4fv( glGetUniformLocation(program, "frontMaterial.ambient"),1,frontMaterial->ambient);
	glUniform4fv( glGetUniformLocation(program, "frontMaterial.diffuse"),1,frontMaterial->diffuse);
	glUniform4fv( glGetUniformLocation(program, "frontMaterial.specular"),1,frontMaterial->specular);
	glUniform4fv( glGetUniformLocation(program, "frontMaterial.emission"),1,frontMaterial->emission);
	glUniform1fv( glGetUniformLocation(program, "frontMaterial.shininess"),1,&(frontMaterial->shininess));

	glUniform4fv( glGetUniformLocation(program, "backMaterial.ambient"),1,backMaterial->ambient);
	glUniform4fv( glGetUniformLocation(program, "backMaterial.diffuse"),1,backMaterial->diffuse);
	glUniform4fv( glGetUniformLocation(program, "backMaterial.specular"),1,backMaterial->specular);
	glUniform4fv( glGetUniformLocation(program, "backMaterial.emission"),1,backMaterial->emission);
	glUniform1fv( glGetUniformLocation(program, "backMaterial.shininess"),1,&(backMaterial->shininess));

	glUniform1iv( glGetUniformLocation(program, "Fog.enable"),1,&(Fog->enable));
	glUniform1fv( glGetUniformLocation(program, "Fog.density"),1,&(Fog->density));
	glUniform1fv( glGetUniformLocation(program, "Fog.end"),1,&(Fog->end));
	glUniform4fv( glGetUniformLocation(program, "Fog.color"),1,glm::value_ptr(Fog->color));
	if(Fog->pick == (FogProperties::equation)(FogProperties::FOG_EQUATION_LINEAR))
	{
		int x = 0;
		glUniform1iv( glGetUniformLocation(program, "Fog.equation"),1,&(x));
	}
	else if(Fog->pick == (FogProperties::equation)(FogProperties::FOG_EQUATION_EXP))
	{
		int x = 1;
		glUniform1iv( glGetUniformLocation(program, "Fog.equation"),1,&(x));
	}
	else
	{
		int x = 2;
		glUniform1iv( glGetUniformLocation(program, "Fog.equation"),1,&(x));
	}
	Fog->scale = 1.0f / (Fog->end - Fog->start); 
	glUniform1fv( glGetUniformLocation(program, "Fog.scale"),1,&(Fog->scale));
	
	
}

void quadSQ( int a, int b, int c, int d )
{
    //specify temporary vectors along each quad's edge in order to compute the face
    // normal using the cross product rule
    glm::vec3 u = (planeVertices[b]-planeVertices[a]).xyz();
    glm::vec3 v = (planeVertices[c]-planeVertices[b]).xyz();
    glm::vec3 norm = glm::cross(u, v);
    glm::vec3 normal= glm::normalize(norm);
    
	normalsq[IndexSQ]=normal;colorsq[IndexSQ] = planeColor[a]; pointsq[IndexSQ] = planeVertices[a]; tex_coords[IndexSQ] = glm::vec4(0.0,1.0,0.0,0.0);IndexSQ++;
    normalsq[IndexSQ]=normal;colorsq[IndexSQ] = planeColor[b]; pointsq[IndexSQ] = planeVertices[b]; tex_coords[IndexSQ] = glm::vec4(1.0,1.0,0.0,0.0);IndexSQ++;
    normalsq[IndexSQ]=normal;colorsq[IndexSQ] = planeColor[c]; pointsq[IndexSQ] = planeVertices[c]; tex_coords[IndexSQ] = glm::vec4(1.0,0.0,0.0,0.0);IndexSQ++;
    normalsq[IndexSQ]=normal;colorsq[IndexSQ] = planeColor[a]; pointsq[IndexSQ] = planeVertices[a]; tex_coords[IndexSQ] = glm::vec4(0.0,1.0,0.0,0.0);IndexSQ++;
    normalsq[IndexSQ]=normal;colorsq[IndexSQ] = planeColor[c]; pointsq[IndexSQ] = planeVertices[c]; tex_coords[IndexSQ] = glm::vec4(1.0,0.0,0.0,0.0);IndexSQ++;
    normalsq[IndexSQ]=normal;colorsq[IndexSQ] = planeColor[d]; pointsq[IndexSQ] = planeVertices[d]; tex_coords[IndexSQ] = glm::vec4(0.0,0.0,0.0,0.0);IndexSQ++;
}

void init3Dmodel()
{

	glGenVertexArrays(1, &vao3Dmodel);
    glBindVertexArray(vao3Dmodel);
	GLuint m_Buffers[4];
	m = new Mesh();
	m->loadMesh("./Models/box1/models/CargoCube01.dae");  

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
    GLuint vPosition = glGetAttribLocation( program3Dmodel, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );

	GLuint vNormal = glGetAttribLocation( program3Dmodel, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET((sizeof(m->Positions[0]) * m->Positions.size())) );

	GLuint vText = glGetAttribLocation( program3Dmodel, "vTexture" );
    glEnableVertexAttribArray( vText );
	glVertexAttribPointer( vText, 2, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET((sizeof(m->Positions[0]) * m->Positions.size()) + (sizeof(m->Normals[0]) * m->Normals.size()) ));
	
	glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void initPyramid()
{
	pointPyramid[0]  = Pyramid[0]; pointPyramid[1]   = Pyramid[1]; pointPyramid[2]   = Pyramid[2];
	pointPyramid[3]  = Pyramid[0]; pointPyramid[4]   = Pyramid[3]; pointPyramid[5]   = Pyramid[2];
	pointPyramid[6]  = Pyramid[0]; pointPyramid[7]   = Pyramid[4]; pointPyramid[8]   = Pyramid[3];
	pointPyramid[9]  = Pyramid[0]; pointPyramid[10]  = Pyramid[4]; pointPyramid[11]  = Pyramid[1];
	pointPyramid[12] = Pyramid[1]; pointPyramid[13]  = Pyramid[4]; pointPyramid[14]  = Pyramid[2];
	pointPyramid[15] = Pyramid[2]; pointPyramid[16]  = Pyramid[4]; pointPyramid[17]  = Pyramid[3];

	for(int i = 0;i < 18; i++)
	{
		colorPyramid[i] = PyramidsColor[0];
	}

	//generate and bind a VAO for the 3D axes
	glGenVertexArrays(1, &vaoPyramid);
    glBindVertexArray(vaoPyramid);

	// Load shaders and use the resulting shader program
    programPyramid = LoadShaders( "./Shaders/vPyramidShader.vert", "./Shaders/fPyramidShader.frag" );
    glUseProgram( programPyramid );

	// Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
	glGenBuffers( 1, &bufferPyramid);
    glBindBuffer( GL_ARRAY_BUFFER, bufferPyramid );
	glBufferData( GL_ARRAY_BUFFER, sizeof(pointPyramid) + sizeof(colorPyramid),NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(pointPyramid), pointPyramid );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(pointPyramid), sizeof(colorPyramid), colorPyramid );
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( programPyramid, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );
    
    GLuint vColor = glGetAttribLocation( programPyramid, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(pointPyramid)) );
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
    
    // only one VAO can be bound at a time, so disable it to avoid altering it accidentally
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void initX()
{
	//generate and bind a VAO for the 3D axes
    glGenVertexArrays(1, &vaoX);
    glBindVertexArray(vaoX);

	// Load shaders and use the resulting shader program
    programX = LoadShaders( "./Shaders/vAxisShader.vert", "./Shaders/fAxisShader.frag" );
    glUseProgram( programX );

	// Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
    glGenBuffers( 1, &bufferX );
    glBindBuffer( GL_ARRAY_BUFFER, bufferX );
    glBufferData( GL_ARRAY_BUFFER, sizeof(Xvertices) + sizeof(Xvertex_color),NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(Xvertices), Xvertices );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(Xvertices), sizeof(Xvertex_color), Xvertex_color );
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( programX, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );
    
    GLuint vColor = glGetAttribLocation( programX, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(Xvertices)) );
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
    
    // only one VAO can be bound at a time, so disable it to avoid altering it accidentally
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void initY()
{
	//generate and bind a VAO for the 3D axes
    glGenVertexArrays(1, &vaoY);
    glBindVertexArray(vaoY);

	// Load shaders and use the resulting shader program
    programY = LoadShaders("./Shaders/vAxisShader.vert", "./Shaders/fAxisShader.frag" );
    glUseProgram( programY );

	// Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
    glGenBuffers( 1, &bufferY );
    glBindBuffer( GL_ARRAY_BUFFER, bufferY );
    glBufferData( GL_ARRAY_BUFFER, sizeof(Yvertices) + sizeof(Yvertex_color),NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(Yvertices), Yvertices );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(Yvertices), sizeof(Yvertex_color), Yvertex_color );
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( programY, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );
    
    GLuint vColor = glGetAttribLocation( programY, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(Yvertices)) );
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
    
    // only one VAO can be bound at a time, so disable it to avoid altering it accidentally
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void initZ()
{
	//generate and bind a VAO for the 3D axes
    glGenVertexArrays(1, &vaoZ);
    glBindVertexArray(vaoZ);



	// Load shaders and use the resulting shader program
    programZ = LoadShaders( "./Shaders/vAxisShader.vert", "./Shaders/fAxisShader.frag" );
    glUseProgram( programZ );

	// Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
    glGenBuffers( 1, &bufferZ );
    glBindBuffer( GL_ARRAY_BUFFER, bufferZ );
    glBufferData( GL_ARRAY_BUFFER, sizeof(Zvertices) + sizeof(Zvertex_color),NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(Zvertices), Zvertices );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(Zvertices), sizeof(Zvertex_color), Zvertex_color );
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( programZ, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );
    
    GLuint vColor = glGetAttribLocation( programZ, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(Zvertices)) );
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
    
    // only one VAO can be bound at a time, so disable it to avoid altering it accidentally
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void initPlane()
{
	float numX =-10.0f,numX1 = -10.5f;
	float numZ = -9.5f,numZ1 = -10.0f;

	planeVertices[0] = point4 ( numX, 0.0, numZ1, 1.0);  planeColor[0] = color4 (0.603922, 0.803922, 0.196078, 1.0);
	planeVertices[1] = point4 ( numX, 0.0, numZ, 1.0);   planeColor[1] = color4 (0.603922, 0.803922, 0.196078, 1.0);
	planeVertices[2] = point4 ( numX1, 0.0, numZ, 1.0);  planeColor[2] = color4 (0.603922, 0.803922, 0.196078, 1.0);
	planeVertices[3] = point4 ( numX1, 0.0, numZ1, 1.0); planeColor[3] = color4 (0.603922, 0.803922, 0.196078, 1.0);

	int k = 4;
	int counter = 0;
	for(k=4;k<NumVerticesSQ;k=k+4)
	{
		numX+=0.5f; numX1+=0.5f; counter++;

		planeVertices[k]   =   point4 (numX,  0.0, numZ1, 1.0); planeColor[k]   = color4 (0.603922, 0.803922, 0.196078, 1.0);
		planeVertices[k+1]	=	point4 (numX,  0.0, numZ, 1.0); planeColor[k+1] = color4 (0.603922, 0.803922, 0.196078, 1.0);
		planeVertices[k+2]	=	point4 (numX1, 0.0, numZ, 1.0); planeColor[k+2] = color4 (0.603922, 0.803922, 0.196078, 1.0);
		planeVertices[k+3]	=   point4 (numX1, 0.0, numZ1, 1.0); planeColor[k+3] = color4 (0.603922, 0.803922, 0.196078, 1.0);

		if( counter == (NumOfSQ - 1) )
		{
			numX = -10.0f;numX1 = -10.5f;k+=4;
			counter = 0;

			numZ+=0.5f;numZ1+=0.5f;

			planeVertices[k]   =   point4 (numX,  0.0, numZ1, 1.0); planeColor[k]   = color4 (0.603922, 0.803922, 0.196078, 1.0);
			planeVertices[k+1]	=	point4 (numX,  0.0, numZ, 1.0); planeColor[k+1] = color4 (0.603922, 0.803922, 0.196078, 1.0);
			planeVertices[k+2]	=	point4 (numX1, 0.0, numZ, 1.0); planeColor[k+2] = color4 (0.603922, 0.803922, 0.196078, 1.0);
			planeVertices[k+3]	=   point4 (numX1, 0.0, numZ1, 1.0); planeColor[k+3] = color4 (0.603922, 0.803922, 0.196078, 1.0);
		}

	}

	//generate and bind a VAO for the 3D axes
    glGenVertexArrays(1, &vaoPlane);
    glBindVertexArray(vaoPlane);

	pTexture = new Texture(GL_TEXTURE_2D,"./Textures/nvidia_logo.jpg");
	//pTexture = new Texture(GL_TEXTURE_2D,"./Textures/NVIDIA.jpg");

	if (!pTexture->loadTexture()) {

		exit(EXIT_FAILURE);
    }

	int lp = 0,a=1,b=0,c=3,d=2;

	for(lp = 0;lp < (NumOfSQ * NumOfSQ);lp++)
	{
		quadSQ(a,b,c,d);

		a+=4;b+=4;c+=4;d+=4;
	}

	// Load shaders and use the resulting shader program
    programPlane = LoadShaders( "./Shaders/vPlaneShader.vert", "./Shaders/fPlaneShader.frag" );
    glUseProgram( programPlane );

	// Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
    glGenBuffers( 1, &bufferPlane );
    glBindBuffer( GL_ARRAY_BUFFER, bufferPlane );
	
	glBufferData( GL_ARRAY_BUFFER, sizeof(pointsq) + sizeof(colorsq) + sizeof(normalsq) + sizeof(tex_coords),NULL, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(pointsq), pointsq );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(pointsq), sizeof(colorsq), colorsq );
	glBufferSubData( GL_ARRAY_BUFFER,sizeof(pointsq) + sizeof(colorsq),sizeof(normalsq),normalsq );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(pointsq) + sizeof(colorsq) + sizeof(normalsq) ,sizeof(tex_coords) , tex_coords );

    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( programPlane, "MCvertex" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );
    
    GLuint vColor = glGetAttribLocation( programPlane, "vColor" ); 
    glEnableVertexAttribArray( vColor );
	glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(pointsq)) );

	GLuint vNormal = glGetAttribLocation( programPlane, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(pointsq) + sizeof(colorsq)) );

	GLuint vText = glGetAttribLocation( programPlane, "MultiTexCoord0" );
    glEnableVertexAttribArray( vText );
	glVertexAttribPointer( vText, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(pointsq) + sizeof(colorsq) + sizeof(normalsq)) );
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
    
    // only one VAO can be bound at a time, so disable it to avoid altering it accidentally
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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
	MVlight = glGetUniformLocation(program3Dmodel, "MVl_mat");
	eyes_uniform = glGetUniformLocation(program3Dmodel, "eyes");

	//Calculation of ModelView Matrix
	glm::mat4 model_mat = tsl;
	glm::mat4 view_mat = glm::lookAt(positionv,positionv + directionv,upv);

	glm::mat4 Model = glm::mat4();
	glm::mat4 ModelView = view_mat * Model;
	glUniformMatrix4fv(MVlight,1, GL_FALSE, glm::value_ptr(ModelView));

	glUniformMatrix4fv(V_uniform3D,1,GL_FALSE,glm::value_ptr(view_mat));
	glm::mat4 MV_mat = view_mat * model_mat;
	glUniformMatrix4fv(MV_uniform3D,1, GL_FALSE, glm::value_ptr(MV_mat)); 

	//Calculation of Normal Matrix
	glm::mat3 Normal_mat = glm::transpose(glm::inverse(glm::mat3(MV_mat)));
	glUniformMatrix3fv(Normal_uniform3D,1, GL_FALSE, glm::value_ptr(Normal_mat));

	//Calculation of ModelViewProjection Matrix
	float aspect = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 projection_mat = glm::perspective(45.0f, aspect,0.1f,100.0f);
	glm::mat4 MVP_mat = projection_mat * MV_mat;
	glUniformMatrix4fv(MVP_uniform3D, 1, GL_FALSE, glm::value_ptr(MVP_mat));

	glm::mat4 InView = glm::inverse(view_mat);
	float eyse[] = {InView[3][0],InView[3][1],InView[3][2],InView[3][3]};
	glm::vec4 eye = glm::vec4(positionv,1.0);
	glUniform4fv(eyes_uniform,1,glm::value_ptr(eye));

	gSampler1 =  glGetUniformLocationARB(program3Dmodel, "gSampler1");
	glUniform1iARB(gSampler1, 0);

	LoadUniforms(program3Dmodel);

	m->render(); 
	if(timesc < 1)
		checkActiveUniforms();
	timesc++;
	glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);
}

void displayPyramid(glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv)
{
	glUseProgram(programPyramid);
	glBindVertexArray(vaoPyramid);

	glDisable(GL_CULL_FACE);
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (wireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	MV_uniformPyramid = glGetUniformLocation(programPyramid, "MV_mat");
	MVP_uniform_Pyramid = glGetUniformLocation(programPyramid, "MVP_mat");

	glUniform1fv( glGetUniformLocation(programPyramid, "Fog.density"),1,&(Fog->density));
	glUniform1fv( glGetUniformLocation(programPyramid, "Fog.start"),1,&(Fog->start));
	glUniform1fv( glGetUniformLocation(programPyramid, "Fog.end"),1,&(Fog->end));
	glUniform4fv( glGetUniformLocation(programPyramid, "Fog.color"),1,glm::value_ptr(Fog->color));
//	glUniform1iv( glGetUniformLocation(programPyramid, "Fog.equation"),1,&(Fog->equation));
	glUniform1fv( glGetUniformLocation(programPyramid, "Fog.scale"),1,&(Fog->scale));
	glUniform1iv( glGetUniformLocation(programPyramid, "Fog.enable"),1,&(Fog->enable));

	// Calculation of ModelView Matrix
	glm::mat4 model_mat_Pyramid = glm::mat4();
	glm::mat4 view_mat_Pyramid = glm::lookAt(positionv,positionv + directionv,upv);
	glm::mat4 MV_mat_Pyramid = view_mat_Pyramid * model_mat_Pyramid;
	glUniformMatrix4fv(MV_uniformPyramid,1, GL_FALSE, glm::value_ptr(MV_mat_Pyramid));

	// Calculation of ModelViewProjection Matrix
	float aspect_Pyramid = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 projection_mat_Pyramid = glm::perspective(45.0f, aspect_Pyramid,0.1f,100.0f);
	glm::mat4 MVP_mat_Pyramid = projection_mat_Pyramid * MV_mat_Pyramid;
	glUniformMatrix4fv(MVP_uniform_Pyramid, 1, GL_FALSE, glm::value_ptr(MVP_mat_Pyramid));

    glDrawArrays( GL_TRIANGLES, 0, 18);
    
    glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);

}

void displayX(glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv)
{
	glUseProgram(programX);
	glBindVertexArray(vaoX);

	glDisable(GL_CULL_FACE);
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (wireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	
	MV_uniformX = glGetUniformLocation(programX, "MV_mat");
	MVP_uniformX = glGetUniformLocation(programX, "MVP_mat");

	// Calculation of ModelView Matrix
	glm::mat4 model_matx = glm::mat4();
	glm::mat4 view_matx = glm::lookAt(positionv,positionv + directionv,upv);
	glm::mat4 MV_matx = view_matx * model_matx;
	glUniformMatrix4fv(MV_uniformX,1, GL_FALSE, glm::value_ptr(MV_matx));

	// Calculation of ModelViewProjection Matrix
	float aspectx = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 projection_matx = glm::perspective(45.0f, aspectx,0.1f,100.0f);
	glm::mat4 MVP_matx = projection_matx * MV_matx;
	glUniformMatrix4fv(MVP_uniformX, 1, GL_FALSE, glm::value_ptr(MVP_matx));

    glDrawArrays( GL_LINES, 0, NumVerticesl );
    
    glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);
}

void displayY(glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv)
{
	glUseProgram(programY);
	glBindVertexArray(vaoY);

	glDisable(GL_CULL_FACE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (wireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	
	MV_uniformY = glGetUniformLocation(programY, "MV_mat");
	MVP_uniformY = glGetUniformLocation(programY, "MVP_mat");

	// Calculation of ModelView Matrix
	glm::mat4 model_maty = glm::mat4();
	glm::mat4 view_maty = glm::lookAt(positionv,positionv + directionv,upv);
	glm::mat4 MV_maty = view_maty * model_maty;
	glUniformMatrix4fv(MV_uniformY,1, GL_FALSE, glm::value_ptr(MV_maty));

	// Calculation of ModelViewProjection Matrix
	float aspecty = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 projection_maty = glm::perspective(45.0f, aspecty,0.1f,100.0f);
	glm::mat4 MVP_maty = projection_maty * MV_maty;
	glUniformMatrix4fv(MVP_uniformY, 1, GL_FALSE, glm::value_ptr(MVP_maty));

    glDrawArrays( GL_LINES, 0, NumVerticesl );
    
    glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);
}

void displayZ(glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv)
{
	glUseProgram(programZ);
	glBindVertexArray(vaoZ);

	glDisable(GL_CULL_FACE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (wireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	MV_uniformZ = glGetUniformLocation(programZ, "MV_mat");
	MVP_uniformZ = glGetUniformLocation(programZ, "MVP_mat");

	// Calculation of ModelView Matrix
	glm::mat4 model_matz = glm::mat4();
	glm::mat4 view_matz = glm::lookAt(positionv,positionv + directionv,upv);
	glm::mat4 MV_matz = view_matz * model_matz;
	glUniformMatrix4fv(MV_uniformZ,1, GL_FALSE, glm::value_ptr(MV_matz));

	// Calculation of ModelViewProjection Matrix
	float aspect = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 projection_matz = glm::perspective(45.0f, aspect,0.1f,100.0f);
	glm::mat4 MVP_matz = projection_matz * MV_matz;
	glUniformMatrix4fv(MVP_uniformZ, 1, GL_FALSE, glm::value_ptr(MVP_matz));
    
    glDrawArrays( GL_LINES, 0, NumVerticesl );
    
    glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);
}

void displayPlane(glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv)
{
	glUseProgram(programPlane);
	glBindVertexArray(vaoPlane);

	glDisable(GL_CULL_FACE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);	
	
    if (wireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	V_uniformPlane = glGetUniformLocation(programPlane, "V_mat" );
	MV_uniformPlane = glGetUniformLocation(programPlane, "MV_mat");
	MVP_uniformPlane = glGetUniformLocation(programPlane, "MVP_mat");
	Normal_uniformPlane = glGetUniformLocation(programPlane, "Normal_mat");
	GLuint eyes_uniformPlane = glGetUniformLocation(programPlane, "eyes");
	TextureMatrix_Uniform = glGetUniformLocation(programPlane, "TextureMatrix");

	glm::mat4 TexMat = glm::mat4();
	//TexMat = glm::scale(glm::mat4(1.0),glm::vec3(-2.0,-2.0,-2.0));

	glUniformMatrix4fv(TextureMatrix_Uniform,1,GL_FALSE,glm::value_ptr(TexMat));

	// Calculation of ModelView Matrix
	glm::mat4 model_mat_plane =  glm::mat4();
	glm::mat4 view_mat_plane = glm::lookAt(positionv,positionv + directionv,upv);
	glUniformMatrix4fv(V_uniformPlane,1,GL_FALSE, glm::value_ptr(view_mat_plane));
	glm::mat4 MV_mat_plane = view_mat_plane * model_mat_plane;
	glUniformMatrix4fv(MV_uniformPlane,1, GL_FALSE, glm::value_ptr(MV_mat_plane));

	// Calculation of Normal Matrix
	glm::mat3 Normal_mat_plane = glm::transpose(glm::inverse(glm::mat3(MV_mat_plane)));
	glUniformMatrix3fv(Normal_uniformPlane,1, GL_FALSE, glm::value_ptr(Normal_mat_plane));

	glm::mat4 InView = glm::inverse(view_mat_plane);
	float eye[] = {InView[3][0],InView[3][1],InView[3][2],InView[3][3]};
	glUniformMatrix4fv(eyes_uniformPlane,1,GL_FALSE,eye);

	// Calculation of ModelViewProjection Matrix
	float aspect_plane = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 projection_mat_plane = glm::perspective(45.0f, aspect_plane,0.1f,100.0f);
	glm::mat4 MVP_mat_plane = projection_mat_plane * MV_mat_plane;
	glUniformMatrix4fv(MVP_uniformPlane, 1, GL_FALSE, glm::value_ptr(MVP_mat_plane));

	LoadUniforms(programPlane);

	gSampler =  glGetUniformLocationARB(programPlane, "gSampler");
	glUniform1iARB(gSampler, 0);
    
	pTexture->bindTexture(GL_TEXTURE0);

    glDrawArrays( GL_TRIANGLES, 0, NumVerticesSQ );

	//GLExitIfError();
	//GLCheckError();
    
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
    glfwSetWindowTitle("Chapter-9");

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

	 // Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	initX();	// initialize X axis

	initY();	// initialize Y axis

	initZ();	// initialize Z axis

	initPlane(); //initialize Plane

	init3Dmodel(); // initialize 3D model

//	initPyramid();

	initLights(); // initialize Lighting of the Scene
	initMaterials(); // initialize Material properties of the Objects and the Scene
	initFog(); // initialize Fog Properties

	TwAddVarRW(myBar, "Fog_Enable", TW_TYPE_INT8,&Fog->enable,"min=0 max=1 Label='Fog Enable' group='Fog Properties'");
	TwAddVarRW(myBar, "Fog_Start", TW_TYPE_FLOAT,&Fog->start, "min=0.0 max=100.0 step=5 Label='Fog Start' group='Fog Properties'");
	TwAddVarRW(myBar, "Fog_End", TW_TYPE_FLOAT,&Fog->end, "min=0.0 max=100.0 step=5 Label='Fog End' group='Fog Properties'");
	TwAddVarRW(myBar, "Fog_Density", TW_TYPE_FLOAT,&Fog->density,"min=0.01 step=0.01 Label='Fog Density' group='Fog Properties'");
	 // Define a new enum type for the tweak bar
    TwEnumVal modeEV[] = // array used to describe the Scene::AnimMode enum values
	{
		{ FogProperties::FOG_EQUATION_LINEAR,    "Linear Equation"     }, 
		{ FogProperties::FOG_EQUATION_EXP,       "Exp Equation"        }, 
		{ FogProperties::FOG_EQUATION_EXP2,      "Exp2 Equation"       }
	};

    TwType modeType = TwDefineEnum("Mode", modeEV, 3);  // create a new TwType associated to the enum defined by the modeEV array

	TwAddVarRW(myBar, "Fog_Equation", modeType, &Fog->pick, " Label='Fog Equation' group='Fog Properties'");
	TwAddVarRW(myBar, "Fog_Color", TW_TYPE_COLOR3F,&Fog->color, " Label='Fog Color' group='Fog Properties'");

	create_Light_Material_bar();

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
		cube1 = glm::translate(cube1,glm::vec3(-4.0,1.0,4.0));

		glm::mat4 cube2 = glm::mat4();
		cube2 = glm::translate(cube2,glm::vec3(-4.0,1.0,-4.0));

		glm::mat4 cube3 = glm::mat4();
		cube3 = glm::translate(cube3,glm::vec3(4.0,1.0,-4.0));

		glm::mat4 cube4 = glm::mat4();
		cube4 = glm::translate(cube4,glm::vec3(4.0,1.0,4.0));

		displayX(pos,direction,up);
		displayY(pos,direction,up);
		displayZ(pos,direction,up);
		displayPlane(pos,direction,up);
	//	displayPyramid(pos,direction,up);

		display3Dmodel(cube1,pos,direction,up);
		display3Dmodel(cube2,pos,direction,up);
		display3Dmodel(cube3,pos,direction,up);
		display3Dmodel(cube4,pos,direction,up);
		
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