varying vec2	uv;

void main( void )
{
	uv			= sign( gl_Vertex.xy );
	gl_Position	= vec4( uv, 0.0, 1.0 ) - 0.5;
}
