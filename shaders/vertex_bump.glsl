varying vec2 texCoord;
varying vec3 normalVec;
varying vec3 tangentVec;
varying float depth;

uniform vec2 cameraRange;

attribute vec3 tangent;
	
void main(void)
{
	gl_Position = ftransform();
	texCoord = gl_MultiTexCoord0.xy;
		
	normalVec = gl_NormalMatrix * gl_Normal;
	tangentVec = gl_NormalMatrix * tangent;

	vec4 viewPos = gl_ModelViewMatrix * gl_Vertex;
	depth = (-viewPos.z-cameraRange.x)/(cameraRange.y-cameraRange.x);
}