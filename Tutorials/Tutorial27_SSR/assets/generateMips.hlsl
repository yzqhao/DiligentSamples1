
cbuffer RootConstant : register(b0)
{
	uint MipSize;
};


RWTexture2D<float> Source : register(u0);
RWTexture2D<float> Destination : register(u1);

[numthreads(16, 16, 1)]
void csmain(int3 id : SV_DispatchThreadID)
{
	if (id.x < MipSize.x && id.y < MipSize.y)
	{
		float color = 1.0f;

		[unroll] 
        for (int x = 0; x < 2; ++x)
		{
			[unroll] 
            for (int y = 0; y < 2; ++y)
				color = min(color, Source[id.xy * 2 + uint2(x, y)].x);
		}

        Destination[id.xy] = color;
	}
}