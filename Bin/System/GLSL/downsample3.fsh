
uniform	sampler2D		sampler;
uniform vec2			texelSize;

void main (void)
{
	vec2 tx  = gl_TexCoord [0].xy;
	
	vec4 color, color2;
	vec4 tap0 = texture2D(sampler, tx);
	vec4 tap1 = texture2D(sampler, tx + texelSize * vec2( 0.4,  0.9));
	vec4 tap2 = texture2D(sampler, tx + texelSize * vec2(-0.4, -0.9));
	vec4 tap3 = texture2D(sampler, tx + texelSize * vec2(-0.9,  0.4));
	vec4 tap4 = texture2D(sampler, tx + texelSize * vec2( 0.9, -0.4));
	
	color = 0.2 * (tap0 + tap1 + tap2 + tap3 + tap4);
	
	vec4 tap11 = texture2D(sampler, tx + texelSize * vec2( 0.9,  1.9));
	vec4 tap21 = texture2D(sampler, tx + texelSize * vec2(-0.9, -1.9));
	vec4 tap31 = texture2D(sampler, tx + texelSize * vec2(-1.9,  0.9));
	vec4 tap41 = texture2D(sampler, tx + texelSize * vec2( 1.9, -0.9));
	
	color2 = 0.2 * (tap0 + tap11 + tap21 + tap31 + tap41);
	
	float mask = clamp(color2.w, 0.0, 1.0);
	
	gl_FragColor.rgb = color.rgb * mask + color2.rgb * (1.0-mask);
	gl_FragColor.w = mask;
}
