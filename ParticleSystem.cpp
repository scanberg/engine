#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glfw.h>
#include "SceneHandler.h"
#include "MD5Model.h"
#include "ParticleSystem.h"

using glm::vec3;


int ParticleSystem::delay[NUM_PARTICLES];
glm::vec3 ParticleSystem::particle_array[NUM_PARTICLES], ParticleSystem::color_array[NUM_PARTICLES];
float ParticleSystem::xdir[NUM_PARTICLES], ParticleSystem::ydir[NUM_PARTICLES];
GLuint ParticleSystem::particle_texture, ParticleSystem::alphachan;

void ParticleSystem::Init()
{
	for (int i=0; i<NUM_PARTICLES; i++)
	{
		particle_array[i] = vec3(0.0f,0.0f,0.0f);
		color_array[i] = vec3(1.0f,0.1f,0.0f);
		float random = ((rand() % 10) - 5.0)/5.0;
		xdir[i] = random;
		random = ((rand() % 10) - 5.0)/5.0;
		ydir[i] = random;
		delay[i]=i;
	}
	//from lab6 in TNM061 at Linköping Uni.
	glGenTextures(1, &particle_texture);
	glBindTexture(GL_TEXTURE_2D, particle_texture);
	glfwLoadTexture2D("media/particle/particle_diffuse.tga",GLFW_BUILD_MIPMAPS_BIT);
	glEnable(GL_TEXTURE_2D);

	/*glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/

	glBindTexture(GL_TEXTURE_2D, 0);

}
void ParticleSystem::Update()
{
	for (int i=0; i<NUM_PARTICLES; i++)
	{
		if (delay[i] <= 0)
		{
			particle_array[i] += vec3(60.0f*xdir[i],60.0f*ydir[i],100.0f)*SceneHandler::g_dt;
			color_array[i] += vec3(0.0f,0.03f,0.0f);
		}
		if (particle_array[i].z > 30)
		{
			particle_array[i] = vec3(0.0f, 0.0f, 0.0f);
			color_array[i] = vec3(1.0f,0.1f,0.0f);
			float random = ((rand() % 20) - 10.0)/10.0;
			xdir[i] = random;
			random = ((rand() % 20) - 10.0)/10.0;
			ydir[i] = random;
		}
		delay[i]-=1;
	}
}
void ParticleSystem::Render()
{
    //billboard, get view coordinates
    //http://www.lighthouse3d.com/opengl/billboarding/index.php3?billCheat2
    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    vec3 right = vec3(modelview[0],modelview[4],modelview[8]);//vec3(1.0f,0.0f,0.0f);
    vec3 up = vec3(modelview[1],modelview[5],modelview[9]);//vec3(0.0f,0.0f,1.0f);

    glMatrixMode(GL_TEXTURE);
    glActiveTexture(GL_TEXTURE0);
    glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
        glColor4f(1.0f,1.0f,1.0f,0.0f);
        glClearColor(0,0,0,0);
        //http://www.andersriggelsen.dk/OpenGL/
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);//_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        //glColor3f(1.0f,1.0f,1.0f);
        glEnable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, particle_texture);
        glBegin( GL_QUADS );
            for ( int i=0; i<NUM_PARTICLES; i++)
            {

                    glColor3fv(glm::value_ptr(vec3(1.0f,0.0f,0.0f)));//color_array[i]));
                    glTexCoord2d(0.0,0.0);
                    glVertex3fv(glm::value_ptr(particle_array[i]+(-up - right)*1.0f));
                    glTexCoord2d(1.0,0.0);
                    glVertex3fv(glm::value_ptr(particle_array[i]+(-up + right)*1.0f));
                    glTexCoord2d(1.0,1.0);
                    glVertex3fv(glm::value_ptr(particle_array[i]+( up + right)*1.0f));
                    glTexCoord2d(0.0,1.0);
                    glVertex3fv(glm::value_ptr(particle_array[i]+( up - right)*1.0f));
            }
        glEnd();
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
    glPopMatrix();
}
