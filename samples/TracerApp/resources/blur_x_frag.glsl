#version 150 core

uniform vec2		size;
uniform sampler2D	tex;

in vec2		uv;

out vec4	gl_FragColor;

void main( void )
{
	vec4 color		= vec4( 0.0 );
	vec4 src		= texture( tex, uv );
	color			= src * 0.16;
	color			+= texture( tex, vec2( uv.s - 4.0 * size.x, uv.t ) ) * 0.05;
	color			+= texture( tex, vec2( uv.s - 3.0 * size.x, uv.t ) ) * 0.09;
	color			+= texture( tex, vec2( uv.s - 2.0 * size.x, uv.t ) ) * 0.12;
	color			+= texture( tex, vec2( uv.s - 1.0 * size.x, uv.t ) ) * 0.15;
	color			+= texture( tex, vec2( uv.s + 1.0 * size.x, uv.t ) ) * 0.15;
	color			+= texture( tex, vec2( uv.s + 2.0 * size.x, uv.t ) ) * 0.12;
	color			+= texture( tex, vec2( uv.s + 3.0 * size.x, uv.t ) ) * 0.09;
	color			+= texture( tex, vec2( uv.s + 4.0 * size.x, uv.t ) ) * 0.05;
	gl_FragColor	= color;
}
