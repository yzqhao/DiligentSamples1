
Buffer<float> data;

RWTexture3D<float> dstTexture : register(u0);

inline void Write3D(RWTexture3D<float> tex, int3 p, float val) 
{ 
	tex[p] = val;
}

[numthreads(16, 16, 1)]
void csmain(int3 DTid : SV_DispatchThreadID) 
{
	int idx = DTid.x * DTid.y * DTid.z;
	Write3D(dstTexture, DTid.xyz, data[idx]);
}
