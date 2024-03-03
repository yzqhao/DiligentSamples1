// 个人Interl GPU比较差，分开传输
Buffer<float> data1;
Buffer<float> data2;

cbuffer RootConstant
{
	uint dimension;
}

RWTexture3D<float4> dstTexture : register(u0);

inline void Write3D(RWTexture3D<float4> tex, int3 p, float4 val) 
{ 
	tex[p] = val;
}

[numthreads(16, 16, 1)]
void csmain(int3 DTid : SV_DispatchThreadID) 
{
	int idx = DTid.x * DTid.y * DTid.z;
	float4 col = DTid.z < dimension/2 ? float4(data1[idx*4], data1[idx*4+1], data1[idx*4+2], data1[idx*4+3])
		: float4(data2[idx*4], data2[idx*4+1], data2[idx*4+2], data2[idx*4+3]);
	Write3D(dstTexture, DTid.xyz, col);
}
