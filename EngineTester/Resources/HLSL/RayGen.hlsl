RWTexture2D<float4> Output : register(u0);

//RaytracingAccelerationStructure SceneBVH : register(t0);

[shader("raygeneration")]
void Main()
{
	uint2 launchIndex = DispatchRaysIndex().xy;
	Output[launchIndex] = float4(1.0f, 0.0f, 0.0f, 1.0f);
}
