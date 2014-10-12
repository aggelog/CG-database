#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <../_thirdPartyLibs/glew-1.9.0/include/GL/glew.h>

#if defined(__APPLE__)
#include <OpenGL/gl3.h>  
#endif

#include <glGAHelper.h>


//*********************************** SHADER Helper functions ***********************/

// Create a NULL-terminated string by reading the provided file (in C)
static char* readShaderSource(const char* shaderFile)
{
    FILE* fp = fopen(shaderFile, "r");
    
    if ( fp == NULL ) { return NULL; }
    
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    
    fseek(fp, 0L, SEEK_SET);
    char* buf = new char[size + 1];
    fread(buf, 1, size, fp);
    
    buf[size] = '\0';
    fclose(fp);
    
    return buf;
}

// Create a GLSL program object from vertex and fragment shader files (in C)
GLuint InitShader(const char* vShaderFile, const char* fShaderFile)
{
    struct Shader {
        const char*  filename;
        GLenum       type;
        GLchar*      source;
    }  shaders[2] = {
        { vShaderFile, GL_VERTEX_SHADER, NULL },
        { fShaderFile, GL_FRAGMENT_SHADER, NULL }
    };
    
    GLuint program = glCreateProgram();
    
    for ( int i = 0; i < 2; ++i ) {
        Shader& s = shaders[i];
        s.source = readShaderSource( s.filename );
        if ( shaders[i].source == NULL ) {
            std::cerr << "Failed to read " << s.filename << std::endl;
            exit( EXIT_FAILURE );
        }
        
        GLuint shader = glCreateShader( s.type );
        glShaderSource( shader, 1, (const GLchar**) &s.source, NULL );
        glCompileShader( shader );
        
        GLint  compiled;
        glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
        if ( !compiled ) {
            std::cerr << s.filename << " failed to compile:" << std::endl;
            GLint  logSize;
            glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logSize );
            char* logMsg = new char[logSize];
            glGetShaderInfoLog( shader, logSize, NULL, logMsg );
            std::cerr << logMsg << std::endl;
            delete [] logMsg;
            
            exit( EXIT_FAILURE );
        }
        
        delete [] s.source;
        
        glAttachShader( program, shader );
    }
    
    /* link  and error check */
    glLinkProgram(program);
    
    GLint  linked;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );
    if ( !linked ) {
        std::cerr << "Shader program failed to link" << std::endl;
        GLint  logSize;
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logSize);
        char* logMsg = new char[logSize];
        glGetProgramInfoLog( program, logSize, NULL, logMsg );
        std::cerr << logMsg << std::endl;
        delete [] logMsg;
        
        exit( EXIT_FAILURE );
    }
    
    /* use program object */
    //   glUseProgram(program);
    
    return program;
}

// Alternative single C++ method to Load/Compile/Link a pair for vertex and fragment shaders
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path)
{
    
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
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
	int InfoLogLength=10;
    
	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);
    
	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    if (!Result) 
    {
        glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        std::vector<char> VertexShaderErrorMessage(InfoLogLength);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
    }
	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);
    
	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	if (!Result) 
    {
        glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
    }
	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);
    
	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( std::max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
    
    
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
    
	return ProgramID;
}
//*********************************** SHADER Helper functions ***********************/


//*********************************** TEXTURE Helper functions ***********************/
#ifdef USE_MAGICK

Texture::Texture(GLenum textureTarget, const std::string& fileName)
{
    m_textureTarget = textureTarget;
    m_fileName      = fileName;
    m_pImage        = NULL;
	std::cout<< "Constructing texture is done with success! :"<<m_fileName<<std::endl;
}

bool Texture::loadTexture()
{
    try {
        m_pImage = new Magick::Image(m_fileName);
        m_pImage->write(&m_blob, "RGBA");
    } catch (Magick::Error& Error) {
        std::cout<< "Error loading texture: "<<m_fileName<<" :"<<Error.what() <<std::endl;
        return false;
    }
    //then if texture is loaded ok by ImageMagick
    glGenTextures(1, &m_textureObj);
    glBindTexture(m_textureTarget, m_textureObj);
    glTexImage2D(m_textureTarget, 0, GL_RGB, m_pImage->columns(), m_pImage->rows(), -0.5, GL_RGBA, GL_UNSIGNED_BYTE, m_blob.data());
    glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	std::cout<< "loading texture is done with success! :"<<m_fileName<<std::endl;

    return true;
}

void Texture::bindTexture(GLenum textureUnit)
{
    glActiveTexture(textureUnit);
    glBindTexture(m_textureTarget, m_textureObj);
}


#endif









