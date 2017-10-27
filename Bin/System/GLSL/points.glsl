
-- vertex

#version 330                                                                        

layout (location = 0) in vec4 Position;                                             
out float life;

uniform mat4 gVP;                                                                                                                         
uniform float gBillboardSize; 

uniform int			UseSizeCurve;
uniform sampler1D 	SizeCurve;

void main()                                                                         
{           
	float size = gBillboardSize;
	if (UseSizeCurve == 1)
	{
		size = texture( SizeCurve, Position.w ).r;
	}
                                                                        
    gl_Position = gVP * vec4(Position.xyz, 1.0);  
	gl_PointSize = size;
	life = Position.w;
} 


-- fragment

#version 330                                                                        

//uniform sampler2D gColorMap;                                                        
in float life;
out vec4 FragColor;                                                                 

uniform	vec4	gColor;

uniform int			UseColorCurve;
uniform sampler1D	ColorCurve;

void main()                                                                         
{       
	if (life >= 0.99) discard;
                                                                            
    vec4 color = gColor;
	
	if (UseColorCurve == 1)
	{
		color = texture(ColorCurve, life);
	}

	
	
	FragColor = color;
}
