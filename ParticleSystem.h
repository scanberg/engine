#ifndef ParticleSystem_H
#define ParticleSystem_H

#include "Math.h"
#include "Entity.h"

const int NUM_PARTICLES = 20;

class ParticleSystem : public Entity
{
	public:
		ParticleSystem(); //empty constructor
		void Init();
		void Update();
		void Draw();

	private:
		int delay[NUM_PARTICLES];
		glm::vec3 particle_array[NUM_PARTICLES];
		float xdir[NUM_PARTICLES], ydir[NUM_PARTICLES], life[NUM_PARTICLES];
		GLuint particle_texture, shader;

		struct Particle
		{
		    glm::vec3 pos, vel;
		    float life;
		};
};
#endif
