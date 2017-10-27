
-- vertex

#version 330                                                                        

layout (location = 0) in vec4 Position;                                             
layout (location = 1) in vec4 Normal;

layout (location = 4) in vec3 ParticlePosition;

uniform mat4 gVP;  
uniform mat4 gMV;                                                                                                                       
uniform float gBillboardSize; 

uniform int			UseSizeCurve;
uniform sampler1D 	SizeCurve;

out vec3 lNormal;
out float life;

void main()                                                                         
{   
	life = Position.w;
	
	float size = gBillboardSize;
	if (UseSizeCurve == 1)
	{
		size = texture( SizeCurve, Position.w ).r;
	}
                     
	vec4 pos = Position * size + vec4(ParticlePosition, 1.0);
    gl_Position = gVP * vec4(pos.xyz, 1.0);  
	vec4 n = Normal;
	lNormal = n.xyz;
	
} 


-- fragment

#version 330                                                                        

//uniform sampler2D gColorMap;                                                        

uniform vec3	gCamPosition;

in vec3 lNormal;  
in float life;                                                               
out vec4 FragColor;                                                                 

uniform	vec4	gColor;

uniform int			UseColorCurve;
uniform sampler1D	ColorCurve;

void main()                                                                         
{
    //FragColor = texture2D(gColorMap, TexCoord);                                     
  
    //if (FragColor.r >= 0.9 && FragColor.g >= 0.9 && FragColor.b >= 0.9) {           
    //    discard;                                                                    
    //}  
	
	vec4 color = gColor;
	
	if (UseColorCurve == 1)
	{
		color = texture(ColorCurve, life);
	}
	
	vec3 n = normalize(lNormal);
	float diffuse = clamp(dot( normalize(lNormal), normalize(gCamPosition) ), 0.0, 1.0);
	
	FragColor = vec4(diffuse * color.xyz, color.w);
}
