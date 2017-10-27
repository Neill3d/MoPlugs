
uniform	sampler2D		sampler;
uniform vec2			texelSize;

void main (void)
{
	vec2 tx  = gl_TexCoord [0].xy;
	vec4 color = texture2D(sampler, tx);
	
	gl_FragColor = color;
}
