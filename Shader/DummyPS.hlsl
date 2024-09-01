#include "Common.hlsli"

Texture2D renderTex : register(t0);

struct PixelShaderInput
{
	float4 posModel : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

float3 TextureToView(float2 texCoord)
{
	float4 p = 0.0;

	p.x = texCoord.x * 2.0 - 1.0;
	p.y = -texCoord.y * 2.0 + 1.0;
	p.z = shadowMap0.Sample(pointClampSS, texCoord).r;
	p.w = 1.0;

	p = mul(p, projInv);
	p /= p.w;

	return p.xyz;
}

int RaySphereIntersection(in float3 start, in float3 dir, in float3 center, in float radius,
	out float t1, out float t2)
{
	float3 p = start - center;
	float pdotv = dot(p, dir);
	float p2 = dot(p, p);
	float r2 = radius * radius;
	float m = pdotv * pdotv - (p2 - r2);

	if (m < 0.0)
	{
		t1 = 0;
		t2 = 0;
		return 0;
	}
	else
	{
		m = sqrt(m);
		t1 = -pdotv - m;
		t2 = -pdotv + m;
		return 1;
	}
}

float HaloEmission(float3 posView, float radius)
{
	// Halo
	float3 rayStart = float3(0, 0, 0); // View space
	float3 dir = normalize(posView - rayStart);

	float3 center = mul(float4(haloPosition, 1.0), view).xyz; // View 공간으로 변환
	


	float t1 = 0.0;
	float t2 = 0.0;
	if (RaySphereIntersection(rayStart, dir, center, radius, t1, t2) && t1 < posView.z)
	{
		t2 = min(posView.z, t2);

		float p2 = dot(rayStart - center, rayStart - center);
		float pdotv = dot(rayStart - center, dir);
		float r2 = radius * radius;
		float invr2 = 1.0 / r2;
		float haloEmission = (1 - p2 * invr2) * (t2 - t1)
			- pdotv * invr2 * (t2 * t2 - t1 * t1)
			- 1.0 / (3.0 * r2) * (t2 * t2 * t2 - t1 * t1 * t1);

		haloEmission /= (4 * radius / 3.0);

		return haloEmission;
	}
	else
	{
		return 0.0;
	}
}

float4 main(PixelShaderInput input) : SV_TARGET
{
	float3 color = clamp(renderTex.Sample(linearClampSS, input.texCoord).rgb, 0, 1);

	float3 posView = TextureToView(input.texCoord);

	// Halo
	float3 haloColor = float3(0.96, 0.94, 0.82);
	float radius = haloRadius;
	color += HaloEmission(posView.xyz, radius) * haloColor * haloStrength;

	// Fog
	float dist = length(posView.xyz); // 눈의 위치가 원점인 좌표계
	float3 fogColor = float3(1, 1, 1);
	float fogMin = 1.0;
	float fogMax = 10.0;
	float distFog = saturate((dist - fogMin) / (fogMax - fogMin));
	float fogFactor = exp(-distFog * fogStrength);

	color = lerp(fogColor, color, fogFactor);

	return float4(color, 1.0);
}