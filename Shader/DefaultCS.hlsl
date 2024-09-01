RWTexture2D<float4> gOutput : register(u0);

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, uint3 tID : SV_DispatchThreadID)
{
	gOutput[tID.xy] = float4(0.5, 0.5, 0.5, 1.0);
}