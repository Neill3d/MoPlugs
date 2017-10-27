//
// Fragment shader for 3*3 Gaussian blur
//

uniform sampler2D tex;

void main (void)
{
	const vec2	d1 = vec2 ( -1.0/128.0, -1.0/128.0 );
	const vec2	d2 = vec2 (  0.0,        1.0/128.0 );
	const vec2	d3 = vec2 (  1.0/128.0,  1.0/128.0 );
	const vec2	d4 = vec2 ( -1.0/128.0,  0.0 );
	const vec2	d5 = vec2 (  0.0,        0.0 );
	const vec2	d6 = vec2 (  1.0/128.0,  0.0 );
	const vec2	d7 = vec2 ( -1.0/128.0, -1.0/128.0 );
	const vec2	d8 = vec2 (  0.0,       -1.0/128.0 );
	const vec2	d9 = vec2 (  1.0/128.0, -1.0/128.0 );

	vec2	p = gl_TexCoord [0].st;
	
	gl_FragData [0] =  0.0625 * texture2D ( tex, vec2 ( p + d1 ) ) + 
                       0.125  * texture2D ( tex, vec2 ( p + d2 ) ) + 
					   0.0625 * texture2D ( tex, vec2 ( p + d3 ) ) + 
                       0.125  * texture2D ( tex, vec2 ( p + d4 ) ) + 
					   0.25   * texture2D ( tex, vec2 ( p + d5 ) ) + 
                       0.125  * texture2D ( tex, vec2 ( p + d6 ) ) + 
					   0.0625 * texture2D ( tex, vec2 ( p + d7 ) ) + 
			           0.125  * texture2D ( tex, vec2 ( p + d8 ) ) +
			           0.0625 * texture2D ( tex, vec2 ( p + d9 ) );
}
