
-- vertex

#version 330                                                                        

layout (location = 0) in vec3 Position; 
out vec2 uv;                                            

uniform mat4 gVP;       

uniform vec3 offset;
uniform vec3 scale;

void main()                                                                         
{                                                                                   
    gl_Position = gVP * vec4(Position.xyz, 1.0);  
	
	uv = (Position.xz - offset.xz) / scale.xz;
} 


-- fragment

#version 330                                                                        

uniform sampler2D gColorMap;                                                        

in vec2 uv;
out vec4 FragColor;                                                                 

float unpackFloatFromVec4i(const vec4 value)
{
  const vec4 bitSh = vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
  return(dot(value, bitSh));
}

void main()                                                                         
{               
	float Depth = unpackFloatFromVec4i( texture(gColorMap, uv) );
 
	FragColor = vec4(Depth, Depth, Depth, 1.0);
}
