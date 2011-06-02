varying vec2 texCoord;
varying vec3 normalVec;
varying float depth;

uniform vec2 cameraRange;
	
void main(void)
{
	gl_Position = ftransform();
	texCoord = gl_MultiTexCoord0.xy;
		
	normalVec = gl_NormalMatrix * gl_Normal;

	vec4 viewPos = gl_ModelViewMatrix * gl_Vertex;
	depth = (-viewPos.z-cameraRange.x)/(cameraRange.y-cameraRange.x);
}