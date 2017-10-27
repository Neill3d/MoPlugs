
// simple shader to draw model and cull by the main depth buffer

#version 120     

uniform vec4			SCREEN_SIZE;
uniform vec4			MATERIAL_COLOR;

uniform float			useColorSampler;

uniform sampler2D		colorSampler;
uniform sampler2D		depthSampler;

// z - do we need to compare with depth
uniform vec4			options; // x - near plane, y - far plane, w - bias factor to compare depth

//varying vec4 pos;

float GetFragmentZ()
{
	return gl_FragCoord.z;
}

void ComputeDepth(in float d, out float depth)
{
	float n = options.x;
	float f = options.y;
	
	float lz = 2.0 * d - 1.0; 
	depth = (2.0 * n) / (f + n - lz * (f - n));
}

void main()                                                                         
{
	if (options.z > 0.0)
	{
		vec2 coords = vec2((gl_FragCoord.x+SCREEN_SIZE.x) * SCREEN_SIZE.z, (gl_FragCoord.y+SCREEN_SIZE.y) * SCREEN_SIZE.w);
		float mainDepthValue = texture2D(depthSampler, coords).z;

		// convert depth values into linear space
		float mainLinearD, fragLinearD;

		ComputeDepth( mainDepthValue, mainLinearD );
		ComputeDepth( GetFragmentZ(), fragLinearD );
	
		float bias = options.w;
		if (mainLinearD+bias < fragLinearD)
			discard;
	}
	
	vec4 color = MATERIAL_COLOR;
	
	//if (useColorSampler > 0.0)
	//{
		vec2 tx  = gl_TexCoord [0].xy;
		color = texture2D(colorSampler, tx);
	//}
	
	gl_FragColor = color;
}
