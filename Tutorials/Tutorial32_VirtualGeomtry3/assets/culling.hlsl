
struct Cluster{
    float4 bounds;
    float4 lod_bounds;
    float4 parent_lod_bounds;
    float lod_error;
    float max_parent_lod_error;
};

struct Group{
    float4 lod_bounds;
    uint num_clusters;
    uint cluster_data_ofs;
    float max_parent_lod_error;
};

cbuffer FrameContext
{
	float4x4 vp_mat;
	float4x4 w_mat;
	float4x4 v_mat;
	float4x4 p_mat;
};

Buffer<uint> data;
Buffer<uint> instance;
RWBuffer<uint> arg;
RWBuffer<uint> visibility;

Texture2D<float4> level_deps;
SamplerState level_deps_sampler;

uint get_num_clusters()
{
    return data[0];
}

uint get_num_group(){
    return data[1];
}

uint get_group_data_offset(){
    return data[2];
}


Cluster get_cluster(uint cluster_id){
    Cluster cluster;
    
    uint ofs=4+20*cluster_id;

    cluster.bounds.x = asfloat(data[4+ofs]);
	cluster.bounds.y = asfloat(data[5+ofs]);
	cluster.bounds.z = asfloat(data[6+ofs]);
	cluster.bounds.w = asfloat(data[7+ofs]);

    cluster.lod_bounds.x = asfloat(data[8+ofs]);
	cluster.lod_bounds.y = asfloat(data[9+ofs]);
	cluster.lod_bounds.z = asfloat(data[10+ofs]);
	cluster.lod_bounds.w = asfloat(data[11+ofs]);

    cluster.parent_lod_bounds.x = asfloat(data[12+ofs]);
	cluster.parent_lod_bounds.y = asfloat(data[13+ofs]);
	cluster.parent_lod_bounds.z = asfloat(data[14+ofs]);
	cluster.parent_lod_bounds.w = asfloat(data[15+ofs]);

    cluster.lod_error = asfloat(data[16+ofs]);
    cluster.max_parent_lod_error = asfloat(data[17+ofs]);
    return cluster;
}

Group get_group(uint group_id){
    Group group;
    
    uint ofs=get_group_data_offset()+8*group_id;

    group.num_clusters = data[ofs];
    group.cluster_data_ofs = data[1+ofs];
    group.max_parent_lod_error = asfloat(data[2+ofs]);

    group.lod_bounds.x = asfloat(data[4+ofs]);
    group.lod_bounds.y = asfloat(data[5+ofs]);
    group.lod_bounds.z = asfloat(data[6+ofs]);
    group.lod_bounds.w = asfloat(data[7+ofs]);
    return group;
}

uint get_cluster_id(Group group,uint c_idx){
    uint ofs=group.cluster_data_ofs;
    return data[c_idx+ofs];
}

void add_cluster(uint cluster_id,uint instance_id){
    uint pos;
    InterlockedAdd(arg[1], 1, pos);
    visibility[pos*2]=cluster_id;
    visibility[pos*2+1]=instance_id;
}

float3 get_instance_offset(uint instance_id){
    float3 p;
    p.x = asfloat(instance[instance_id*3]);
    p.y = asfloat(instance[instance_id*3+1]);
    p.z = asfloat(instance[instance_id*3+2]);
    return p;
}

bool check_lod(float4x4 v_mat, float3 c, float r, float error){
    float3 p = mul(v_mat, mul(w_mat, float4(c,1))).xyz;
    float d=max(length(p)-r,0);
    float theta=radians(40)/1080.0f;
    return theta*d>=error;
}

bool frustum_cull(float3 view_c,float r,float4x4 p_mat){
    float3 p1=normalize(float3(p_mat[0][0],0,1));
    float3 p2=normalize(float3(-p_mat[0][0],0,1));
    float3 p3=normalize(float3(0,p_mat[1][1],1));
    float3 p4=normalize(float3(0,-p_mat[1][1],1));
    bool visiable=true;
    visiable=visiable&&dot(p1,view_c)<r;
    visiable=visiable&&dot(p2,view_c)<r;
    visiable=visiable&&dot(p3,view_c)<r;
    visiable=visiable&&dot(p4,view_c)<r;
    return visiable;
}

float2 project_sphere(float a,float z,float r){
    float t=sqrt(a*a+z*z-r*r);
    float M=(z*r-a*t)/(a*r+z*t);
    float m=(z*r+a*t)/(a*r-z*t);
    return float2(m,M);
}
float4 sphere_to_clip_rect(float3 c,float r,float4x4 p_mat){
    float2 x_range=project_sphere(c.x,c.z,r)*p_mat[0][0];
    float2 y_range=project_sphere(c.y,c.z,r)*p_mat[1][1];
    return float4(x_range.x,y_range.y,x_range.y,y_range.x);
}

int4 to_screen_rect(float4 rect){
    rect=clamp(rect*0.5+0.5,0,1);
    return int4(floor(rect.x*1920),floor(rect.y*1080),ceil(rect.z*1920),ceil(rect.w*1080));
}

uint high_bit(uint x){
    uint res=0,t=16,y=0;
    y=-((x>>t)!=0?1:0),res+=y&t,x>>=y&t,t>>=1;
    y=-((x>>t)!=0?1:0),res+=y&t,x>>=y&t,t>>=1;
    y=-((x>>t)!=0?1:0),res+=y&t,x>>=y&t,t>>=1;
    y=-((x>>t)!=0?1:0),res+=y&t,x>>=y&t,t>>=1;
    y=(x>>t)!=0?1:0,res+=y;
    return res;
}

int4 mip0_to_mip1(int4 rect){
    int tx=(rect.x+1)*1024;
    int x=tx/1920;
    if(tx%1920==0) x--;
    int ty=(rect.y+1)*1024;
    int y=ty/1080;
    if(ty%1080==0) y--;

    int z=(rect.z-1)*1024/1920;
    if(z<x) z=x;
    int w=(rect.w-1)*1024/1080;
    if(w<y) w=y;
    return int4(x,y,z,w);
}

bool hzb_cull(float3 view_c,float r, float4x4 p_mat){
    float4 rect = sphere_to_clip_rect(view_c,r,p_mat);
    int4 screen_rect=to_screen_rect(rect);
    int4 hzb_rect=mip0_to_mip1(screen_rect);
    uint lod=high_bit(max(hzb_rect.z-hzb_rect.x,hzb_rect.w-hzb_rect.y));
    // uint lod=0;

    float2 uv=(hzb_rect.xy+0.5)/float2(1024,1024);
    float x = level_deps.SampleLevel(level_deps_sampler, uv, lod).x;
    float y = level_deps.SampleLevel(level_deps_sampler, uv, lod, int2(1,0)).x;
    float z = level_deps.SampleLevel(level_deps_sampler, uv, lod, int2(1,1)).x;
    float w = level_deps.SampleLevel(level_deps_sampler, uv, lod, int2(0,1)).x;

    float min_z=min(min(x,y),min(y,w));
    float nz=view_c.z+r;
    nz=-0.1/nz;

    return nz>min_z;
}

[numthreads(32, 1, 1)]
void csmain(int3 DTid : SV_DispatchThreadID)
{
    uint t_id = DTid.x;
    uint group_id = t_id/900;
    uint instance_id = t_id%900;
    uint num_group = get_num_group();

    if (group_id < num_group){
        Group group = get_group(group_id);
        float3 ins_ofs=get_instance_offset(instance_id);

        bool parent_chk=check_lod(v_mat,group.lod_bounds.xyz+ins_ofs,group.lod_bounds.w,group.max_parent_lod_error);
        if(!parent_chk){ //父节点不通过子节点才可能通过
            for(uint i=0;i<group.num_clusters;i++){
                uint cluster_id = get_cluster_id(group,i);
                Cluster cluster = get_cluster(cluster_id);

                bool visiable = true;
                bool cluster_chk = check_lod(v_mat, cluster.lod_bounds.xyz+ins_ofs,cluster.lod_bounds.w,cluster.lod_error);

                if(!cluster_chk) visiable=false;
                if(visiable){
                    float3 c=cluster.bounds.xyz+ins_ofs;
                    float r=cluster.bounds.w;
                    float3 view_c = mul(v_mat, mul(w_mat, float4(c,1))).xyz;

                    if(view_c.z-r>-0.1) visiable=false; //近平面
                    else{
                        visiable=frustum_cull(view_c, r, p_mat);
                        if(visiable&&view_c.z+r<-0.1){ //包围球与近平面不相交，如相交直接认为可见
                            visiable=hzb_cull(view_c, r, p_mat);
                        }
                    }
                }
                if(visiable) 
                    add_cluster(cluster_id,instance_id);
            }
        }
    }
}