#version 330 core

in vec4		ciPosition;

out vec2	vUv;

void main( void )
{
	vUv			= sign( ciPosition.xy );
	gl_Position	= vec4( vUv, 0.0, 1.0 ) - vec4( 0.5 );
}
