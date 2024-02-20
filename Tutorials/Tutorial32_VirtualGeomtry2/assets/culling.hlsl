
struct Cluster{
    float4 lod_bounds;
    float4 parent_lod_bounds;
    float lod_error;
    float max_parent_lod_error;
};

cbuffer FrameContext
{
	float4x4 vp_mat;
	float4x4 w_mat;
	float4x4 v_mat;
};

Buffer<uint> data;
RWBuffer<uint> arg;
RWBuffer<uint> visibility;

Cluster get_cluster(uint cluster_id){
    Cluster cluster;
    
    uint ofs=4+16*cluster_id;

    cluster.lod_bounds.x = asfloat(data[4+ofs]);
	cluster.lod_bounds.y = asfloat(data[5+ofs]);
	cluster.lod_bounds.z = asfloat(data[6+ofs]);
	cluster.lod_bounds.w = asfloat(data[7+ofs]);

    cluster.parent_lod_bounds.x = asfloat(data[8+ofs]);
	cluster.parent_lod_bounds.y = asfloat(data[9+ofs]);
	cluster.parent_lod_bounds.z = asfloat(data[10+ofs]);
	cluster.parent_lod_bounds.w = asfloat(data[11+ofs]);

    cluster.lod_error = asfloat(data[12+ofs]);
    cluster.max_parent_lod_error = asfloat(data[13+ofs]);
    return cluster;
}

uint get_num_clusters(){
    return data[0];
}

void init_arg(){
    arg[0]=128*3;
    arg[1]=0;
    arg[2]=0;
    arg[3]=0;
}

void add_cluster(uint cluster_id){
    uint pos;
    InterlockedAdd(arg[1], 1, pos);
    visibility[pos] = cluster_id;
}

bool check_lod(float4x4 v_mat, float4 bounds, float error){
    float3 p= mul(v_mat, mul(w_mat, float4(bounds.xyz, 1))).xyz;
    float d = max(length(p)-bounds.w,0);
    float theta = radians(40)/1080.0f;
    return theta*d >= error;
}

[numthreads(32, 1, 1)]
void csmain(int3 DTid : SV_DispatchThreadID)
{
    uint cluster_id = DTid.x;
    uint num_clusters = get_num_clusters();

    if(DTid.x==0)
    {
        init_arg();
    }

    GroupMemoryBarrier();

    if(cluster_id<num_clusters){
        Cluster cluster=get_cluster(cluster_id);
        
        bool parent_chk=check_lod(v_mat,cluster.parent_lod_bounds,cluster.max_parent_lod_error);
        bool cluster_chk=check_lod(v_mat,cluster.lod_bounds,cluster.lod_error);
        if(!parent_chk&&cluster_chk){
            add_cluster(cluster_id);
        }
    }
}