varying vec3 lightVec;
varying vec3 eyeVec;
varying vec2 texCoord;

uniform sampler2D diff;
uniform sampler2D norm;
uniform sampler2D height;
uniform float invRadius;

void main (void)
{
	

   	// Setting the bias this way is presented in "Parallax Mapping with Offset Limiting:
   	// A Per Pixel Approximation of Uneven Surfaces" by Terry Walsh.  See Section 4.1
   	// for a detailed explanation.
	
	const float scale=0.05;
   	float bias = scale * 0.5;

	float distSqr = dot(lightVec, lightVec);
	//float att = clamp(1.0 - invRadius * sqrt(distSqr), 0.0, 1.0);

	vec3 lVec = lightVec * inversesqrt(distSqr);

	vec3 vVec = normalize(eyeVec);

   	//vec3 heightVector = texture2D( height, texCoord).xyz;
   	//float h = scale * length( heightVector ) - bias;
	
	float h = texture2D( height, texCoord).r;
	h = h * 0.05 - 0.025;

   	vec2 nextTexCoord = h * vVec.xy + texCoord;
	
	vec4 base = texture2D(diff, nextTexCoord);
	
	vec3 bump = normalize( texture2D(norm, nextTexCoord).xyz * 2.0 - 1.0);

	vec4 vAmbient = gl_LightSource[0].ambient * gl_FrontMaterial.ambient;

	float diffuse = max( dot(lVec, bump), 0.0 );
	
	vec4 vDiffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * 
					diffuse;	

	float specular = pow(clamp(dot(reflect(-lVec, bump), vVec), 0.0, 1.0), 
	                 gl_FrontMaterial.shininess );
	
	vec4 vSpecular = gl_LightSource[0].specular * gl_FrontMaterial.specular * 
					 specular;	
	
	gl_FragColor = ( vAmbient*base + 
					 vDiffuse*base + 
					 vSpecular);// * att;
}