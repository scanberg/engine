varying vec4 ShadowCoord;
	
void main(void)
{
	gl_Position = ftransform();

	ShadowCoord = gl_TextureMatrix[3] * gl_Vertex;
}