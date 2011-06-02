#ifndef ParticleSystem_H
#define ParticleSystem_H
class ParticleSystem
{
	public:
		ParticleSystem(); //empty constructor
		static void	Init();
		static void Update();
		static void Render();
		static const int NUM_PARTICLES = 100;
	private:
		static int delay[NUM_PARTICLES];
		static glm::vec3 particle_array[NUM_PARTICLES];
		static float xdir[NUM_PARTICLES], ydir[NUM_PARTICLES], life[NUM_PARTICLES];
		static GLuint particle_texture, shader;
};
#endif
