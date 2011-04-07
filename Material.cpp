#define GLEW_STATIC
#include <GL/glew.h>

#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdio.h>   // for printf() and sscanf()
#include <stdlib.h>  // for malloc() and free()
#include "Material.h"
#include "Error.h"

using namespace std;

bool FileExists(string strFilename);

void LoadMaterial(string s, Material &mat)
{
    //Sätt materialets typ till none, i fallet då ingen textur hittas för materialet.
    mat.type = NONE;
    if(FileExists(s+"_diffuse.tga"))
    {
        cout<<"HEJ"<<endl;
        glEnable(GL_TEXTURE_2D); // Enable texturing
        glGenTextures(1, &mat.diffuseMap); // Generate 1 unique texture IDs to use
        glBindTexture(GL_TEXTURE_2D, mat.diffuseMap); // Activate first texture
        glfwLoadTexture2D((s+"_diffuse.tga").c_str(), GLFW_BUILD_MIPMAPS_BIT); // Load image

        // when texture area is small, bilinear filter the closest mipmap
        //glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
        // when texture area is large, bilinear filter the original
        //glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );

        // Specify trilinear interpolation
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );


        mat.type=DIFF;
    }
    if(FileExists(s+"_specular.tga"))
    {
        glEnable(GL_TEXTURE_2D); // Enable texturing
        glGenTextures(1, &mat.specularMap); // Generate 1 unique texture IDs to use
        glBindTexture(GL_TEXTURE_2D, mat.specularMap); // Activate first texture

        //SKALL MIPMAPS ANVÄNDAS??
        glfwLoadTexture2D((s+"_specular.tga").c_str(),GLFW_ALPHA_BITS); // Load image
        mat.type=DIFF_SPEC;
    }
    if(FileExists(s+"_normal.tga"))
    {
        glEnable(GL_TEXTURE_2D); // Enable texturing
        glGenTextures(1, &mat.normalMap); // Generate 1 unique texture IDs to use
        glBindTexture(GL_TEXTURE_2D, mat.normalMap); // Activate first texture

        //SKALL MIPMAPS ANVÄNDAS??
        glfwLoadTexture2D((s+"_normal.tga").c_str(), GLFW_BUILD_MIPMAPS_BIT); // Load image

        // Specify trilinear interpolation
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

        mat.type=DIFF_SPEC_NORM;
    }
    if(FileExists(s+"_height.tga"))
    {
        glEnable(GL_TEXTURE_2D); // Enable texturing
        glGenTextures(1, &mat.heightMap); // Generate 1 unique texture IDs to use
        glBindTexture(GL_TEXTURE_2D, mat.heightMap); // Activate first texture

        //SKALL MIPMAPS ANVÄNDAS??
        glfwLoadTexture2D((s+"height.tga").c_str(), GLFW_ALPHA_BITS); // Load image

        // Specify trilinear interpolation
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

        mat.type=DIFF_SPEC_NORM_HEIGHT;
    }

    //Tilldela standardvärden
    mat.ambient[0]=mat.ambient[1]=mat.ambient[2] = 0.1;
    mat.diffuse[0]=mat.diffuse[1]=mat.diffuse[2] = 1.0;
    mat.specular[0]=mat.specular[1]=mat.specular[2] = 0.5;
    mat.shininess[0] = 50.0;
}

//TAGEN FRÅN http://www.gamedev.net/topic/211918-determining-if-a-file-exists-c/
bool FileExists(string strFilename)
{
    FILE* fp = NULL;

    //will not work if you do not have read permissions
    //to the file, but if you don''t have read, it
    //may as well not exist to begin with.

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


/*
 * createShader - create, load, compile and link a shader program object. /STEGU
 */
GLuint createShader( const char *vertfilename, const char *fragfilename ) {

  GLuint programObj;
  GLuint fragmentShader;
  GLuint vertexShader;
  const char *vertexShaderStrings[1];
  GLint vertexCompiled;
  const char *fragmentShaderStrings[1];
  GLint fragmentCompiled;
  GLint shadersLinked;
  char str[4096]; // For error messages from the GLSL compiler and linker

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

 void setUniformVariables( GLuint programObj,
			   int diff, int norm, int height, const GLvoid* tangentPointer) {

    GLint location_diff = -1;
    GLint location_norm = -1;
    GLint location_height = -1;
    GLint location_tang = -1;

    // Activate the shader to set its state
    glUseProgram( programObj );

        // Locate the uniform shader variables by name and set them:
        location_diff = glGetUniformLocation( programObj, "diffuseMap" );
        if(location_diff != -1)
            glUniform1i( location_diff, diff );

        location_norm = glGetUniformLocation( programObj, "normalMap" );
        if(location_norm != -1)
            glUniform1i( location_norm, norm );

        location_norm = glGetUniformLocation( programObj, "heightMap" );
        if(location_height != -1)
            glUniform1i( location_height, height );

        location_tang = glGetAttribLocation(programObj, "tangent");
        if(location_tang != -1)
        {
            glEnableVertexAttribArray(location_tang);
            glVertexAttribPointer(location_tang, 3, GL_FLOAT,GL_FALSE, 0, tangentPointer);
        }

    // Deactivate the shader again
    glUseProgram( 0 );
}

void LoadShader(std::string svertex, std::string sfragment, Material &mat)
{
    mat.shader = createShader(svertex.c_str(), sfragment.c_str());
}
