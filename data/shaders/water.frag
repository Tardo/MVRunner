uniform sampler2D texture_water;
uniform sampler2D texture;
uniform float wave_phase;

float randv(vec2 co){
  return fract(sin(dot(co.xy*wave_phase, vec2(12.9898,78.233))) * 43758.5453);
}

float acc = 0.0;
void main()
{
	acc += randv(gl_TexCoord[0].xy);
	vec4 pixel_water = gl_Color * texture2D(texture_water, gl_TexCoord[0].xy);

	if (pixel_water.rgb != vec3(0.0, 0.0, 0.0))
	{
		vec2 offset;
		offset.x += cos(gl_TexCoord[0].y*wave_phase * 3.8)*0.0009;
		offset.y += sin(gl_TexCoord[0].x*wave_phase * 6.3)*0.00016;
		vec4 pixel = texture2D(texture, gl_TexCoord[0].xy + offset * acc);

		pixel.b += 0.15;
		gl_FragColor = gl_Color * pixel;
	}
	else
	{
		gl_FragColor = gl_Color * texture2D(texture, gl_TexCoord[0].xy);
	}
}
