uniform sampler2D diffuseMap;
varying vec2 textCoord;

void main( void )
{
	// Pass the texture coordinates as st
	textCoord = gl_MultiTexCoord0.xy;
	gl_Position = ftransform();
}
