varying vec2 texCoord;

uniform sampler2D colorMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

uniform vec2 screenSize;
uniform vec2 cameraRange;

uniform int numLights;
uniform vec3 lightPosition[64];
uniform vec3 lightColor[64];
uniform vec3 lightDirection[64];
uniform float invLightRadiusSqr[64];

float linearizeDepth(float depth) {
	return (2.0 * cameraRange.x) / (cameraRange.y + cameraRange.x - depth * (cameraRange.y - cameraRange.x));
}

// --- CRED TO NVIDIA --- //
// Beräknar FragCoord från depth till ViewSpace //

const float fovy           = 65.0 * 3.14159265 / 180.0; // 65 deg in radian
float invFocalLenY   = tan(fovy * 0.5);
float invFocalLenX   = tan(fovy * 0.5) * screenSize.x / screenSize.y;

vec3 uv_to_eye(vec2 uv, float eye_z)
{
   uv = (uv * vec2(2.0, -2.0) - vec2(1.0, -1.0));
   return vec3(uv * vec2(invFocalLenX, -invFocalLenY) * eye_z, -eye_z);
}

vec3 fetch_eye_pos(vec2 uv)
{
   float z = texture2D(depthMap, uv).x*(cameraRange.y-cameraRange.x);
   return uv_to_eye(uv, z);
}

// --------- //
	
void main (void)
{
	vec3 viewCoord = fetch_eye_pos(texCoord);
	vec3 viewVec = -viewCoord;
	vec3 vVec = normalize(viewVec);
	vec3 lightVec;	
	vec3 finalColor;

	float distSqr;
	float invRadiusSqr;
	float att;

	vec3 lVec;

	float diffuse;
	vec3 vDiffuse;

	float specular;

	vec4 bump = texture2D(normalMap,texCoord);
	bump.xyz = bump.xyz * 2.0 - 1.0;
	float specFactor = bump.w;

	vec4 base = texture2D(colorMap,texCoord);
	float ambient = base.w;

	for(int i=0; i<numLights; i++)
	{
		lightVec = lightPosition[i] + viewVec;
		distSqr = dot(lightVec, lightVec);
		att = clamp(1.0 - invLightRadiusSqr[i] * distSqr, 0.0, 1.0);

		lVec = normalize(lightVec);

		diffuse = max( dot(lVec,bump.xyz), 0.0 );
		vDiffuse = lightColor[i] * diffuse;

		specular = clamp(dot(reflect(-vVec, bump.xyz), lVec), 0.0, 1.0);
		specular = pow(specular, 10 );
		specular *= specFactor;

		finalColor += vDiffuse*att*base.xyz + att*diffuse*specular;
	}

	const float LOG2 = 1.442695;
	float z = -viewCoord.z / cameraRange.y;
	float density = 2;
	float fogFactor = exp2( - density * density * z * z * LOG2 );
	fogFactor = clamp(fogFactor, 0.0, 1.0);


	finalColor += ambient*base.xyz;

	gl_FragColor = vec4(mix(gl_Fog.color, finalColor, fogFactor ),1.0);
}
