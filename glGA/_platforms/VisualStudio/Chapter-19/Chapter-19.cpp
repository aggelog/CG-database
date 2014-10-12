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

GLuint      programQuad;
GLuint      vaoQuad;
GLuint      bufferQuad;

GLuint		TextureMatrix_Uniform;
int			timesc = 0;
GLuint		gSampler1,gSampler;

Texture	    *BaseTex  = NULL;
Texture		*BlendTex = NULL;
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

point4 QuadVertices[] = {
	point4(-1.0f, -1.0f, 0.0f,1.0f),
	point4( 1.0f, -1.0f, 0.0f,1.0f),
	point4(-1.0f,  1.0f, 0.0f,1.0f),
	point4(-1.0f,  1.0f, 0.0f,1.0f),
	point4( 1.0f, -1.0f, 0.0f,1.0f),
	point4( 1.0f,  1.0f, 0.0f,1.0f)
};

struct BlendModes{

	enum BM {Normal     , Average			 , Dissolve    , Behind   , 
			 Clear      , Darken			 , Lighten     , Multiply ,
			 Screen     , Color_Burn		 , Color_Dodge , Overlay  ,
			 Soft_Light , Hard_Light         , Add         , Subtract ,
			 Difference , Inverse_Difference , Exclusion			 };
	BM blend_mode;
};

BlendModes *x;

float opacity = 1.0f;

bool Op = true;

float noiseScale = 1.0f;

glm::vec3 pos = glm::vec3( 0.0f, 0.0f , 30.0f );

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

	pos = glm::vec3( -4.0f, 0.0f , 30.0f );

	zNear = 0.1f;
	zFar  = 100.0f;
	FOV	  = 45.0f;
	glfwSetMouseWheel(0);

	horizAngle = 3.14f;
	verticAngle = 0.0f;
}

glm::vec4 Background = glm::vec4(0.0,0.0,0.0,1.0);


void checkActiveUniforms()
{
	GLint nUniforms, maxLen;
	glGetProgramiv( programQuad, GL_ACTIVE_UNIFORM_MAX_LENGTH,&maxLen);
	glGetProgramiv( programQuad, GL_ACTIVE_UNIFORMS,&nUniforms);
	GLchar * name = (GLchar *) malloc( maxLen );
	GLint size, location;
	GLsizei written;
	GLenum type;

	printf(" Location | Name\n");
	printf("------------------------------------------------\n");

	for( int i = 0; i < nUniforms; ++i ) {

		glGetActiveUniform( programQuad, i, maxLen, &written,&size, &type, name );
		location = glGetUniformLocation(programQuad, name);
		printf(" %-8d | %s\n", location, name);
	}
	free(name);
}

void initQuad()
{
	//generate and bind a VAO for the 3D axes
    glGenVertexArrays(1, &vaoQuad);
    glBindVertexArray(vaoQuad);

	BaseTex  = new Texture(GL_TEXTURE_2D,"./Textures/Special-Summer-Sunset.jpg");
	BlendTex = new Texture(GL_TEXTURE_2D,"./Textures/sail-boat-images.jpg");

	if (!BaseTex->loadTexture() || !BlendTex->loadTexture()) {

		exit(EXIT_FAILURE);
    }

	// Load shaders and use the resulting shader program
    programQuad = LoadShaders( "./Shaders/vBlend.vert", "./Shaders/fBlend.frag" );
    glUseProgram( programQuad );

	// Create and initialize a buffer object on the server side (GPU)
    //GLuint buffer;
    glGenBuffers( 1, &bufferQuad );
    glBindBuffer( GL_ARRAY_BUFFER, bufferQuad );
    glBufferData( GL_ARRAY_BUFFER, sizeof(QuadVertices),NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(QuadVertices), QuadVertices );
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( programQuad, "MCvertex" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
    
    // only one VAO can be bound at a time, so disable it to avoid altering it accidentally
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void updateBlendMode(BlendModes *y)
{
	for(int i = 0; i < 19 ; i++)
	{
		if(y->blend_mode == i)
		{
			glUniform1i(glGetUniformLocation(programQuad,"Mode"),i);
		}
	}
}

void displayQuad()
{
	glUseProgram(programQuad);
	glBindVertexArray(vaoQuad);

	glDisable(GL_CULL_FACE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (wireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glUniform1f(glGetUniformLocation(programQuad,"Opacity"),opacity);

	glUniform1f(glGetUniformLocation(programQuad,"noiseScale"),noiseScale);

	if(Op == true)
	{
		glUniform1i(glGetUniformLocation(programQuad,"Opacity_Mode"),1);
	}
	else
	{
		glUniform1i(glGetUniformLocation(programQuad,"Opacity_Mode"),0);
	}

	updateBlendMode(x);

	gSampler1 =  glGetUniformLocationARB(programQuad, "BaseImage");
	glUniform1iARB(gSampler1, 0);

	BaseTex->bindTexture(GL_TEXTURE0);

	gSampler =  glGetUniformLocationARB(programQuad, "BlendImage");
	glUniform1iARB(gSampler, 1);

	BlendTex->bindTexture(GL_TEXTURE1);

    glDrawArrays( GL_TRIANGLES, 0, 6);
    
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

	
	x = new BlendModes();
	x->blend_mode = BlendModes::Normal;

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
    glfwSetWindowTitle("Chapter-19");

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

	std::string vendor ="Title : Chapter-19   Vendor : " + stream1.str() + "   Renderer : " +stream2.str();

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

	TwAddVarRW(myBar, "MS per 1 Frame" , TW_TYPE_DOUBLE, &FPS, "label='MS per 1 Frame' readonly=true group='Frame Rate'");

	TwAddVarRW(myBar, "Frames Per Second" , TW_TYPE_INT32, &rate, "label='FPS' readonly=true group='Frame Rate'");

	TwAddVarRW(myBar, "vSYNC" , TW_TYPE_BOOL8, &SYNC, "label='vSync' readonly=true group='Frame Rate'");

	// Define a new enum type for the tweak bar
    TwEnumVal modeEV[] =
    {
		{ BlendModes::Normal			 , "1.Normal"			   }, 
		{ BlendModes::Average			 , "2.Average"			   },
		{ BlendModes::Dissolve			 , "3.Dissolve"			   },
		{ BlendModes::Behind			 , "4.Behind"			   },
		{ BlendModes::Clear				 , "5.Clear"			   },
		{ BlendModes::Darken			 , "6.Darken"			   },
		{ BlendModes::Lighten			 , "7.Lighten"			   },
		{ BlendModes::Multiply			 , "8.Multiply"			   },
		{ BlendModes::Screen			 , "9.Screen"			   },
		{ BlendModes::Color_Burn		 , "10.Color_Burn"		   },
		{ BlendModes::Color_Dodge		 , "11.Color_Dodge"		   },
		{ BlendModes::Overlay			 , "12.Overlay"			   },
		{ BlendModes::Soft_Light		 , "13.Soft_Light"		   },
		{ BlendModes::Hard_Light		 , "14.Hard_Light"		   },
		{ BlendModes::Add				 , "15.Add"				   },
		{ BlendModes::Subtract			 , "16.Subtract"		   },
		{ BlendModes::Difference		 , "17.Difference"		   },
		{ BlendModes::Inverse_Difference , "18.Inverse_Difference" },
		{ BlendModes::Exclusion			 , "19.Exclusion"		   }
    };

    TwType modeType = TwDefineEnum("Mode", modeEV, 19);  // create a new TwType associated to the enum defined by the modeEV array

	TwAddVarRW(myBar, "Blend-Modes" ,modeType, &x->blend_mode, "label='Blend Mode' group='Imaging Control'");

	TwAddVarRW(myBar, "Opacity" , TW_TYPE_FLOAT, &opacity, "label='Opacity' min=0.05 max=3.0 step=0.05 group='Imaging Control'");

	TwAddVarRW(myBar, "Opacity mode", TW_TYPE_BOOL32, &Op," label='Opacity mode' key=o group='Imaging Control' ");

	TwAddVarRW(myBar, "noiseScale" , TW_TYPE_FLOAT, &noiseScale, "label='noiseScale for Dissolve' min=0.5 max=100.0 step=0.05 group='Imaging Control'");
	
	 // Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	initQuad();

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


		currentTime = glfwGetTime();
		float dTime = float(currentTime - lastTime);
		lastTime = (float)currentTime;

		displayQuad();

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