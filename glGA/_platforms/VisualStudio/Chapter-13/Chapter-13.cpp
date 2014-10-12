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

// global variables
int         windowWidth = 1024 , windowHeight = 768;
int			winMapWd    = 300  , winMapHt     = 300;	  

GLuint      programPlane, program3Dmodel,programQuad , programDepth;
GLuint      vaoPlane, vao3Dmodel,vaoQuad;
GLuint      bufferPlane,buffer3Dmodel,bufferQuad;

GLuint		MVP_uniformQuad;
GLuint		View_uniformPlane , MVP_uniformPlane   , Model_uniformPlane, LightPos_uniformPlane, ShadowMatrixPlane;
GLuint		View_uniform3D    , MVP_uniform3D      , Model_uniform3D   , LightPos_uniform3D	  , ShadowMatrix3D;
GLuint      CamPos_uniform3D,CamPos_uniformPlane;
GLuint		TextureMatrix_Uniform;

int			timesc = 0;
GLuint		gSampler1,gSampler,texID,shadowMapId;

Texture	    *pTexture = NULL;
Mesh		*m		  = NULL;

const		int			NumVerticesl = 2;
const		int			NumVerticesSQ = ( (NumOfSQ) * (NumOfSQ)) * (2) * (3);
const       int         NumVerticesCube = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

bool        wireFrame = false;
bool		camera    = true;

bool		SYNC	  = false;

typedef     glm::vec4   color4;
typedef     glm::vec4   point4;

int			IndexSQ = 0,IndexSQ1 = 0,IndexCube = 0;
bool		ANIMATION = true;

//Modelling arrays
glm::vec3	pointsq[NumVerticesSQ];
glm::vec3	normalsq[NumVerticesSQ];
glm::vec2	tex_coords[NumVerticesSQ];

point4      pointsCube[NumVerticesCube];
color4      colorsCube[NumVerticesCube];

glm::vec3 pos = glm::vec3( 5.0f, 1.5f , 15.5f );

float horizAngle = 3.14f;
float verticAngle = 0.0f;


float speedo = 3.0f;
float mouseSpeedo = 0.005f;

int	  xpos = 0,ypos = 0;

float zNear;
float zFar;
float FOV;
float initialFoV = 45.0f;

float Cleft   = -10.0f;
float Cright  =  10.0f;
float Cbottom = -10.0f;
float Ctop	  =  10.0f;
float Cnear   = -10.0f;
float Cfar    =  20.0f;

// Scene orientation (stored as a quaternion)
float Rotation[] = { 0.0f, 0.0f, 0.0f, 1.0f };

glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0, 
					 0.0, 0.5, 0.0, 0.0,
					 0.0, 0.0, 0.5, 0.0,
					 0.5, 0.5, 0.5, 1.0);

//Plane
glm::vec3		planeVertices[NumVerticesSQ];

point4 QuadVertices[] = {
	point4(-1.0f, -1.0f, 0.0f,1.0f),
	point4( 1.0f, -1.0f, 0.0f,1.0f),
	point4(-1.0f,  1.0f, 0.0f,1.0f),
	point4(-1.0f,  1.0f, 0.0f,1.0f),
	point4( 1.0f, -1.0f, 0.0f,1.0f),
	point4( 1.0f,  1.0f, 0.0f,1.0f)
};
		

struct Lights
{
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];

	GLfloat position[4];
	
	GLfloat intensity;
};

struct MaterialProperties
{
   GLfloat ambient[4];   
   GLfloat diffuse[4];   
   GLfloat specular[4];

   GLfloat shininess; 
};

struct MaterialProperties *frontMaterial;

int				    maxLights = 0;                  // maximum number of dynamic lights allowed by the graphic card
Lights			   *lights = NULL;						// array of lights
MaterialProperties *materials = NULL;
TwBar			   *light_Material_Bar = NULL; 

GLuint FBO;
GLuint ShadowMap;

bool initFrameBuffer()
{
	// Create the FBO
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
	
	// Create the depth buffer
	glGenTextures(1, &ShadowMap);
	glBindTexture(GL_TEXTURE_2D, ShadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT, windowWidth, windowHeight+256, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ShadowMap, 0);
	
    // Disable writes to the color buffer
    glDrawBuffer(GL_NONE);

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

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

void create_Light_Material_bar()
{
	// Create a new tweak bar and change its label, position and transparency
	light_Material_Bar = TwNewBar("Lights-Materials");
    TwDefine(" Lights-Materials label='Lights-Materials TweakBar' position='800 16' alpha=0 help='Use this bar to edit the lights and materials in the scene.' ");

	// Define a new struct type: light variables are embedded in this structure
    TwStructMember lightMembers[] = // array used to describe tweakable variables of the Light structure
    {
        { "Ambient",   TW_TYPE_COLOR4F, offsetof(Lights, ambient),   " noalpha help='Ambient color.' " },      // Lights::ambient is represented by 4 floats, but alpha channel should be ignored
		{ "Diffuse",   TW_TYPE_COLOR4F, offsetof(Lights, diffuse),   " noalpha help='Diffuse color.' " },      // Lights::diffuse is represented by 4 floats, but alpha channel should be ignored
		{ "Specular",  TW_TYPE_COLOR4F, offsetof(Lights, specular),  " noalpha help='Specular color.' " },     // Lights::specular is represented by 4 floats, but alpha channel should be ignored
		{ "Position",  TW_TYPE_DIR3F,   offsetof(Lights, position),  " help='Lights position.' " },
		{ "Intensity",  TW_TYPE_FLOAT,   offsetof(Lights, intensity),  " min=1.0 max=100.0 step=1.0 help='Lights position.' " }
	};
	TwType lightType = TwDefineStruct("Light", lightMembers, 5, sizeof(Lights), NULL, NULL);  // create a new TwType associated to the struct defined by the lightMembers array
	
	TwAddVarRW(light_Material_Bar,"LightSource #1", lightType, &lights[0], " group='Edit lights' "); // Add a lightType variable and group it into the 'Edit lights' group

	// Define a new struct type: FrontMaterial variables are embedded in this structure
    TwStructMember MaterialMembers[] = // array used to describe tweakable variables of the Material structure
    {
		{ "Ambient",   TW_TYPE_COLOR4F, offsetof(MaterialProperties, ambient),   " noalpha help='Ambient color.' " },      // MaterialProperties::ambient is represented by 4 floats, but alpha channel should be ignored
		{ "Diffuse",   TW_TYPE_COLOR4F, offsetof(MaterialProperties, diffuse),   " noalpha help='Diffuse color.' " },      // MaterialProperties::diffuse is represented by 4 floats, but alpha channel should be ignored
		{ "Specular",  TW_TYPE_COLOR4F, offsetof(MaterialProperties, specular),  " noalpha help='Specular color.' " },     // MaterialProperties::specular is represented by 4 floats, but alpha channel should be ignored
		{ "Shininess", TW_TYPE_FLOAT  , offsetof(MaterialProperties, shininess), " min=0.1 max=100 step= 0.1 help='Shininess value.' " } // MaterialProperties::shininnes is represented by 1 float
	};
	TwType MaterialType = TwDefineStruct("Material", MaterialMembers, 4, sizeof(MaterialProperties), NULL, NULL);  // create a new TwType associated to the struct defined by the MaterialMembers array

	TwAddVarRW(light_Material_Bar,"FrontMaterial", MaterialType, frontMaterial," group='Materials' "); // Add a frontMaterialType variable and group it into the 'Material' group

	TwAddVarRW(light_Material_Bar,"Left",TW_TYPE_FLOAT, &Cleft, "label='Left' group='Light-Ortho Proj'");
	TwAddVarRW(light_Material_Bar,"Right",TW_TYPE_FLOAT, &Cright, "label='Right' group='Light-Ortho Proj'");
	TwAddVarRW(light_Material_Bar,"Bottom",TW_TYPE_FLOAT, &Cbottom, "label='Bottom' group='Light-Ortho Proj'");
	TwAddVarRW(light_Material_Bar,"Top",TW_TYPE_FLOAT, &Ctop, "label='Top' group='Light-Ortho Proj'");
	TwAddVarRW(light_Material_Bar,"Near",TW_TYPE_FLOAT, &Cnear, "label='Near' group='Light-Ortho Proj'");
	TwAddVarRW(light_Material_Bar,"Far",TW_TYPE_FLOAT, &Cfar, "label='Far' group='Light-Ortho Proj'");
}

void initLights()
{
	lights = new Lights[1];

	lights[0].ambient[0]  = 1.0f; lights[0].ambient[1]  = 1.0f; lights[0].ambient[2]  = 1.0f; lights[0].ambient[3]  = 1.0f;
	lights[0].diffuse[0]  = 1.0f; lights[0].diffuse[1]  = 1.0f; lights[0].diffuse[2]  = 1.0f; lights[0].diffuse[3]  = 1.0f;
	lights[0].specular[0] = 1.0f; lights[0].specular[1] = 1.0f; lights[0].specular[2] = 1.0f; lights[0].specular[3] = 1.0f;
	lights[0].position[0] = 0.5f; lights[0].position[1] = 2.0f; lights[0].position[2] = 2.0f; lights[0].position[3] = 1.0f;
	lights[0].intensity   = 20.0f;
}

void initMaterials()
{
	frontMaterial = new MaterialProperties;

	frontMaterial->ambient[0] = 0.2f;frontMaterial->ambient[1] = 0.2f;frontMaterial->ambient[2] = 0.2f;frontMaterial->ambient[3] = 1.0f;
	frontMaterial->diffuse[0] = 1.0f;frontMaterial->diffuse[1] = 0.8f;frontMaterial->diffuse[2] = 0.8f;frontMaterial->diffuse[3] = 1.0f;
	frontMaterial->specular[0] = 1.0f;frontMaterial->specular[1] = 1.0f;frontMaterial->specular[2] = 1.0f;frontMaterial->specular[3] = 1.0f;
	frontMaterial->shininess = 5.0f;
}

void LoadUniforms(GLuint program)
{
	//Load Lights
	
	glUniform4fv( glGetUniformLocation(program, "lights[0].ambient"),1,lights[0].ambient);
	glUniform4fv( glGetUniformLocation(program, "lights[0].diffuse"),1,lights[0].diffuse);
	glUniform4fv( glGetUniformLocation(program, "lights[0].specular"),1,lights[0].specular);
	glUniform4fv( glGetUniformLocation(program, "lights[0].position"),1,lights[0].position);
	glUniform1f ( glGetUniformLocation(program, "lights[0].intensity"),lights[0].intensity);
	
	//Load Material Properties

	glUniform4fv( glGetUniformLocation(program, "frontMaterial.ambient"),1,frontMaterial->ambient);
	glUniform4fv( glGetUniformLocation(program, "frontMaterial.diffuse"),1,frontMaterial->diffuse);
	glUniform4fv( glGetUniformLocation(program, "frontMaterial.specular"),1,frontMaterial->specular);
	glUniform1fv( glGetUniformLocation(program, "frontMaterial.shininess"),1,&(frontMaterial->shininess));

}

// Callback function called by GLFW when window size changes
void GLFWCALL WindowSizeCB(int width, int height)
{
    // Set OpenGL viewport and default camera
    glViewport(0, 0, width, height);

	float aspect = (GLfloat)width / (GLfloat)height;

	windowWidth = width;
	windowHeight = height;

	if(width > winMapWd && winMapHt < height)
	{
		winMapWd = (int)(width  / 3) - 150; 
		winMapHt = (int)(height / 3);

		if(windowWidth <= 1024 && windowHeight <= 768 )
		{
			winMapWd = (int)(width  / 3) - 50; 
			winMapHt = (int)(height / 3);
		}
	}
	
	glDeleteFramebuffers(1, &FBO);
	glDeleteTextures(1, &ShadowMap);

	initFrameBuffer();

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

void TW_CALL ResetView(void * a)
{
	Rotation[0] = 0.0f;Rotation[1] = 0.0f;Rotation[2] = 0.0f;Rotation[3] = 1.0f;

	pos = glm::vec3( 5.0f, 1.5f , 15.5f );

	zNear = 0.1f;
	zFar  = 100.0f;
	FOV	  = 45.0f;
	glfwSetMouseWheel(0);

	horizAngle = 3.14f;
	verticAngle = 0.0f;

	Cleft   = -10.0f;
	Cright  =  10.0f;
	Cbottom = -10.0f;
	Ctop	=  10.0f;
	Cnear   = -10.0f;
	Cfar    =  20.0f;
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

float ax = (0.0f/NumOfSQ),ay = (0.0f/NumOfSQ);	// { 0.0 , 0.0 }
float bx = (0.0f/NumOfSQ),by = (1.0f/NumOfSQ);  // { 0.0 , 1.0 }
float cx = (1.0f/NumOfSQ),cy = (1.0f/NumOfSQ);  // { 1.0 , 1.0 }
float dx = (1.0f/NumOfSQ),dy = (0.0f/NumOfSQ);  // { 1.0 , 0.0 }


int counter2 = 0,counter3 = 1;

void quadSQ( int a, int b, int c, int d )
{
			//   0,     3,     2,     1
    //specify temporary vectors along each quad's edge in order to compute the face
    // normal using the cross product rule
    glm::vec3 u = (planeVertices[b]-planeVertices[a]).xyz();
    glm::vec3 v = (planeVertices[c]-planeVertices[b]).xyz();
    glm::vec3 norm = glm::cross(u, v);
    glm::vec3 normal= glm::normalize(norm);
    

	normalsq[IndexSQ]=normal;pointsq[IndexSQ] = planeVertices[a];IndexSQ++;
    normalsq[IndexSQ]=normal;pointsq[IndexSQ] = planeVertices[b];IndexSQ++;
    normalsq[IndexSQ]=normal;pointsq[IndexSQ] = planeVertices[c];IndexSQ++;
    normalsq[IndexSQ]=normal;pointsq[IndexSQ] = planeVertices[a];IndexSQ++;
    normalsq[IndexSQ]=normal;pointsq[IndexSQ] = planeVertices[c];IndexSQ++;
    normalsq[IndexSQ]=normal;pointsq[IndexSQ] = planeVertices[d];IndexSQ++;

	// Texture Coordinate Generation for the Plane

	if(counter2 != NumOfSQ)
	{
		tex_coords[IndexSQ1] = glm::vec2((bx) + (counter2 * (1.0/NumOfSQ)),(by));IndexSQ1++; //                              { 0.0 , 1.0 }
		tex_coords[IndexSQ1] = glm::vec2((cx) + (counter2 * (1.0/NumOfSQ)),(cy));IndexSQ1++; // { 1.0 , 1.0 }
        tex_coords[IndexSQ1] = glm::vec2((dx) + (counter2 * (1.0/NumOfSQ)),(dy));IndexSQ1++; //               { 1.0 , 0.0 }
        tex_coords[IndexSQ1] = glm::vec2((bx) + (counter2 * (1.0/NumOfSQ)),(by));IndexSQ1++; //                              { 0.0 , 1.0 }
        tex_coords[IndexSQ1] = glm::vec2((dx) + (counter2 * (1.0/NumOfSQ)),(dy));IndexSQ1++; //               { 1.0 , 0.0 }
        tex_coords[IndexSQ1] = glm::vec2((ax) + (counter2 * (1.0/NumOfSQ)),(ay));IndexSQ1++; // { 0.0 , 0.0 }

		counter2++;
	}
	else
	{
		ax = (ax);ay = (ay) + (counter3 * (1.0/NumOfSQ));  // { 0.0 , 0.0 }
		bx = (bx);by = (by) + (counter3 * (1.0/NumOfSQ));  // { 0.0 , 1.0 }
		cx = (cx);cy = (cy) + (counter3 * (1.0/NumOfSQ));  // { 1.0 , 1.0 }
		dx = (dx);dy = (dy) + (counter3 * (1.0/NumOfSQ));  // { 1.0 , 0.0 }

		tex_coords[IndexSQ1] = glm::vec2(bx,by);IndexSQ1++;
		tex_coords[IndexSQ1] = glm::vec2(cx,cy);IndexSQ1++;
        tex_coords[IndexSQ1] = glm::vec2(dx,dy);IndexSQ1++;
        tex_coords[IndexSQ1] = glm::vec2(bx,by);IndexSQ1++;
        tex_coords[IndexSQ1] = glm::vec2(dx,dy);IndexSQ1++;
        tex_coords[IndexSQ1] = glm::vec2(ax,ay);IndexSQ1++;

		counter2 = 1;
	}

}

void init3Dmodel()
{

	
	glGenVertexArrays(1, &vao3Dmodel);
    glBindVertexArray(vao3Dmodel);
	GLuint m_Buffers[4];
	m = new Mesh();

	m->loadMesh("./Models/box1/models/CargoCube01.dae");

    //Load shaders and use the resulting shader program
	program3Dmodel = LoadShaders( "./Shaders/vSceneShader.vert", "./Shaders/fSceneShader.frag" );
    glUseProgram( program3Dmodel );

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

	///////////////////////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

	GLuint MCvertex = glGetAttribLocation( programDepth , "MCvertex" );
	glEnableVertexAttribArray( MCvertex );
    glVertexAttribPointer( MCvertex, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );

	/////////////////////////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

	GLuint vNormal = glGetAttribLocation( program3Dmodel, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET((sizeof(m->Positions[0]) * m->Positions.size())) );

	GLuint vText = glGetAttribLocation( program3Dmodel, "vTexture" );
    glEnableVertexAttribArray( vText );
	glVertexAttribPointer( vText, 2, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET((sizeof(m->Positions[0]) * m->Positions.size()) + (sizeof(m->Normals[0]) * m->Normals.size()) ));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void initQuad()
{
	//generate and bind a VAO for the 3D axes
    glGenVertexArrays(1, &vaoQuad);
    glBindVertexArray(vaoQuad);

	// Load shaders and use the resulting shader program
    programQuad = LoadShaders( "./Shaders/v1pass.vert", "./Shaders/f1pass.frag" );
    glUseProgram( programQuad );

	// Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
    glGenBuffers( 1, &bufferQuad );
    glBindBuffer( GL_ARRAY_BUFFER, bufferQuad );
    glBufferData( GL_ARRAY_BUFFER, sizeof(QuadVertices),NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(QuadVertices), QuadVertices );
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( programQuad, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );
    
    // only one VAO can be bound at a time, so disable it to avoid altering it accidentally
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void initPlane()
{
	float numX = 0.0f,numX1 = 0.5f;
	float numZ = 0.0f,numZ1 = 0.5f;

	planeVertices[0] = glm::vec3 ( numX,  0.0, numZ1);  // 0 a 
	planeVertices[1] = glm::vec3 ( numX,  0.0, numZ );   // 1 d
	planeVertices[2] = glm::vec3 ( numX1, 0.0, numZ );  // 2 c
	planeVertices[3] = glm::vec3 ( numX1, 0.0, numZ1); // 3 b

	int k = 4;
	int counter = 0;
	for(k=4;k<NumVerticesSQ;k=k+4)
	{
		numX+=0.5f; numX1+=0.5f; counter++;

		planeVertices[k]   =   glm::vec3 (numX,  0.0, numZ1 );
		planeVertices[k+1]	=	glm::vec3 (numX,  0.0, numZ );
		planeVertices[k+2]	=	glm::vec3 (numX1, 0.0, numZ );
		planeVertices[k+3]	=   glm::vec3 (numX1, 0.0, numZ1);

		if( counter == (NumOfSQ - 1) )
		{
			numX = 0.0f;numX1 = 0.5f;k+=4;
			counter = 0;

			numZ+=0.5f;numZ1+=0.5f;

			planeVertices[k]   =   glm::vec3 (numX,  0.0, numZ1 );
			planeVertices[k+1]	=	glm::vec3 (numX,  0.0, numZ );
			planeVertices[k+2]	=	glm::vec3 (numX1, 0.0, numZ );
			planeVertices[k+3]	=   glm::vec3 (numX1, 0.0, numZ1);
		}

	}

	//generate and bind a VAO for the 3D axes
    glGenVertexArrays(1, &vaoPlane);
    glBindVertexArray(vaoPlane);

	pTexture = new Texture(GL_TEXTURE_2D,"./Textures/nvidia_logo.jpg");

	if (!pTexture->loadTexture()) {

		exit(EXIT_FAILURE);
    }

	int lp = 0,a,b,c,d;
	a=0,b=3,c=2,d=1;


	for(lp = 0;lp < (NumOfSQ * NumOfSQ);lp++)
	{
		quadSQ(a,b,c,d);

		a+=4;b+=4;c+=4;d+=4;
	}

	// Load shaders and use the resulting shader program
    programPlane = LoadShaders( "./Shaders/vSceneShader.vert", "./Shaders/fSceneShader.frag" );
    glUseProgram( programPlane );

	// Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
    glGenBuffers( 1, &bufferPlane );
    glBindBuffer( GL_ARRAY_BUFFER, bufferPlane );
	
	glBufferData( GL_ARRAY_BUFFER, sizeof(pointsq) + sizeof(normalsq) + sizeof(tex_coords),NULL, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(pointsq), pointsq );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(pointsq), sizeof(normalsq), normalsq );
	glBufferSubData( GL_ARRAY_BUFFER,sizeof(pointsq) + sizeof(normalsq),sizeof(tex_coords),tex_coords );

    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( programPlane, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );

	GLuint MCvertex = glGetAttribLocation( programDepth , "MCvertex" );
	glEnableVertexAttribArray( MCvertex );
    glVertexAttribPointer( MCvertex, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );

	GLuint vNormal = glGetAttribLocation( programPlane, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(pointsq)) );

	GLuint vText = glGetAttribLocation( programPlane, "vTexture" );
    glEnableVertexAttribArray( vText );
	glVertexAttribPointer( vText, 2, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(pointsq) + sizeof(normalsq)) );
    
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

void display3Dmodel(glm::mat4 tsl,glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv,GLuint program)
{
	if(program == program3Dmodel)
	{
		glUseProgram(program3Dmodel);
		glBindVertexArray(vao3Dmodel);

		if (wireFrame)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		Model_uniform3D = glGetUniformLocation(program3Dmodel, "M_mat");
		View_uniform3D = glGetUniformLocation(program3Dmodel, "V_mat");
		MVP_uniform3D = glGetUniformLocation(program3Dmodel, "MVP_mat");
		ShadowMatrix3D = glGetUniformLocation(program3Dmodel, "ShadowMatrix");
		CamPos_uniform3D = glGetUniformLocation(program3Dmodel, "CameraPosition");
		LightPos_uniform3D = glGetUniformLocation(program3Dmodel, "LightPosition");
		

		glm::mat4 model_mat = tsl;
		glUniformMatrix4fv(Model_uniform3D,1, GL_FALSE, glm::value_ptr(model_mat));
		glm::mat4 view_mat = glm::lookAt(positionv,positionv+directionv,upv);
		glUniformMatrix4fv(View_uniform3D,1, GL_FALSE, glm::value_ptr(view_mat));
		
		glm::mat4 ModelView = view_mat * model_mat;

		//Calculation of ModelViewProjection Matrix
		float aspect = (GLfloat)windowWidth / (GLfloat)windowHeight;
		glm::mat4 projection_mat = glm::perspective(FOV,aspect,zNear,zFar);
		glm::mat4 MVP_mat = projection_mat * ModelView;
		glUniformMatrix4fv(MVP_uniform3D, 1, GL_FALSE, glm::value_ptr(MVP_mat));

		glUniform3fv(CamPos_uniform3D,1,glm::value_ptr(positionv));

		// Calculation of Shadow Matrix
		glm::vec3 sLpos = glm::vec3(lights[0].position[0],lights[0].position[1],lights[0].position[2]);

		glUniform3fv(LightPos_uniform3D,1,glm::value_ptr(sLpos));

		glm::mat4 model_smat = tsl;
		glm::mat4 view_smat  = glm::lookAt(sLpos,glm::vec3(0,0,0),glm::vec3(0,1,0));
		glm::mat4 projection_mats = glm::ortho(Cleft, Cright, Cbottom, Ctop, Cnear, Cfar);

		glm::mat4 SMVP_MAP = biasMatrix * projection_mats * view_smat * model_smat;

		glUniformMatrix4fv(ShadowMatrix3D,1,GL_FALSE,glm::value_ptr(SMVP_MAP));

		gSampler1 =  glGetUniformLocationARB(program3Dmodel, "textureMap");
		glUniform1iARB(gSampler1, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ShadowMap);

		shadowMapId = glGetUniformLocationARB(program3Dmodel, "shadowMap");
		glUniform1iARB(shadowMapId, 1);

		LoadUniforms(program3Dmodel);

		m->render(); 

		if(timesc < 1)
			checkActiveUniforms();
		timesc++;

		glBindVertexArray(0);
		glUseProgram(0);
	}
	else if(program == programDepth)
	{
		glUseProgram(programDepth);
		glBindVertexArray(vao3Dmodel);

		if (wireFrame)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		MVP_uniform3D = glGetUniformLocation(programDepth, "depthMVP");

		//Calculation of ModelView Matrix
		glm::mat4 model_mat = tsl;
		glm::vec3 sLpos = glm::vec3(lights[0].position[0],lights[0].position[1],lights[0].position[2]);
		glm::mat4 view_mat = glm::lookAt(sLpos,glm::vec3(0,0,0),glm::vec3(0,1,0));
		glm::mat4 MV_mat = view_mat * model_mat;

		//Calculation of ModelViewProjection Matrix
		float aspect = (GLfloat)windowWidth / (GLfloat)windowHeight;
		glm::mat4 projection_mat = glm::ortho(Cleft, Cright, Cbottom, Ctop, Cnear, Cfar);
		glm::mat4 MVP_mat = projection_mat * MV_mat;

		glUniformMatrix4fv(MVP_uniform3D, 1, GL_FALSE, glm::value_ptr(MVP_mat));

		m->render(); 

		glBindVertexArray(0);
		glUseProgram(0);
	}
	else
	{
		std::cout<<"\nProgram 3D Reports :  Problem to Draw Function @ : GLuint programm doesn't mach.\n";
		exit(EXIT_FAILURE);
	}
}

void displayQuad(glm::mat4 md,glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv,GLuint program)
{
	glUseProgram(programQuad);
	glBindVertexArray(vaoQuad);

	if (wireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ShadowMap);

	texID =  glGetUniformLocationARB(programQuad, "texID");
	glUniform1i(texID, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glDrawArrays( GL_TRIANGLES, 0, 6);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

    glBindVertexArray(0);
	glUseProgram(0);
}

void displayPlane(glm::mat4 md,glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv,GLuint program)
{
	if(program == programPlane)
	{
		glUseProgram(programPlane);
		glBindVertexArray(vaoPlane);
	
		if (wireFrame)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		Model_uniformPlane = glGetUniformLocation(programPlane, "M_mat");
		View_uniformPlane = glGetUniformLocation(programPlane, "V_mat");
		MVP_uniformPlane = glGetUniformLocation(programPlane, "MVP_mat");
		ShadowMatrixPlane = glGetUniformLocation(programPlane, "ShadowMatrix");
		CamPos_uniformPlane = glGetUniformLocation(programPlane, "CameraPosition");
		LightPos_uniformPlane = glGetUniformLocation(programPlane, "LightPosition");

		// Calculation of ModelView Matrix
		glm::mat4 model_mat_plane =  md;
		glUniformMatrix4fv(Model_uniformPlane,1, GL_FALSE, glm::value_ptr(model_mat_plane));

		glm::mat4 view_mat_plane = glm::lookAt(positionv,positionv+directionv,upv);
		glUniformMatrix4fv(View_uniformPlane,1, GL_FALSE, glm::value_ptr(view_mat_plane));

		glm::mat4 MV_mat_plane = view_mat_plane * model_mat_plane;

		// Calculation of ModelViewProjection Matrix
		float aspect_plane = (GLfloat)windowWidth / (GLfloat)windowHeight;
		glm::mat4 projection_mat_plane = glm::perspective(FOV, aspect_plane,zNear,zFar);
		glm::mat4 MVP_mat_plane = projection_mat_plane * MV_mat_plane;
		glUniformMatrix4fv(MVP_uniformPlane, 1, GL_FALSE, glm::value_ptr(MVP_mat_plane));

		// Calculation of Shadow Matrix
		glm::vec3 sLpos = glm::vec3(lights[0].position[0],lights[0].position[1],lights[0].position[2]);
		glUniform3fv(LightPos_uniformPlane,1,glm::value_ptr(sLpos));

		glUniform3fv(CamPos_uniformPlane,1,glm::value_ptr(positionv));

		glm::mat4 model_smat = md;
		glm::mat4 view_smat  = glm::lookAt(sLpos,glm::vec3(0,0,0),glm::vec3(0,1,0));
		glm::mat4 projection_mats = glm::ortho(Cleft, Cright, Cbottom, Ctop, Cnear, Cfar);

		glm::mat4 SMVP_MAP = biasMatrix * projection_mats * view_smat * model_smat; 

		glUniformMatrix4fv(ShadowMatrixPlane,1,GL_FALSE,glm::value_ptr(SMVP_MAP));

		LoadUniforms(programPlane);

		pTexture->bindTexture(GL_TEXTURE0);

		gSampler =  glGetUniformLocationARB(programPlane, "textureMap");
		glUniform1iARB(gSampler, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ShadowMap);

		shadowMapId = glGetUniformLocationARB(programPlane, "shadowMap");
		glUniform1iARB(shadowMapId, 1);

		glDrawArrays( GL_TRIANGLES, 0, NumVerticesSQ );
   
		glBindVertexArray(0);
		glUseProgram(0);
	}
	else if(program == programDepth)
	{
		glUseProgram(programDepth);
		glBindVertexArray(vaoPlane);
	
		if (wireFrame)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		MVP_uniformPlane = glGetUniformLocation(programDepth, "depthMVP");

		// Calculation of ModelView Matrix
		glm::mat4 model_mat_plane =  md;
		glm::vec3 sLpos = glm::vec3(lights[0].position[0],lights[0].position[1],lights[0].position[2]);
		glm::mat4 view_mat_plane = glm::lookAt(sLpos,glm::vec3(0,0,0),glm::vec3(0,1,0));
		glm::mat4 MV_mat_plane = view_mat_plane * model_mat_plane;

		// Calculation of ModelViewProjection Matrix
		float aspect_plane = (GLfloat)windowWidth / (GLfloat)windowHeight;
		glm::mat4 projection_mat_plane = glm::ortho(Cleft, Cright, Cbottom, Ctop, Cnear, Cfar);
		glm::mat4 MVP_mat_plane = projection_mat_plane * MV_mat_plane;

		glUniformMatrix4fv(MVP_uniformPlane, 1, GL_FALSE, glm::value_ptr(MVP_mat_plane));

		glDrawArrays( GL_TRIANGLES, 0, NumVerticesSQ );
    
		glBindVertexArray(0);
		glUseProgram(0);
	}
	else
	{
		std::cout<<"\nProgram Plane Reports :  Problem to Draw Function @ : GLuint programm doesn't mach.\n";
		exit(EXIT_FAILURE);
	}
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
    glfwSetWindowTitle("Chapter-13");

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

	std::string vendor ="Title : Chapter-13   Vendor : " + stream1.str() + "   Renderer : " +stream2.str();

	const char *tit = vendor.c_str();
	glfwSetWindowTitle(tit);
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
	
	 // Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	if(!initFrameBuffer())
	{
		std::cout<<"\nProblem initializing the FRAMEBUFFER!\n";
		exit(EXIT_FAILURE);
	}

	programDepth = LoadShaders( "./Shaders/vDepthShader.vert", "./Shaders/fDepthShader.frag" );
	glUseProgram(programDepth);

	initQuad(); // initialize a Square

	init3Dmodel(); // initialize 3D model

	initPlane(); //initialize Plane

	

	initLights(); // initialize Lighting of the Scene
	initMaterials(); // initialize Material properties of the Objects and the Scene

	create_Light_Material_bar();

	bool flagPlus  = false;
	bool flagMinus = true;
	GLfloat rat = 0.001f;
	float posL1 = 1.1f;
	float posL2 = 0.05f;

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

		FOV = initialFoV - 5 * glfwGetMouseWheel();

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

		mat = ConvertQuaternionToMatrix(Rotation, mat);

		glm::mat4 cube,cubeL1,cubeL2;

		glm::mat4 translateMat = glm::mat4();
		translateMat = glm::translate(translateMat,glm::vec3(5.0,3.0,4.0));

		cube  = translateMat * mat;
		///////////////////////////////////////////////////  FIRST PASS
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
		glViewport(0, 0,windowWidth,windowHeight);

		glClear(GL_DEPTH_BUFFER_BIT);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		display3Dmodel(cube,pos,direction,up,programDepth);
		displayPlane(glm::mat4(1.0),pos,direction,up,programDepth);

		glDisable(GL_CULL_FACE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//////////////////////////////////////////////////   SECOND PASS
		glViewport(0, 0,windowWidth,windowHeight);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		displayPlane(glm::mat4(1.0),pos,direction,up,programPlane);
		display3Dmodel(cube,pos,direction,up,program3Dmodel);

		//glPopAttrib();

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

		glViewport(0, 0,winMapWd,winMapHt);

		
		displayQuad(mat,pos,direction,up,programQuad);

		glDisable(GL_CULL_FACE);

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
