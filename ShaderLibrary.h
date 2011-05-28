#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glfw.h>

#include <map>
#include <string>

class ShaderLibrary
{
    public:
    void Init();
    GLuint GetShaderFromDistance(short type, float dist);
    GLuint GetShaderFromType(short type);

    GLuint LoadShader( const char *vertfilename, const char *fragfilename );

    private:
    std::map<short, GLuint> shader;
};

void setUniform1i( GLuint programObj, GLint var, std::string name);

void setUniform1f( GLuint programObj, GLfloat var, std::string name);
void setUniform2f( GLuint programObj, GLfloat x, GLfloat y, std::string name);
void setUniform3f( GLuint programObj, GLfloat x, GLfloat y, GLfloat z, std::string name);
void setUniform4f( GLuint programObj, GLfloat x, GLfloat y, GLfloat z, GLfloat w, std::string name);

void setUniform1fv( GLuint programObj, GLint size, GLfloat *var, std::string name);
void setUniform2fv( GLuint programObj, GLint size, GLfloat *var, std::string name);
void setUniform3fv( GLuint programObj, GLint size, GLfloat *var, std::string name);

void setAttributeTangent(GLuint programObj, const GLvoid* tangentPointer, std::string name);
