varying vec2 texCoord;
varying vec3 normalVec;
	
void main(void)
{
	gl_Position = ftransform();
	texCoord = gl_MultiTexCoord0.xy;
		
	normalVec = gl_NormalMatrix * gl_Normal;
}