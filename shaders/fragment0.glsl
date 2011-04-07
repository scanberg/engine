// GLSL demo code, Stefan Gustavson 2009
// This code is in the public domain.

uniform sampler2D tex;
varying vec2 st;

void main( void )
{
  // Final fragment color
  gl_FragColor = texture2D(tex, st);
}
