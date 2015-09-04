uniform vec2		uSize;
uniform sampler2D	uSampler;

in vec2				vTexCoord0;

out vec4			oColor;

void main( void )
{
	oColor = vec4( 0.0 );
	oColor += texture( uSampler, vTexCoord0 - 4.0 * uSize ) * 0.05;
	oColor += texture( uSampler, vTexCoord0 - 3.0 * uSize ) * 0.09;
	oColor += texture( uSampler, vTexCoord0 - 2.0 * uSize ) * 0.12;
	oColor += texture( uSampler, vTexCoord0 - 1.0 * uSize ) * 0.15;
	oColor += texture( uSampler, vTexCoord0				  ) * 0.16;
	oColor += texture( uSampler, vTexCoord0 + 1.0 * uSize ) * 0.15;
	oColor += texture( uSampler, vTexCoord0 + 2.0 * uSize ) * 0.12;
	oColor += texture( uSampler, vTexCoord0 + 3.0 * uSize ) * 0.09;
	oColor += texture( uSampler, vTexCoord0 + 4.0 * uSize ) * 0.05;
}
