#version 150 core

in vec4 position;

out vec2	uv;

void main( void )
{
	uv			= sign( position.xy );
	gl_Position	= vec4( uv, 0.0, 1.0 ) - vec4( 0.5 );
}
