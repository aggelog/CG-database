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
int         windowWidth=1024, windowHeight=768;

GLuint      programPlane, program3Dmodel;
GLuint      vao, vaoPlane, vao3Dmodel;
GLuint      bufferPlane, buffer3Dmodel;

GLuint		MV_uniformPlane   , MVP_uniformPlane   , Normal_uniformPlane;
GLuint		MV_uniform3D      , MVP_uniform3D      , Normal_uniform3D;

GLuint		TextureMatrix_Uniform;
int			timesc = 0;
GLuint		gSampler1,gSampler;

Texture	    *pTexture = NULL;
Mesh		*m		  = NULL;

const		int			NumVerticesSQ = ( (NumOfSQ) * (NumOfSQ)) * (2) * (3);
const       int         NumVerticesCube = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

bool        wireFrame = false;
bool		camera    = true;

bool		SYNC	  = true;

typedef     glm::vec4   color4;
typedef     glm::vec4   point4;

int			IndexSQ = 0,IndexSQ1 = 0,IndexCube = 0;

//Modelling arrays
point4		pointsq[NumVerticesSQ];
color4		colorsq[NumVerticesSQ];
glm::vec3	normalsq[NumVerticesSQ];
glm::vec4	tex_coords[NumVerticesSQ];

glm::vec3 pos = glm::vec3( 5.0f, 3.0f , 18.0f );

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

glm::vec3 light_position  = glm::vec3(0.7,0.7,2.0);
glm::vec4 surface_color   = glm::vec4(0.7f,0.6f,0.18f,1.0f);
float     bump_density    = 16.0f;
float     bump_size		  = 0.15f;
float     specular_factor = 0.5f;

TwBar	  *BumpWin        = NULL; 

//Plane
point4		planeVertices[NumVerticesSQ];
color4		planeColor[NumVerticesSQ];

void create_Bump_bar()
{
	// Create a new tweak bar and change its label, position and transparency
	BumpWin = TwNewBar("Bump-Control");
    TwDefine(" Bump-Control label='Bump Mapping Control' position='16 380' alpha=0 help='Use this bar to control the bump mapping effect.' ");

	TwAddVarRW(BumpWin, "LightPos", TW_TYPE_DIR3F, &light_position," label='Light Position'");

	TwAddVarRW(BumpWin, "BumpDen", TW_TYPE_FLOAT, &bump_density," label='Bump Density' min=0.0 group='Bump Parameters'");

	TwAddVarRW(BumpWin, "BumpSize", TW_TYPE_FLOAT, &bump_size," label='Bump Size' min=0.01 max=1.0 step=0.01 group='Bump Parameters'");

	TwAddVarRW(BumpWin, "SurColor", TW_TYPE_COLOR3F, &surface_color, " label='Surface Color' group='Surface Characteristics' ");

	TwAddVarRW(BumpWin, "SpecFactor", TW_TYPE_FLOAT, &specular_factor," label='Specular Factor' min=0 max=5 step=0.5 group='Surface Characteristics'");

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

void TW_CALL ResetView(void * a)
{
	Rotation[0] = 0.0f;Rotation[1] = 0.0f;Rotation[2] = 0.0f;Rotation[3] = 1.0f;

	pos = glm::vec3( 5.0f, 3.0f , 18.0f );

	zNear = 0.1f;
	zFar  = 100.0f;
	FOV	  = 45.0f;
	glfwSetMouseWheel(0);

	horizAngle = 3.14f;
	verticAngle = 0.0f;
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
    

	normalsq[IndexSQ]=normal;colorsq[IndexSQ] = planeColor[a]; pointsq[IndexSQ] = planeVertices[a];IndexSQ++;
    normalsq[IndexSQ]=normal;colorsq[IndexSQ] = planeColor[b]; pointsq[IndexSQ] = planeVertices[b];IndexSQ++;
    normalsq[IndexSQ]=normal;colorsq[IndexSQ] = planeColor[c]; pointsq[IndexSQ] = planeVertices[c];IndexSQ++;
    normalsq[IndexSQ]=normal;colorsq[IndexSQ] = planeColor[a]; pointsq[IndexSQ] = planeVertices[a];IndexSQ++;
    normalsq[IndexSQ]=normal;colorsq[IndexSQ] = planeColor[c]; pointsq[IndexSQ] = planeVertices[c];IndexSQ++;
    normalsq[IndexSQ]=normal;colorsq[IndexSQ] = planeColor[d]; pointsq[IndexSQ] = planeVertices[d];IndexSQ++;

	// Texture Coordinate Generation for the Plane

	if(counter2 != NumOfSQ)
	{
		tex_coords[IndexSQ1] = glm::vec4((bx) + (counter2 * (1.0/NumOfSQ)),(by),0.0,0.0);IndexSQ1++; //                              { 0.0 , 1.0 }
		tex_coords[IndexSQ1] = glm::vec4((cx) + (counter2 * (1.0/NumOfSQ)),(cy),0.0,0.0);IndexSQ1++; // { 1.0 , 1.0 }
        tex_coords[IndexSQ1] = glm::vec4((dx) + (counter2 * (1.0/NumOfSQ)),(dy),0.0,0.0);IndexSQ1++; //               { 1.0 , 0.0 }
        tex_coords[IndexSQ1] = glm::vec4((bx) + (counter2 * (1.0/NumOfSQ)),(by),0.0,0.0);IndexSQ1++; //                              { 0.0 , 1.0 }
        tex_coords[IndexSQ1] = glm::vec4((dx) + (counter2 * (1.0/NumOfSQ)),(dy),0.0,0.0);IndexSQ1++; //               { 1.0 , 0.0 }
        tex_coords[IndexSQ1] = glm::vec4((ax) + (counter2 * (1.0/NumOfSQ)),(ay),0.0,0.0);IndexSQ1++; // { 0.0 , 0.0 }

		counter2++;
	}
	else
	{
		ax = (ax);ay = (ay) + (counter3 * (1.0/NumOfSQ));  // { 0.0 , 0.0 }
		bx = (bx);by = (by) + (counter3 * (1.0/NumOfSQ));  // { 0.0 , 1.0 }
		cx = (cx);cy = (cy) + (counter3 * (1.0/NumOfSQ));  // { 1.0 , 1.0 }
		dx = (dx);dy = (dy) + (counter3 * (1.0/NumOfSQ));  // { 1.0 , 0.0 }

		tex_coords[IndexSQ1] = glm::vec4(bx,by,0.0,0.0);IndexSQ1++;
		tex_coords[IndexSQ1] = glm::vec4(cx,cy,0.0,0.0);IndexSQ1++;
        tex_coords[IndexSQ1] = glm::vec4(dx,dy,0.0,0.0);IndexSQ1++;
        tex_coords[IndexSQ1] = glm::vec4(bx,by,0.0,0.0);IndexSQ1++;
        tex_coords[IndexSQ1] = glm::vec4(dx,dy,0.0,0.0);IndexSQ1++;
        tex_coords[IndexSQ1] = glm::vec4(ax,ay,0.0,0.0);IndexSQ1++;

		counter2 = 1;
	}

}

void init3Dmodel()
{

	
	glGenVertexArrays(1, &vao3Dmodel);
    glBindVertexArray(vao3Dmodel);
	GLuint m_Buffers[5];
	m = new Mesh();

	//m->loadMesh("./Models/box1/models/CargoCube01.dae");
	m->loadMesh("./Models/sphere.dae");

    //Load shaders and use the resulting shader program
	program3Dmodel = LoadShaders( "./Shaders/vshader3D.vert", "./Shaders/fshader3D.frag" );
    glUseProgram( program3Dmodel );

	// Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
	glGenBuffers( 1, &buffer3Dmodel );
	glBindBuffer( GL_ARRAY_BUFFER, buffer3Dmodel );
	glBufferData( GL_ARRAY_BUFFER,(sizeof(m->Positions[0]) * m->Positions.size()) + (sizeof(m->Normals[0]) * m->Normals.size()) + (sizeof(m->TexCoords[0]) * m->TexCoords.size()) + (sizeof(m->Tangents[0]) * m->Tangents.size()),NULL, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, (sizeof(m->Positions[0]) * m->Positions.size()), &m->Positions[0] );
	glBufferSubData( GL_ARRAY_BUFFER, (sizeof(m->Positions[0]) * m->Positions.size()), (sizeof(m->Normals[0]) * m->Normals.size()), &m->Normals[0] );
	glBufferSubData( GL_ARRAY_BUFFER, (sizeof(m->Positions[0]) * m->Positions.size()) + (sizeof(m->Normals[0]) * m->Normals.size()),(sizeof(m->TexCoords[0]) * m->TexCoords.size()), &m->TexCoords[0] );
	glBufferSubData( GL_ARRAY_BUFFER, (sizeof(m->Positions[0]) * m->Positions.size()) + (sizeof(m->Normals[0]) * m->Normals.size()) + (sizeof(m->TexCoords[0]) * m->TexCoords.size()),(sizeof(m->Tangents[0]) * m->Tangents.size()), &m->Tangents[0] );
	
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

	GLuint vTangent = glGetAttribLocation( program3Dmodel, "vTangent" );
	glEnableVertexAttribArray( vTangent );
	glVertexAttribPointer( vTangent, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET((sizeof(m->Positions[0]) * m->Positions.size()) + (sizeof(m->Normals[0]) * m->Normals.size()) + (sizeof(m->TexCoords[0]) * m->TexCoords.size()) ));

	glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void initPlane()
{
	float numX = 0.0f,numX1 = 0.5f;
	float numZ = 0.0f,numZ1 = 0.5f;

	planeVertices[0] = point4 ( numX, 0.0, numZ1, 1.0);  planeColor[0] = color4 (0.603922, 0.803922, 0.196078, 1.0); // 0 a 
	planeVertices[1] = point4 ( numX, 0.0, numZ, 1.0);   planeColor[1] = color4 (0.603922, 0.803922, 0.196078, 1.0); // 1 d
	planeVertices[2] = point4 ( numX1, 0.0, numZ, 1.0);  planeColor[2] = color4 (0.603922, 0.803922, 0.196078, 1.0); // 2 c
	planeVertices[3] = point4 ( numX1, 0.0, numZ1, 1.0); planeColor[3] = color4 (0.603922, 0.803922, 0.196078, 1.0); // 3 b

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
			numX = 0.0f;numX1 = 0.5f;k+=4;
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

	int lp = 0,a,b,c,d;
	a=0,b=3,c=2,d=1;


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

void display3Dmodel(glm::mat4 tsl,glm::mat4 lm,glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv)
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

	glUniform3fv(glGetUniformLocation(program3Dmodel,"LightPosition"),1,glm::value_ptr(light_position));
	glUniform4fv(glGetUniformLocation(program3Dmodel,"SurfaceColor"),1,glm::value_ptr(surface_color));
	glUniform1f(glGetUniformLocation(program3Dmodel,"BumpDensity"),bump_density);
	glUniform1f(glGetUniformLocation(program3Dmodel,"BumpSize"),bump_size);
	glUniform1f(glGetUniformLocation(program3Dmodel,"SpecularFactor"),specular_factor);

	//Calculation of ModelView Matrix
	glm::mat4 model_mat = tsl;
	glm::mat4 view_mat = glm::lookAt(positionv,positionv + directionv,upv);

	glm::mat4 MV_mat = view_mat * model_mat;
	glUniformMatrix4fv(MV_uniform3D,1, GL_FALSE, glm::value_ptr(MV_mat)); 

	//Calculation of Normal Matrix
	glm::mat3 Normal_mat = glm::transpose(glm::inverse(glm::mat3(MV_mat)));
	glUniformMatrix3fv(Normal_uniform3D,1, GL_FALSE, glm::value_ptr(Normal_mat));

	//Calculation of ModelViewProjection Matrix
	float aspect = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 projection_mat = glm::perspective(FOV,aspect,zNear,zFar);
	glm::mat4 MVP_mat = projection_mat * MV_mat;
	glUniformMatrix4fv(MVP_uniform3D, 1, GL_FALSE, glm::value_ptr(MVP_mat));

	m->render(); 

	if(timesc < 1)
		checkActiveUniforms();
	timesc++;

	glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);
}

void displayPlane(glm::mat4 md,glm::vec3 positionv,glm::vec3 directionv,glm::vec3 upv)
{
	glUseProgram(programPlane);
	glBindVertexArray(vaoPlane);

	glDisable(GL_CULL_FACE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);	
	
    if (wireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	MV_uniformPlane = glGetUniformLocation(programPlane, "MV_mat");
	MVP_uniformPlane = glGetUniformLocation(programPlane, "MVP_mat");
	Normal_uniformPlane = glGetUniformLocation(programPlane, "Normal_mat");
	TextureMatrix_Uniform = glGetUniformLocation(programPlane, "TextureMatrix");

	glm::mat4 TexMat = glm::mat4();

	glUniformMatrix4fv(TextureMatrix_Uniform,1,GL_FALSE,glm::value_ptr(TexMat));

	// Calculation of ModelView Matrix
	glm::mat4 model_mat_plane =  md;
	glm::mat4 view_mat_plane = glm::lookAt(positionv,positionv + directionv,upv);
	glm::mat4 MV_mat_plane = view_mat_plane * model_mat_plane;
	glUniformMatrix4fv(MV_uniformPlane,1, GL_FALSE, glm::value_ptr(MV_mat_plane));

	// Calculation of Normal Matrix
	glm::mat3 Normal_mat_plane = glm::transpose(glm::inverse(glm::mat3(MV_mat_plane)));
	glUniformMatrix3fv(Normal_uniformPlane,1, GL_FALSE, glm::value_ptr(Normal_mat_plane));

	// Calculation of ModelViewProjection Matrix
	float aspect_plane = (GLfloat)windowWidth / (GLfloat)windowHeight;
	glm::mat4 projection_mat_plane = glm::perspective(FOV, aspect_plane,zNear,zFar);
	glm::mat4 MVP_mat_plane = projection_mat_plane * MV_mat_plane;
	glUniformMatrix4fv(MVP_uniformPlane, 1, GL_FALSE, glm::value_ptr(MVP_mat_plane));


	gSampler =  glGetUniformLocationARB(programPlane, "gSampler");
	glUniform1iARB(gSampler, 0);
    
	pTexture->bindTexture(GL_TEXTURE0);

    glDrawArrays( GL_TRIANGLES, 0, NumVerticesSQ );
    
    glPopAttrib();
    glBindVertexArray(0);
	glUseProgram(0);
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
    glfwSetWindowTitle("Chapter-11");

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

	std::string vendor ="Title : Chapter-11   Vendor : " + stream1.str() + "   Renderer : " +stream2.str();

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

	initPlane(); //initialize Plane

	init3Dmodel(); // initialize 3D model

	create_Bump_bar();

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

		glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
        glClearColor( bgColor[0], bgColor[1], bgColor[2], bgColor[3]); //black color

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

		glm::mat4 cube;

		glm::mat4 translateMat = glm::mat4();
		translateMat = glm::translate(translateMat,glm::vec3(5.0,3.0,4.0));

		cube  = mat * translateMat;

		displayPlane(mat,pos,direction,up);

		display3Dmodel(cube,mat,pos,direction,up);

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