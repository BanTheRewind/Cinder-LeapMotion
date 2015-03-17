#version 330 core

uniform vec2		uSize;
uniform sampler2D	uTexture;

in vec2				vUv;

out vec4			oColor;

void main( void )
{
	oColor = texture( uTexture, vUv ) * 0.16;
	oColor += texture( uTexture, vec2( vUv.s - 4.0 * uSize.x, vUv.t ) ) * 0.05;
	oColor += texture( uTexture, vec2( vUv.s - 3.0 * uSize.x, vUv.t ) ) * 0.09;
	oColor += texture( uTexture, vec2( vUv.s - 2.0 * uSize.x, vUv.t ) ) * 0.12;
	oColor += texture( uTexture, vec2( vUv.s - 1.0 * uSize.x, vUv.t ) ) * 0.15;
	oColor += texture( uTexture, vec2( vUv.s + 1.0 * uSize.x, vUv.t ) ) * 0.15;
	oColor += texture( uTexture, vec2( vUv.s + 2.0 * uSize.x, vUv.t ) ) * 0.12;
	oColor += texture( uTexture, vec2( vUv.s + 3.0 * uSize.x, vUv.t ) ) * 0.09;
	oColor += texture( uTexture, vec2( vUv.s + 4.0 * uSize.x, vUv.t ) ) * 0.05;
}
