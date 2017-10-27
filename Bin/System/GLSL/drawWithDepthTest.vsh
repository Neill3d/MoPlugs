
// simple shader to draw model and cull by the main depth buffer

//varying vec4 pos;

void main(void)
{

	vec4 position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord [0] = gl_MultiTexCoord0;
	
	//pos = position;
	gl_Position = position;
}
