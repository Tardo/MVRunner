uniform sampler2D texture;

void main()
{
	vec4 pixelColor = texture2D(texture, gl_TexCoord[0].xy).rgba; 
	pixelColor = floor(pixelColor / 0.3) * 0.5;
	gl_FragColor = pixelColor;
}