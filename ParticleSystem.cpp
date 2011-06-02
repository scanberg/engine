#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glfw.h>
#include "SceneHandler.h"
#include "MD5Model.h"
#include "ParticleSystem.h"

using glm::vec3;

//int ParticleSystem::delay[NUM_PARTICLES];
//glm::vec3 ParticleSystem::particle_array[NUM_PARTICLES];
//float ParticleSystem::xdir[NUM_PARTICLES], ParticleSystem::ydir[NUM_PARTICLES], ParticleSystem::life[NUM_PARTICLES];
//GLuint ParticleSystem::particle_texture, ParticleSystem::shader;

ParticleSystem::ParticleSystem()
{
    Init();
}

void ParticleSystem::Init()
{
	for (int i=0; i<NUM_PARTICLES; i++)
	{
		particle_array[i] = vec3(0.0f,0.0f,0.0f);
		float random = ((rand() % 10) - 5.0)/5.0;
		xdir[i] = random;
		random = ((rand() % 10) - 5.0)/5.0;
		ydir[i] = random;
		delay[i]=i;
		life[i]=0.0;
	}

	particle_texture = SceneHandler::resources.loadTexture("media/particle/particle_diffuse.tga",GLFW_BUILD_MIPMAPS_BIT);
	shader = SceneHandler::shaderLib.LoadShader("shaders/vertex_particle.glsl", "shaders/fragment_particle.glsl");

}
void ParticleSystem::Update()
{
	for (int i=0; i<NUM_PARTICLES; i++)
	{
		if (delay[i] <= 0)
		{
			particle_array[i] += vec3(10.2f*xdir[i]*life[i]*SceneHandler::g_dt,10.2f*ydir[i]*life[i]*SceneHandler::g_dt,50.0f*SceneHandler::g_dt);
			life[i] += 15.0*SceneHandler::g_dt;
		}
		if (life[i] >= 3.2)
		{
			particle_array[i] = vec3( ((rand() % 10)-5.0)/5.0, ((rand() % 10)-5.0)/5.0, 0.0f);
			float random = ((rand() % 20) - 10.0)/10.0;
			xdir[i] = random;
			random = ((rand() % 20) - 10.0)/10.0;
			ydir[i] = random;
			life[i] = 0.0;
		}
		if (delay[i] >0)
			delay[i]-=1;
	}
}
void ParticleSystem::Draw()
{
			//billboard, get view coordinates
			//http://www.lighthouse3d.com/opengl/billboarding/index.php3?billCheat2
			float modelview[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
			vec3 right = vec3(modelview[0],modelview[4],modelview[8]);
			vec3 up = vec3(modelview[1],modelview[5],modelview[9]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, particle_texture);
			setUniform1i(shader,0,"textureMap");

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, SceneHandler::deferred.depthMap);
			setUniform1i(shader,1,"depthMap");

            setUniform2f(shader,SceneHandler::deferred.width,SceneHandler::deferred.height,"screenSize");
            setUniform2f(shader,SceneHandler::near,SceneHandler::far,"cameraRange");

			glPushMatrix();

                glMultMatrixf(&matrix[0][0]);
                glScalef(scale,scale,scale);

				//glEnable(GL_TEXTURE_2D);
				glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
				glColor4f(1.0f,1.0f,1.0f,0.0f);
				glClearColor(0,0,0,0);
				//http://www.andersriggelsen.dk/OpenGL/
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);//_MINUS_SRC_ALPHA);
				glEnable(GL_BLEND);
				glAlphaFunc(GL_GREATER,0);
//************************************************************************************
				glDepthMask(GL_FALSE);
//************************************************************************************

					for ( int i=0; i<NUM_PARTICLES; i++)
					{

						if(life[i] < 0.5)
                            setUniform1f(shader, 0.5, "life");
						else if(life[i] > 3.5)
                            setUniform1f(shader, 1/life[i]/life[i], "life");
						else
                            setUniform1f(shader, 1/life[i], "life");
                        glUseProgram(shader);
						glBegin( GL_QUADS );
							glTexCoord2d(0.0,0.0);
							glVertex3fv(glm::value_ptr(particle_array[i]+(-up - right)*1.0f));
							glTexCoord2d(1.0,0.0);
							glVertex3fv(glm::value_ptr(particle_array[i]+(-up + right)*1.0f));
							glTexCoord2d(1.0,1.0);
							glVertex3fv(glm::value_ptr(particle_array[i]+( up + right)*1.0f));
							glTexCoord2d(0.0,1.0);
							glVertex3fv(glm::value_ptr(particle_array[i]+( up - right)*1.0f));
						glEnd();
						glUseProgram(0);
					}
			glBindTexture(GL_TEXTURE_2D, 0);
			glDepthMask(GL_TRUE);

            glDisable(GL_BLEND);
		glPopMatrix();
}
