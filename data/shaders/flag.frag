uniform sampler2D texture;
uniform float wave_phase;

void main()
{
	vec2 offset = vec2(cos(wave_phase*40.0)*0.005, sin(wave_phase*40.0)*0.005);
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	gl_FragColor = gl_Color * pixel;
}
