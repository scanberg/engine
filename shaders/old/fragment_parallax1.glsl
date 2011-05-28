varying vec3 lightVec;
varying vec3 viewVec;
varying vec2 texCoord;
varying vec3 normal;
varying vec4 ShadowCoord;
varying vec3 lightDir;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D ShadowMap;
	
void main (void)
{
	vec4 finalColor = gl_LightSource[0].ambient * gl_FrontMaterial.ambient;

	vec4 shadowCoordinateWdivide = ShadowCoord / ShadowCoord.w;
	shadowCoordinateWdivide.z += 0.0001;

	//shadowCoordinateWdivide.s = clamp(shadowCoordinateWdivide.s,0.0,1.0);
	//shadowCoordinateWdivide.t = clamp(shadowCoordinateWdivide.t,0.0,1.0);

	float distanceFromLight = texture2D(ShadowMap,shadowCoordinateWdivide.st).z;
	
	float shadow = 1.0;

	if(shadowCoordinateWdivide.s<0.0 || shadowCoordinateWdivide.t<0.0
	||shadowCoordinateWdivide.s>1.0 || shadowCoordinateWdivide.t>1.0)
		shadow = 1.0;
	else if (ShadowCoord.w > 0.0)
		shadow = distanceFromLight < shadowCoordinateWdivide.z ? 0.5 : 1.0;


	float totLight = 100000.0/dot(lightVec,lightVec);
	totLight = clamp(totLight,0.0,1.0);

	//Normalize the incomming vectors
	vec3 lVec = normalize(lightVec);
	vec3 vVec = normalize(viewVec);
	vec3 dVec = normalize(lightDir);

	if(dot(vec3(0.0,0.0,1.0),lightVec)>0.0)// && dot(dVec,-lVec) > gl_LightSource[0].spotCosCutoff)
	{
		
		// Calculate offset, scale & biais
		float height = texture2D(normalMap, texCoord).w; // Heightmap -> the forth component in normalMap
		vec2 newTexCoord = texCoord + ((height * 0.03 - 0.015) * (vVec.xy));
		
		vec4 base = texture2D(diffuseMap, newTexCoord);
		vec3 bump = texture2D(normalMap, newTexCoord).xyz * 2.0 - 1.0;

		float diffuse = max( dot(lVec, bump), 0.0 );

		float specular = pow(clamp(dot(reflect(-vVec, bump), lVec), 0.0, 1.0), gl_FrontMaterial.shininess );
	
		vec4 vDiffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * diffuse;	
		vec4 vSpecular = gl_LightSource[0].specular * gl_FrontMaterial.specular *specular;	
		finalColor *= base;
		finalColor += shadow*totLight*(vDiffuse*base + vSpecular);
		//gl_FragColor = vAmbient*base + shadow*totLight*(vDiffuse*base + vSpecular);
	}
	else
	{
		vec4 base = texture2D(diffuseMap, texCoord);	
		finalColor *= base;
	}

	gl_FragColor = finalColor;
	
}