
cbuffer FrameContext
{
	float4x4 vp_mat;
	float4x4 world_mat;
    uint view_mode; // 0:tri 1:cluster 2:group
    uint level;
	uint display_ext_edge;
};

struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 coord  : TEX_COORD;	// is_ext
    float3 color  : TEX_COORD1;
	float is_ext : TEX_COORD2;
};

Buffer<uint> data;
Buffer<uint> ClusterOffsets;

uint MurmurMix(uint Hash){
	Hash^=Hash>>16;
	Hash*=0x85ebca6b;
	Hash^=Hash>>13;
	Hash*=0xc2b2ae35;
	Hash^=Hash>>16;
	return Hash;
}

float3 to_color(uint idx)
{
	uint Hash = MurmurMix(idx+1);

	float3 color = float3((Hash>>0)&255, (Hash>>8)&255, (Hash>>16)&255);

	return color*(1.0f/255.0f);
}

uint cycle3(uint i)
{
    uint imod3=i%3;
    return i-imod3+((1<<imod3)&3);
}

uint cycle3(uint i,uint ofs)
{
    return i-i%3+(i+ofs)%3;
}

struct Cluster{
	uint num_tri;
	uint idx_ofs;
	uint num_vert;
	uint pos_ofs;

	float4 bounds;
	uint group_id;
	uint mip_level;

	//de
	uint cluster_id;
	uint buffer_idx;
	uint flag_ofs;
};

Cluster get_cluster(uint cluster_id){
	Cluster cluster;
	uint idx = ClusterOffsets[cluster_id];  //cluster_buffer_offset();

	cluster.num_tri = data[idx+0];
	cluster.num_vert = data[idx+1];
	cluster.group_id = data[idx+2];
	cluster.mip_level = data[idx+3];

	cluster.bounds.x = asfloat(data[idx+4]);
	cluster.bounds.y = asfloat(data[idx+5]);
	cluster.bounds.z = asfloat(data[idx+6]);
	cluster.bounds.w = asfloat(data[idx+7]);

	cluster.idx_ofs=8;
	cluster.pos_ofs=8+cluster.num_tri;

	//de
	cluster.cluster_id=cluster_id;
	cluster.buffer_idx=cluster_id;
	cluster.flag_ofs=8+cluster.num_tri+cluster.num_vert*3;
	return cluster;
}

float3 get_position(Cluster cluster,uint index){
	uint tri_id=index/3;
	uint packed_tri = data[ClusterOffsets[cluster.buffer_idx] + tri_id+cluster.idx_ofs];
	uint v_idx=((packed_tri>>(index%3*8))&255);

	float3 p;
    p.x = asfloat(data[ClusterOffsets[cluster.buffer_idx] + v_idx*3+cluster.pos_ofs]);
    p.y = asfloat(data[ClusterOffsets[cluster.buffer_idx] + v_idx*3+1+cluster.pos_ofs]);
    p.z = asfloat(data[ClusterOffsets[cluster.buffer_idx] + v_idx*3+2+cluster.pos_ofs]);
	return p;
}

#define pi 3.1415926535

void VS(uint vid : SV_VertexID, uint instanceID : SV_InstanceID, out PSInput PSIn) 
{
	uint cluster_id = instanceID;
	uint index_id = vid;
	uint tri_id=index_id/3;

	Cluster cluster = get_cluster(instanceID);

    uint displayExtEdge = display_ext_edge;

	if (tri_id >= cluster.num_tri || cluster.mip_level != level){
		PSIn.Pos.z = -1; //discard
		return;
	}
	else{
		float3 p;
		if(view_mode==3){
			if(tri_id>=64){
				PSIn.Pos.z = -1;
				return;
			}
			uint t=tri_id/2;
			uint x=t>>2;
			uint y=t&3;
			uint i=index_id%6;
			if(i>=3) i-=2;
			x+=(i&1);
			y+=(i>>1);
			float3 tp;
			float theta = y * pi * 0.25;
			float phi = x * pi * 0.25;
			tp.x=sin(theta)*cos(phi);
			tp.y=sin(theta)*sin(phi);
			tp.z=cos(theta);

			p=cluster.bounds.xyz+tp*cluster.bounds.w;
			displayExtEdge = 0;
		}
		else{
			p=get_position(cluster,index_id);
		}

		if (view_mode==0) PSIn.color = to_color(tri_id);
		else if (view_mode==1) PSIn.color = to_color(cluster_id);
		else if (view_mode==2) PSIn.color = to_color(cluster.group_id);
		else if (view_mode==3) PSIn.color = to_color(cluster_id);

		if(displayExtEdge != 0){
			uint t = data[ClusterOffsets[cluster.buffer_idx] + index_id+cluster.flag_ofs];
			t |= data[ClusterOffsets[cluster.buffer_idx] + cycle3(index_id,2)+cluster.flag_ofs];
			PSIn.is_ext = t;

			float tc[3] = {0,0,0};
			tc[index_id%3] = 1;
			PSIn.coord = float3(tc[0], tc[1], tc[2]);
		}
		else{
			PSIn.is_ext = 0;
			PSIn.coord = float3(0,0,0);
		}

		float4 pos = mul(vp_mat, mul(world_mat, float4(p,1)));
		pos = pos/pos.w;
		if(pos.z<0 || pos.z>1)
            pos.z = -1;
		PSIn.Pos=pos;
	}
}

struct PSOutput
{
    float4 Color : SV_TARGET;
};

void PS(in PSInput PSIn, out PSOutput PSOut)
{
    float t = 1 - min(min(PSIn.coord.x, PSIn.coord.y), PSIn.coord.z);
    t = min(t, PSIn.is_ext);
    t = clamp(pow(t+0.1, 50)+0.2,0,1);

    float3 c = lerp(PSIn.color,float3(1,1,1), t);
    PSOut.Color = float4(c, 1);
}

