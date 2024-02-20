cbuffer FrameContext
{
	float4x4 vp_mat;
	float4x4 v_mat;
    uint view_mode; // 0:tri 1:cluster 2:group 3:level
};

struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 color  : TEX_COORD1;
};

Buffer<uint> data;
Buffer<uint> visibility;

struct Cluster{
    uint num_vert;
    uint v_data_ofs;
    uint num_tri;
    uint t_data_ofs;

    uint group_id;
    uint mip_level;
};


Cluster get_cluster(uint cluster_id){
    Cluster cluster;
	
    uint ofs=4+16*cluster_id;

    cluster.num_vert = data[0+ofs];
    cluster.v_data_ofs = data[1+ofs];
    cluster.num_tri = data[2+ofs];
    cluster.t_data_ofs = data[3+ofs];

    cluster.group_id = data[14+ofs];
    cluster.mip_level = data[15+ofs];
    return cluster;
}


uint get_visiable_cluster(uint id){
    return visibility[id];
}

float3 get_position(Cluster cluster,uint index){
    uint tri_id=index/3;
	uint packed_tri = data[tri_id+cluster.t_data_ofs];
	uint v_idx=((packed_tri>>(index%3*8))&255);

	float3 p;
    p.x = asfloat(data[v_idx*3+cluster.v_data_ofs]);
    p.y = asfloat(data[v_idx*3+1+cluster.v_data_ofs]);
    p.z = asfloat(data[v_idx*3+2+cluster.v_data_ofs]);
	return p;
}

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

	float3 color = float3(
		(Hash>>0)&255,
		(Hash>>8)&255,
		(Hash>>16)&255
	);

	return color*(1.0f/255.0f);
}

void VS(uint vid : SV_VertexID, uint instanceID : SV_InstanceID, out PSInput PSIn) 
{
	uint cluster_id = get_visiable_cluster(instanceID);
	uint index = vid;
	uint tri_id=index/3;

	Cluster cluster=get_cluster(cluster_id);

    if(tri_id>=cluster.num_tri){
        PSIn.Pos.z = 0;
        return;
    }

    if(view_mode==0) PSIn.color=to_color(tri_id);
	else if(view_mode==1) PSIn.color=to_color(cluster_id);
	else if(view_mode==2) PSIn.color=to_color(cluster.group_id);
	else if(view_mode==3) PSIn.color=to_color(cluster.mip_level);

    float3 p=get_position(cluster,index);

	PSIn.Pos= mul(vp_mat, mul(v_mat, float4(p,1)));
}

struct PSOutput
{
    float4 Color : SV_TARGET;
};

void PS(in PSInput PSIn, out PSOutput PSOut)
{
    float3 c = lerp(PSIn.color, float3(1,1,1), 0.2);
    PSOut.Color = float4(c,1);
}
