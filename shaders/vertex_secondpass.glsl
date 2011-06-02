varying vec2 texCoord;

uniform vec2 vertex;
uniform vec2 screenSize;
	
void main(void)
{
	gl_Position = gl_Vertex;
	texCoord = gl_Vertex.xy * 0.5 + 0.5;
	//texCoord = (gl_ModelViewMatrix * gl_Vertex).xy / screenSize;
}