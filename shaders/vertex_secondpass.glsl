varying vec2 texCoord;

uniform vec2 vertex;
	
void main(void)
{
	gl_Position = gl_Vertex;
	texCoord = gl_Vertex.xy * 0.5 + 0.5;
}