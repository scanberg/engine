#include "ShaderLibrary.h"
#include "Error.h"
#include <iostream>
#include <stdio.h>   // for printf() and sscanf()
#include <stdlib.h>  // for malloc() and free()
#include <string>

//#include <string>

const short TEX_NONE        = 0;
const short TEX_DIFFUSE     = 1;
const short TEX_NORMAL      = 2;
const short TEX_HEIGHT      = 4;
const short TEX_SPECULAR    = 8;

void ShaderLibrary::Init()
{
    shader[TEX_DIFFUSE]=LoadShader( "shaders/vertex_diffuse.glsl", "shaders/fragment_diffuse.glsl" );
    shader[TEX_DIFFUSE+TEX_NORMAL]=LoadShader( "shaders/vertex_diffuse.glsl", "shaders/fragment_diffuse.glsl" );
    shader[TEX_DIFFUSE+TEX_NORMAL]=LoadShader( "shaders/vertex_bump.glsl", "shaders/fragment_bump.glsl" );
    shader[TEX_DIFFUSE+TEX_NORMAL+TEX_SPECULAR]=LoadShader( "shaders/vertex_bump_spec.glsl", "shaders/fragment_bump_spec.glsl" );
    shader[TEX_DIFFUSE+TEX_NORMAL+TEX_HEIGHT]=LoadShader( "shaders/vertex_parallax.glsl", "shaders/fragment_parallax.glsl" );
    shader[TEX_DIFFUSE+TEX_NORMAL+TEX_HEIGHT+TEX_SPECULAR]=LoadShader( "shaders/vertex_parallax_spec.glsl", "shaders/fragment_parallax_spec.glsl" );
}

GLuint ShaderLibrary::GetShaderFromType(short type)
{
    std::map<short, GLuint>::iterator mit;
    mit=shader.find(type);

    if(mit!=shader.end())
        return mit->second;

    return 0;
}

GLuint ShaderLibrary::GetShaderFromDistance(short type, float dist)
{
    short minType;
    if(dist < 100)
    {
        minType=std::min(type,(short)(TEX_DIFFUSE+TEX_NORMAL+TEX_HEIGHT+TEX_SPECULAR));
    }
    else if(dist < 200)
    {
        minType=std::min(type,(short)(TEX_DIFFUSE+TEX_NORMAL));
    }
    else
    {
        minType=std::min(type,(short)TEX_DIFFUSE);
    }
    return shader[minType];
}

//TAGEN FRÅN http://www.gamedev.net/topic/211918-determining-if-a-file-exists-c/
bool fileExists(std::string strFilename)
{
    FILE* fp = NULL;

    fp = fopen( strFilename.c_str(), "rb" );
    if( fp != NULL )
    {
        fclose( fp );
        return true;
    }

    return false;
}

/*
 * filelength - Determine the number of bytes in a file.
 * This is a lazy hack to avoid calling stat(), but it works. /STEGU
 */
int filelength( const char *filename ) {
    FILE *ifp;
    int length = 0;

    ifp = fopen( filename, "r" );
    fseek( ifp, 0, SEEK_END );
    length = (int)ftell( ifp );
    fclose( ifp );
    return length;
}

/*
 * readShaderFile - read shader source from a file to a string. /STEGU
 */
char* readShaderFile( const char *filename ) {
  FILE *file = fopen( filename, "r" );
  if( file == NULL )
    {
      printError( "I/O error", "Cannot open shader file!" );
      return 0;
    }
  int bytesinfile = filelength( filename );
  char *buffer = (char*)malloc( bytesinfile+1 );
  int bytesread = fread( buffer, 1, bytesinfile, file );
  buffer[bytesread] = 0; // Terminate the string with 0
  fclose( file );

  return buffer;
}

//CREDS TO STEGU
GLuint ShaderLibrary::LoadShader( const char *vertfilename, const char *fragfilename )
{
    GLuint programObj;
    GLuint fragmentShader;
    GLuint vertexShader;
    const char *vertexShaderStrings[1];
    GLint vertexCompiled;
    const char *fragmentShaderStrings[1];
    GLint fragmentCompiled;
    GLint shadersLinked;
    char str[4096]; // For error messages from the GLSL compiler and linker

    std::cout<<"Creating shader "<<vertfilename<<std::endl;
    // Create the vertex and fragment shaders
    vertexShader = glCreateShader( GL_VERTEX_SHADER );

    char *vertexShaderAssembly = readShaderFile( vertfilename );
    vertexShaderStrings[0] = vertexShaderAssembly;
    glShaderSource( vertexShader, 1, vertexShaderStrings, NULL );
    glCompileShader( vertexShader );
    free( (void *)vertexShaderAssembly );

    glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &vertexCompiled );
    if( vertexCompiled == GL_FALSE )
    {
        glGetShaderInfoLog( vertexShader, sizeof(str), NULL, str );
        printError( "Vertex shader compile error", str );
    }

    std::cout<<"Creating shader "<<fragfilename<<std::endl;

    fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

    char *fragmentShaderAssembly = readShaderFile( fragfilename );
    fragmentShaderStrings[0] = fragmentShaderAssembly;
    glShaderSource( fragmentShader, 1, fragmentShaderStrings, NULL );
    glCompileShader( fragmentShader );
    free( (void *)fragmentShaderAssembly );

    glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &fragmentCompiled );
    if( fragmentCompiled == GL_FALSE )
    {
        glGetShaderInfoLog( fragmentShader, sizeof(str), NULL, str );
        printError( "Fragment shader compile error", str );
    }

    // Create a program object and attach the compiled shaders
    programObj = glCreateProgram();
    glAttachShader( programObj, vertexShader );
    glAttachShader( programObj, fragmentShader );

    // Link the program object and print out the info log
    glLinkProgram( programObj );
    glGetProgramiv( programObj, GL_LINK_STATUS, &shadersLinked );

    if( shadersLinked == GL_FALSE )
    {
        glGetProgramInfoLog( programObj, sizeof(str), NULL, str );
        printError( "Program object linking error", str );
    }

    return programObj;
}

void setAttributeTangent(GLuint programObj, const GLvoid* tangentPointer, std::string name)
{
    GLint location_var = -1;
    location_var = glGetAttribLocation(programObj, name.c_str());
    if(location_var != -1)
    {
        glEnableVertexAttribArray(location_var);
        glVertexAttribPointer(location_var, 3, GL_FLOAT, GL_FALSE, 0, tangentPointer);
    }
}

void setUniform1f( GLuint programObj, GLfloat var, std::string name)
{
    GLint location_var = -1;

    // Activate the shader to set its state
    glUseProgram( programObj );

        // Locate the uniform shader variables by name and set them:
        location_var = glGetUniformLocation( programObj, name.c_str() );
        if(location_var != -1)
            glUniform1f( location_var, var);

    // Deactivate the shader again
    glUseProgram( 0 );
}

void setUniform2f( GLuint programObj, GLfloat x, GLfloat y, std::string name)
{
    GLint location_var = -1;

    // Activate the shader to set its state
    glUseProgram( programObj );

        // Locate the uniform shader variables by name and set them:
        location_var = glGetUniformLocation( programObj, name.c_str() );
        if(location_var != -1)
            glUniform2f( location_var, x, y);

    // Deactivate the shader again
    glUseProgram( 0 );
}

void setUniform3f( GLuint programObj, GLfloat x, GLfloat y, GLfloat z, std::string name)
{
    GLint location_var = -1;

    // Activate the shader to set its state
    glUseProgram( programObj );

        // Locate the uniform shader variables by name and set them:
        location_var = glGetUniformLocation( programObj, name.c_str() );
        if(location_var != -1)
            glUniform3f( location_var, x, y, z);

    // Deactivate the shader again
    glUseProgram( 0 );
}

void setUniform4f( GLuint programObj, GLfloat x, GLfloat y, GLfloat z, GLfloat w, std::string name)
{
    GLint location_var = -1;

    // Activate the shader to set its state
    glUseProgram( programObj );

        // Locate the uniform shader variables by name and set them:
        location_var = glGetUniformLocation( programObj, name.c_str() );
        if(location_var != -1)
            glUniform4f( location_var, x, y, z, w);

    // Deactivate the shader again
    glUseProgram( 0 );
}

void setUniformMatrix4fv( GLuint programObj, const GLfloat *mat, std::string name)
{
    GLint location_var = -1;

    // Activate the shader to set its state
    glUseProgram( programObj );

        // Locate the uniform shader variables by name and set them:
        location_var = glGetUniformLocation( programObj, name.c_str() );
        if(location_var != -1)
            glUniformMatrix4fv(location_var, 4, GL_FALSE, mat );

    // Deactivate the shader again
    glUseProgram( 0 );
}

void setUniform1i( GLuint programObj, GLint var, std::string name)
{
    GLint location_var = -1;

    // Activate the shader to set its state
    glUseProgram( programObj );

        // Locate the uniform shader variables by name and set them:
        location_var = glGetUniformLocation( programObj, name.c_str() );
        if(location_var != -1)
            glUniform1i( location_var, var );
    // Deactivate the shader again
    glUseProgram( 0 );
}

void setUniform1fv( GLuint programObj, GLint size, GLfloat *var, std::string name)
{
    GLint location_var = -1;

    // Activate the shader to set its state
    glUseProgram( programObj );

        // Locate the uniform shader variables by name and set them:
        location_var = glGetUniformLocation( programObj, name.c_str() );
        if(location_var != -1)
            glUniform1fv(location_var, size, var);

    // Deactivate the shader again
    glUseProgram( 0 );
}

void setUniform2fv( GLuint programObj, GLint size, GLfloat *var, std::string name)
{
    GLint location_var = -1;

    // Activate the shader to set its state
    glUseProgram( programObj );

        // Locate the uniform shader variables by name and set them:
        location_var = glGetUniformLocation( programObj, name.c_str() );
        if(location_var != -1)
            glUniform2fv(location_var, size, var);

    // Deactivate the shader again
    glUseProgram( 0 );
}

void setUniform3fv( GLuint programObj, GLint size, GLfloat *var, std::string name)
{
    GLint location_var = -1;

    // Activate the shader to set its state
    glUseProgram( programObj );

        // Locate the uniform shader variables by name and set them:
        location_var = glGetUniformLocation( programObj, name.c_str() );
        if(location_var != -1)
            glUniform3fv(location_var, size, var);

    // Deactivate the shader again
    glUseProgram( 0 );
}

