// GLSL test code, Stefan Gustavson 2009
// This code is in the public domain.

uniform sampler2D tex;
uniform float life;
varying vec2 st;
varying float z;


void main( void )
{
  	st = gl_MultiTexCoord0.xy;
  	gl_Position = ftransform();

	z = (gl_ModelViewMatrix * gl_Vertex).z;
}
