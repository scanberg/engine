uniform float life;
uniform vec2 cameraRange;
uniform vec2 screenSize;
uniform sampler2D textureMap;
uniform sampler2D depthMap;

varying vec2 st;
varying float z;

void main( void )
{
	vec2 coords = gl_FragCoord.xy / screenSize.xy;
	float depth = -texture2D(depthMap,coords).x*cameraRange.y;

	vec4 color = texture2D(textureMap,st);
	
	if(color.xyz == vec3(0))
		discard;

	if(z < depth)
		discard;

	vec4 finalColor = vec4(color.xyz, color.w * life);


	z = z / cameraRange.y;
	const float LOG2 = 1.442695;
	float density = 1;
	float fogFactor = exp2( - density * density * z * z * LOG2 );
	fogFactor = clamp(fogFactor, 0.0, 1.0);

	gl_FragColor = mix(gl_Fog.color, finalColor, fogFactor);
}
