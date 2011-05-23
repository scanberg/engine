varying vec4 ShadowCoord;

uniform sampler2D ShadowMap;
	
void main (void)
{
	vec4 shadowCoordinateWdivide = ShadowCoord / ShadowCoord.w;
	//shadowCoordinateWdivide.z += 0.0001;

	float distanceFromLight = texture2D(ShadowMap,shadowCoordinateWdivide.st).z;
	
	float shadow = 1.0;

	if(shadowCoordinateWdivide.s<0.0 || shadowCoordinateWdivide.t<0.0
	||shadowCoordinateWdivide.s>1.0 || shadowCoordinateWdivide.t>1.0)
		shadow = 1.0;
	else if (ShadowCoord.w > 0.0)
		shadow = distanceFromLight < shadowCoordinateWdivide.z ? 0.5 : 1.0;

	gl_FragColor = vec4(shadow);
}