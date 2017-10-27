
#version 120                                                                       

uniform vec4 		CameraPos;

varying vec3		Normal;
varying vec3		View;
varying vec3		Color;

void main()                                                                         
{                      
	vec4 viewPos = gl_ModelViewMatrix * gl_Vertex;
    gl_Position     = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord [0] = gl_MultiTexCoord0;
	gl_TexCoord [1] = gl_MultiTexCoord1;
	gl_TexCoord [2] = gl_MultiTexCoord2;
	gl_TexCoord [3] = gl_MultiTexCoord3;
	
	Normal = normalize((gl_NormalMatrix * gl_Normal).xyz);
	//View = 	normalize(-viewPos.xyz);
	View = normalize(-viewPos.xyz+CameraPos.xyz);
	Color = gl_Color.rgb;
} 
