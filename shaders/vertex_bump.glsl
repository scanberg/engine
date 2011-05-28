varying vec2 texCoord;
varying vec3 normalVec;
varying vec3 tangentVec;

attribute vec3 tangent;
	
void main(void)
{
	gl_Position = ftransform();
	texCoord = gl_MultiTexCoord0.xy;
		
	normalVec = gl_NormalMatrix * gl_Normal;
	tangentVec = gl_NormalMatrix * tangent;
}