sampler2D TextureSampler : register(s0); // Declare the sampler
float4 ColorKeyLow : register(c0);       // Declare the color key low constant
float4 ColorKeyHigh : register(c1);      // Declare the color key high constant

float4 main(float4 position : POSITION0, float2 texcoord : TEXCOORD0, float4 color : COLOR0) : COLOR
{
	// Sample the texture
	float4 texColor = tex2D(TextureSampler, texcoord);

	// Check if the texel color is within the color key range
	if (all(texColor.rgb >= ColorKeyLow.rgb) && all(texColor.rgb <= ColorKeyHigh.rgb))
	{
	    // Discard pixels within the color key range
	    discard;
	}

	// Output the texture color
	return texColor;
}
