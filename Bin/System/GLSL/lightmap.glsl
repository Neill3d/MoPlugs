
-- vertex

#version 120                                                                       


void main()                                                                         
{                                                                           
    gl_Position     = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord [0] = gl_MultiTexCoord0;
} 

-- fragment

#version 120

uniform sampler2D 	gLightMap;                                                        


void main()                                                                         
{
	vec4 color = texture2D(gLightMap, gl_TexCoord[0].xy);
	//if (color.r < 0.01) discard;
	color = vec4(color.r, color.r, color.r, 1.0 - color.r);
	
	gl_FragColor = color;
}
