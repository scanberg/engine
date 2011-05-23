#include "Light.h"
#include <cstdio>

Light::Light()
{
    setDiffuse(1.0f,1.0f,1.0f);
    setPosition(0.0f,0.0f,0.0f);
    setDirection(0.0f,0.0f,-10.0f);
    diffuse[3]=ambient[3]=specular[3]=position[3]=direction[3]=1.0f;
    radius=500.0f;
    cutoff=45.0f;
    generateShadowFBO();
}

void Light::generateShadowFBO()
{
	int shadowMapWidth = SHADOW_MAP_WIDTH;
	int shadowMapHeight = SHADOW_MAP_HEIGHT;

	//GLfloat borderColor[4] = {0,0,0,0};

	GLenum FBOstatus;

	// Try to use a texture depth component
	glGenTextures(1, &depthTextureId);
	glBindTexture(GL_TEXTURE_2D, depthTextureId);

	// GL_LINEAR does not make sense for depth texture. However, next tutorial shows usage of GL_LINEAR and PCF
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Remove artefact on the edges of the shadowmap
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	//glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );

	// No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// create a framebuffer object
	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);

	// Instruct openGL that we won't bind a color texture with the currently binded FBO
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// attach the texture to FBO depth attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D, depthTextureId, 0);

	// check FBO status
	FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(FBOstatus != GL_FRAMEBUFFER_COMPLETE)
		printf("GL_FRAMEBUFFER_COMPLETE_EXT failed, CANNOT use FBO\n");

    printf("Fbo %i textureid %i \n",fboId,depthTextureId);

	// switch back to window-system-provided framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Light::setupMatrices()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(2*cutoff,SHADOW_MAP_WIDTH/SHADOW_MAP_HEIGHT,1,radius);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    gluLookAt(position[0],position[1],position[2],position[0]+direction[0],position[1]+direction[1],position[2]+direction[2],0,0,1);
}

void Light::setTextureMatrix()
{
	static double modelView[16];
	static double projection[16];

	// This is matrix transform every coordinate x,y,z
	// x = x* 0.5 + 0.5
	// y = y* 0.5 + 0.5
	// z = z* 0.5 + 0.5
	// Moving from unit cube [-1,1] to [0,1]
	const GLdouble bias[16] = {
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0};

	// Grab modelview and transformation matrices
	glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	glMatrixMode(GL_TEXTURE);
	glActiveTexture(GL_TEXTURE3);

	glLoadIdentity();
	glLoadMatrixd(bias);

	// concatating all matrice into one.
	glMultMatrixd (projection);
	glMultMatrixd (modelView);

	// Go back to normal matrix mode
	glMatrixMode(GL_MODELVIEW);
}

void Light::assignTo(unsigned int i)
{
    glLightfv(GL_LIGHT0+i, GL_POSITION, position);
    glLightfv(GL_LIGHT0+i, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0+i, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0+i, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION, direction);
    glLightf(GL_LIGHT0+i, GL_SPOT_CUTOFF, cutoff);
}
