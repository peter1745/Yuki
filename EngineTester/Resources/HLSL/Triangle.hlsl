struct VertexData
{
	float3 Position : POSITION;
	float3 Color : COLOR;
};

struct VertexOutput
{
	float4 Position : SV_Position;
	float3 Color : COLOR;
};

VertexOutput VSMain(VertexData vertexData)
{
	VertexOutput output;
	output.Position = float4(vertexData.Position, 1.0f);
	output.Color = vertexData.Color;
	return output;
}

struct PixelInput
{
	float3 Color : COLOR;
};

float4 PSMain(PixelInput input)
{
	return float4(input.Color, 1.0f);
}