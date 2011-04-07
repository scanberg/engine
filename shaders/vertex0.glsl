// GLSL test code, Stefan Gustavson 2009
// This code is in the public domain.

uniform sampler2D tex;
varying vec2 st;

void main( void )
{
  // Pass the texture coordinates as st
  st = gl_MultiTexCoord0.xy;
  gl_Position = ftransform();
}
