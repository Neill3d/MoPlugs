
#version 120     

uniform samplerCube 	cubeMapSampler;                                                        

void main()                                                                         
{
	vec2 sc = vec2(gl_TexCoord[0].st) * vec2(3.14159265, 2*3.14159265);
	//sc.s -= 0.5 * 3.14159265;
	//sc.t += 0.5 * 3.14159265;
	
	float pi = 3.14159265;
	float theta = sc.x * pi;
	float phi = sc.y * pi * 0.5;
	
	// convert to cartesian unit vector
	vec3 coords;
	
	//coords.y = sin(sc.s);
	//coords.x = cos(sc.s) * cos(sc.t);
	//coords.z = cos(sc.s) * sin(sc.t);
	
	coords.x = cos(phi) * cos(theta);
	coords.y = sin(phi);
	coords.z = cos(phi) * sin(theta);
	
	vec4 color = textureCube(cubeMapSampler, coords);
	gl_FragColor = color;
}
