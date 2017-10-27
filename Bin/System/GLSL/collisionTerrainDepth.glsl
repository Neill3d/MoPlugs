
-- vertex

#version 120                                                                        
                                         
varying float Depth;

//uniform mat4 gVP;       
uniform float FarPlane;

void main()                                                                         
{                                                                                   
    vec4 pos = gl_ModelViewProjectionMatrix * gl_Vertex;  
	//Depth = pos.z / 100.0;
	Depth = 1.0 - ((pos.z / pos.w) + 1.0) * 0.5;
	gl_Position = pos;
} 


-- fragment

#version 120                                                                        

varying float Depth;                                                               

vec4 packFloatToVec4i(const float value)
{
  const vec4 bitSh = vec4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0);
  const vec4 bitMsk = vec4(0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0);
  vec4 res = fract(value * bitSh);
  res -= res.xxyz * bitMsk;
  return res;
}

void main()                                                                         
{         
	gl_FragColor = packFloatToVec4i(Depth);
}
