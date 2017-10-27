
-- vertex

#version 330                                                                        

layout (location = 0) in vec4 Position;                                             
out float life0;

void main()                                                                         
{                                                                                   
    gl_Position = vec4(Position.xyz, 1.0);   
	life0 = Position.w;
} 

-- geometry

#version 330                                                                        

layout(points) in;                                                                  
layout(triangle_strip) out;                                                         
layout(max_vertices = 4) out;                                                       
in float life0[];

uniform mat4 gVP;                                                                   
uniform vec3 gCameraPos;                                                            
uniform float gBillboardSize;                                                       

uniform int			UseSizeCurve;
uniform sampler1D 	SizeCurve;

out vec2 TexCoord;  
out float life1;                                                                

void main()                                                                         
{           
	float size = gBillboardSize;
	if (UseSizeCurve == 1)
	{
		size = texture( SizeCurve, life0[0] ).r;
	}
    
    vec3 Pos = gl_in[0].gl_Position.xyz;                                            
    vec3 toCamera = normalize(gCameraPos - Pos);                                    
    vec3 up = vec3(0.0, 1.0, 0.0);                                                  
    vec3 right = cross(toCamera, up) * size;                              

    Pos -= right * 0.5;                                                                   
    gl_Position = gVP * vec4(Pos, 1.0);                                             
    TexCoord = vec2(0.0, 0.0);
	life1 = life0[0];
    EmitVertex();                                                                   

    Pos.y += size;                                                        
    gl_Position = gVP * vec4(Pos, 1.0);                                             
    TexCoord = vec2(0.0, 1.0);    
	life1 = life0[0];	
    EmitVertex();                                                                   

    Pos.y -= size;                                                        
    Pos += right;                                                                   
    gl_Position = gVP * vec4(Pos, 1.0);                                             
    TexCoord = vec2(1.0, 0.0);                                                      
	life1 = life0[0];
    EmitVertex();                                                                   

    Pos.y += size;                                                        
    gl_Position = gVP * vec4(Pos, 1.0);                                             
    TexCoord = vec2(1.0, 1.0);                                                      
	life1 = life0[0];
    EmitVertex();                                                                   

    EndPrimitive();                                                                 
} 

-- fragment

#version 330                                                                        

uniform int gUseColorMap;
uniform sampler2D gColorMap;                                                        
uniform float gAlpha;

uniform	vec4	gColor;

uniform int			UseColorCurve;
uniform sampler1D	ColorCurve;

in vec2 TexCoord;                                                                   
out vec4 FragColor;                                                                 

in float life1;

void main()                                                                         
{   
	if (life1 >= 0.99) discard;
  
	vec4 color = gColor;
	
	if (UseColorCurve == 1)
	{
		color = texture(ColorCurve, life1);
	}
	
	if (gUseColorMap == 1)
	{
		color = color * texture2D(gColorMap, TexCoord);
		color.a *= gAlpha;
	}
	
	FragColor = color;
}
