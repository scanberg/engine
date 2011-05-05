uniform sampler2D diffuseMap;
varying vec2 textCoord;

void main( void )
{
 	// Final fragment color
	gl_FragColor = texture2D(diffuseMap, textCoord);
}
