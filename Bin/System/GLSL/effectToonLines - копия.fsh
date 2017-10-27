uniform	sampler2D	colorTex;
uniform sampler2D	depthTex;

uniform float 		zNear;	// camera near plane
uniform float 		zFar;	// camera far plane

uniform float		edgeForce;
uniform float		baseThresh;

uniform vec2		textureCoordOffset[9];

uniform float		logarithmicDepth;

float LogarithmicDepth(float d)
{
	//float d = texture2D(depthTex, gl_TexCoord[0].st).x;
	float C = 1.0;
	float z = (exp(d*log(C*zFar+1.0)) - 1.0)/C;
	return z;
}

float LinearizeDepth(float z)
{
  float n = zNear; // camera z near
  float f = zFar; // camera z far
  //float z = texture2D(depthTex, gl_TexCoord[0].st).x;
  //return (2.0 * n) / (f + n - z * (f - n));	
  
  //float z_b = texture2D(depthBuffTex, vTexCoord).x;
  float z_b = z;
  float z_n = 2.0 * z_b - 1.0;
  float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
  return z_e;
}


// The fragment shader loop
void main(void)
{
	// Assign these variables now because they will be used next
	float sample[9];
	vec4 texcol = texture2D(colorTex, gl_TexCoord[0].st);

	// Current fragment depth
	float base = 0.0;
	float colDifForce = 0.0;
	
	if (logarithmicDepth > 0.0)
	{
		base = LogarithmicDepth(texture2D(depthTex, gl_TexCoord[0].st).x);
		
		// Gets all neighboring fragments depths stored into the depth texture
		for (int i = 0; i < 9; i++)
		{
			sample[i] = LogarithmicDepth( texture2D(depthTex, gl_TexCoord[0].st + textureCoordOffset[i]).x );
			colDifForce += base - sample[i];
		}
	}
	else
	{
		base = LinearizeDepth(texture2D(depthTex, gl_TexCoord[0].st).x);
		
		// Gets all neighboring fragments depths stored into the depth texture
		for (int i = 0; i < 9; i++)
		{
			sample[i] = LinearizeDepth( texture2D(depthTex, gl_TexCoord[0].st + textureCoordOffset[i]).x );
			colDifForce += base - sample[i];
		}
	}

	// The result fragment sample matrix is as below, where x is the current fragment(4)
	// 0 1 2
	// 3 x 5
	// 6 7 8


	// From all the neighbor fragments gets the one with the greatest and lowest depths and place them
	// into two variables so a subtract can be made later. The check is huge, but GLSL built-in functions
	// are optimized for the GPU
	//float areaMx = max(sample[0], max(sample[1], max(sample[2], max(sample[3], max(sample[5], max(sample[6], max(sample[7], sample [8])))))));

	//float areaMn = min(sample[0], min(sample[1], min(sample[2], min(sample[3], min(sample[5], min(sample[6], min(sample[7], sample [8])))))));


	//colDifForce = areaMx - areaMn; // Gets the average value between the maximum and minimum depths


	//Check for heavy depth difference to darken the current fragment; 
	//we do not want to mess with transparency, so leave alpha alone
	//edgeForce variable control the outline transparency, so 1.0 would be full black.
	// ? : is the same as if else
	// abs is short of absolute value, it tells to disconsider the negativity of a value if it exists
	if (abs(colDifForce) > ( sample[4] * baseThresh ) / zNear)
	{
		//gl_FragColor = vec4(vec3(texcol*edgeForce), 1.0);
		gl_FragColor = (base * vec4(texcol)) + (1.0 - base) * vec4(vec3(texcol*edgeForce), 1.0);
	}
	else
	{	
		gl_FragColor = vec4(texcol);
	}
}
