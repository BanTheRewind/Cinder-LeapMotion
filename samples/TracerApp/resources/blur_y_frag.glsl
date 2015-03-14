#version 150 core

uniform float		offset;
uniform vec2		size;
uniform sampler2D	tex;

in vec2		uv;

out vec4	gl_FragColor;
 
void main( void )
{
	vec4 color		= vec4( 0.0 );
	vec4 src		= texture( tex, uv );
	color			= src * 0.16;
	color			+= texture( tex, vec2( uv.s, uv.t - 4.0 * size.y ) ) * 0.05;
	color			+= texture( tex, vec2( uv.s, uv.t - 3.0 * size.y ) ) * 0.09;
	color			+= texture( tex, vec2( uv.s, uv.t - 2.0 * size.y ) ) * 0.12;
	color			+= texture( tex, vec2( uv.s, uv.t - 1.0 * size.y ) ) * 0.15;
	color			+= texture( tex, vec2( uv.s, uv.t + 1.0 * size.y ) ) * 0.15;
	color			+= texture( tex, vec2( uv.s, uv.t + 2.0 * size.y ) ) * 0.12;
	color			+= texture( tex, vec2( uv.s, uv.t + 3.0 * size.y ) ) * 0.09;
	color			+= texture( tex, vec2( uv.s, uv.t + 4.0 * size.y ) ) * 0.05;
	gl_FragColor	= color;
}
