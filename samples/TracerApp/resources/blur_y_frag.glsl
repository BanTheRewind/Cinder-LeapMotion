uniform float		offset;
uniform vec2		size;
uniform sampler2D	tex;

varying vec2		uv;
 
void main( void )
{
	vec4 color		= vec4( 0.0 );
	vec4 src		= texture2D( tex, uv );
	color			= src * 0.16;
	color			+= texture2D( tex, vec2( uv.s, uv.t - 4.0 * size.y ) ) * 0.05;
	color			+= texture2D( tex, vec2( uv.s, uv.t - 3.0 * size.y ) ) * 0.09;
	color			+= texture2D( tex, vec2( uv.s, uv.t - 2.0 * size.y ) ) * 0.12;
	color			+= texture2D( tex, vec2( uv.s, uv.t - 1.0 * size.y ) ) * 0.15;
	color			+= texture2D( tex, vec2( uv.s, uv.t + 1.0 * size.y ) ) * 0.15;
	color			+= texture2D( tex, vec2( uv.s, uv.t + 2.0 * size.y ) ) * 0.12;
	color			+= texture2D( tex, vec2( uv.s, uv.t + 3.0 * size.y ) ) * 0.09;
	color			+= texture2D( tex, vec2( uv.s, uv.t + 4.0 * size.y ) ) * 0.05;
	gl_FragColor	= color;
}
