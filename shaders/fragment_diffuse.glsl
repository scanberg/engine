uniform sampler2D diffuseMap;
varying vec2 texCoord;

void main( void )
{
 	// Final fragment color
	gl_FragColor = texture2D(diffuseMap, texCoord);
}
