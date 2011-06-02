// GLSL demo code, Stefan Gustavson 2009
// This code is in the public domain.

uniform float life;
uniform sampler2D tex;
varying vec2 st;

void main( void )
{
  //vec4 color1 = vec4(1.0, 1.0, 1.0, 1.0);
  //vec4 color2 = vec4(0.0, 0.0, 0.0, 1.0);
  //vec2 coords = fract(st) - 0.5;
	//float radius = length(coords);
	//float aastep = 0.7*fwidth(radius);
  //float dots = smoothstep(0.2-aastep, 0.2+aastep, radius);

  // Final fragment color
  //gl_FragColor = mix(color1, color2, dots);
	gl_FragColor = vec4(texture2D(tex,st).xyz, texture2D(tex,st).w * life);
	
}
