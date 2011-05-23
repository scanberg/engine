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
#include "SceneHandler.h"

using std::string;

Material::Material()
{
    //Tilldela standardvärden
    ambient[3]=diffuse[3]=specular[3]=1.0f;
    setAmbient(1.0);
    setDiffuse(1.0);
    setSpecular(0.2);
    setShininess(50.0);
    type=TEX_NONE;
}

void Material::setAmbient(float f) { setAmbient(f,f,f); }

void Material::setAmbient(float r, float g, float b)
{
    this->ambient[0]=r;
    this->ambient[1]=g;
    this->ambient[2]=b;
}

void Material::setDiffuse(float f) { setDiffuse(f,f,f); }

void Material::setDiffuse(float r, float g, float b)
{
    this->diffuse[0]=r;
    this->diffuse[1]=g;
    this->diffuse[2]=b;
}

void Material::setSpecular(float f) { setSpecular(f,f,f); }

void Material::setSpecular(float r, float g, float b)
{
    this->specular[0]=r;
    this->specular[1]=g;
    this->specular[2]=b;
}

void Material::setShininess(float f)
{
    this->shininess[0]=f;
}

bool FileExists(std::string strFilename);
GLFWimage mergeNormalAndHeight(std::string s);

int LoadMaterial(const std::string& s, Material& mat)
{
    //Sätt materialets typ till none, i fallet då ingen textur hittas för materialet.
    mat.type = TEX_NONE;

    if(FileExists(s+"_diffuse.tga"))
    {
        mat.diffuseMap=SceneHandler::resources.loadTexture(s+"_diffuse.tga",GLFW_BUILD_MIPMAPS_BIT);
        glBindTexture(GL_TEXTURE_2D, mat.diffuseMap);

        // Specify trilinear interpolation
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

        glBindTexture(GL_TEXTURE_2D, 0);

        mat.type+=TEX_DIFFUSE;
    }

    if(FileExists(s+"_specular.tga"))
    {

        mat.specularMap=SceneHandler::resources.loadTexture(s+"_specular.tga",GLFW_BUILD_MIPMAPS_BIT);
        glBindTexture(GL_TEXTURE_2D, mat.specularMap);
        mat.type+=TEX_SPECULAR;
    }

    if(FileExists(s+"_normal.tga"))
    {

        mat.normalMap=SceneHandler::resources.loadTexture(s+"_normal.tga",GLFW_BUILD_MIPMAPS_BIT);
        glBindTexture(GL_TEXTURE_2D, mat.normalMap);

        //Finns en heightmap så släng in den i alpha-kanalen, RGBA
        if(FileExists(s+"_height.tga"))
        {
            GLFWimage merged;
            merged = mergeRGB_A(&(s+"_normal.tga"),&(s+"_height.tga"));
            glfwLoadTextureImage2D(&merged, GLFW_BUILD_MIPMAPS_BIT);
            glfwFreeImage(&merged);
            mat.type+=TEX_HEIGHT;
        }

        // Specify trilinear interpolation
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

        mat.type+=TEX_NORMAL;
    }

    //Ladda rätt shader beroende på vilka texturer som hittades.
    switch(mat.type)
    {
        case TEX_DIFFUSE:
            mat.shader = createShader( "shaders/vertex_diffuse.glsl", "shaders/fragment_diffuse.glsl" );
            break;

        case TEX_DIFFUSE+TEX_NORMAL:
            mat.shader = createShader( "shaders/vertex_bump.glsl", "shaders/fragment_bump.glsl" );
            break;

        case TEX_DIFFUSE+TEX_NORMAL+TEX_HEIGHT:
            mat.shader = createShader( "shaders/vertex_parallax.glsl", "shaders/fragment_parallax.glsl" );
            break;

        case TEX_DIFFUSE+TEX_NORMAL+TEX_HEIGHT+TEX_SPECULAR:
            mat.shader = createShader( "shaders/vertex_parallax_spec.glsl", "shaders/fragment_parallax_spec.glsl" );
            break;

        default:
            mat.shader = createShader( "shaders/vertex_diffuse.glsl", "shaders/fragment_diffuse.glsl" );
    }
    return 1;
}

void Material::loadShader()
{
    //Ladda rätt shader beroende på vilka texturer som hittades.
    std::cout<<"Type "<<type;
    switch(type)
    {
        case TEX_DIFFUSE:
            shader = createShader( "shaders/vertex_diffuse.glsl", "shaders/fragment_diffuse.glsl" );
            break;

        case TEX_DIFFUSE+TEX_NORMAL:
            shader = createShader( "shaders/vertex_bump.glsl", "shaders/fragment_bump.glsl" );
            break;

        case TEX_DIFFUSE+TEX_NORMAL+TEX_SPECULAR:
            shader = createShader( "shaders/vertex_bump_spec.glsl", "shaders/fragment_bump_spec.glsl" );
            break;

        case TEX_DIFFUSE+TEX_NORMAL+TEX_HEIGHT:
            shader = createShader( "shaders/vertex_parallax.glsl", "shaders/fragment_parallax.glsl" );
            break;

        case TEX_DIFFUSE+TEX_NORMAL+TEX_HEIGHT+TEX_SPECULAR:
            shader = createShader( "shaders/vertex_parallax_spec.glsl", "shaders/fragment_parallax_spec.glsl" );
            break;

        default:
            shader = createShader( "shaders/vertex_diffuse.glsl", "shaders/fragment_diffuse.glsl" );
    }
}

Material LoadMaterial(const std::string& s)
{
    Material mat;

    LoadMaterial(s,mat);

    return mat;
}

/*
    *** mergeNormalAndHeight ***

    Slår samman Normal och Height -map.
    Normal fyller RGB kanalerna och Height lagras i A kanalen.

    TODO:
    Kontrollera så att Width och Height är lika för normal och height.
*/
GLFWimage mergeRGB_A(std::string *rgbFile, std::string *aFile)
{
    GLFWimage img;
    GLFWimage normImg;
    GLFWimage heightImg;

    glfwReadImage(rgbFile->c_str(), &normImg, GLFW_NO_RESCALE_BIT); //GLFW_NO_RESCALE_BIT

    glfwReadImage(aFile->c_str(), &heightImg, GLFW_NO_RESCALE_BIT);

    unsigned char *rgbaData = new unsigned char[normImg.Width * normImg.Height * 4];

    for(int i=0; i<(normImg.Width * normImg.Height); i++)
    {
        rgbaData[4*i]   = normImg.Data[3*i];    //R
        rgbaData[4*i+1] = normImg.Data[3*i+1];  //G
        rgbaData[4*i+2] = normImg.Data[3*i+2];  //B
        rgbaData[4*i+3] = heightImg.Data[i];  //A
    }

    img.Format = GL_RGBA;
    img.BytesPerPixel = 4;
    img.Width = normImg.Width;
    img.Height = normImg.Height;
    img.Data = rgbaData;

    glfwFreeImage(&heightImg);
    glfwFreeImage(&normImg);

    return img;
}

GLFWimage mergeRGB_A(GLuint *rgb_tex, GLuint *a_tex)
{
    GLFWimage img;
    int width, height;

    glBindTexture(GL_TEXTURE_2D, *rgb_tex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    GLubyte *rgbData = new GLubyte[width*height*3];
    glGetTexImage(	GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbData);

    glBindTexture(GL_TEXTURE_2D, *a_tex);
    GLubyte *aData = new GLubyte[width*height];
    glGetTexImage(	GL_TEXTURE_2D, 0, GL_R, GL_UNSIGNED_BYTE, aData);

    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned char *rgbaData = new unsigned char[ width*height*4 ];

    for(int i=0; i<(width*height); i++)
    {
        rgbaData[4*i]   = rgbData[3*i];     //R
        rgbaData[4*i+1] = rgbData[3*i+1];   //G
        rgbaData[4*i+2] = rgbData[3*i+2];   //B
        rgbaData[4*i+3] = aData[i];         //A
    }

    img.Format = GL_RGBA;
    img.BytesPerPixel = 4;
    img.Width = width;
    img.Height = height;
    img.Data = rgbaData;

    return img;
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

void setUniformVariables( GLuint programObj,
			   int diff, int norm, int spec, const GLvoid* tangentPointer) {

    GLint location_diff = -1;
    GLint location_norm = -1;
    GLint location_spec = -1;
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

        location_spec = glGetUniformLocation( programObj, "specularMap" );
        if(location_spec != -1)
            glUniform1i( location_spec, spec );

        location_tang = glGetAttribLocation(programObj, "tangent");
        if(location_tang != -1)
        {
            glEnableVertexAttribArray(location_tang);
            glVertexAttribPointer(location_tang, 3, GL_FLOAT,GL_FALSE, 0, tangentPointer);
        }

    // Deactivate the shader again
    glUseProgram( 0 );
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

void setUniform2f( GLuint programObj, GLfloat *var, std::string name)
{
    GLint location_var = -1;

    // Activate the shader to set its state
    glUseProgram( programObj );

        // Locate the uniform shader variables by name and set them:
        location_var = glGetUniformLocation( programObj, name.c_str() );
        if(location_var != -1)
            glUniform2f( location_var, var[0], var[1]);

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
