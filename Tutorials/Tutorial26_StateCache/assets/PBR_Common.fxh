#ifndef _PBR_COMMON_FXH_
#define _PBR_COMMON_FXH_

#ifndef PI
#   define  PI 3.141592653589793
#endif

// Lambertian diffuse
// see https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
float3 LambertianDiffuse(float3 DiffuseColor)
{
    return DiffuseColor / PI;
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel term from "An Inexpensive BRDF Model for Physically based Rendering" by Christophe Schlick
// https://en.wikipedia.org/wiki/Schlick%27s_approximation
//
//      Rf(Theta) = Rf(0) + (1 - Rf(0)) * (1 - cos(Theta))^5
//
//
//           '.       |       .'
//             '.     |Theta.' 
//               '.   |   .'
//                 '. | .'
//        ___________'.'___________
//                   '|
//                  ' |
//                 '  |
//                '   |
//               ' Phi|
//
// Note that precise relfectance term is given by the following expression:
//
//      Rf(Theta) = 0.5 * (sin^2(Theta - Phi) / sin^2(Theta + Phi) + tan^2(Theta - Phi) / tan^2(Theta + Phi))
//
#define SCHLICK_REFLECTION(VdotH, Reflectance0, Reflectance90) ((Reflectance0) + ((Reflectance90) - (Reflectance0)) * pow(clamp(1.0 - (VdotH), 0.0, 1.0), 5.0))
float SchlickReflection(float VdotH, float Reflectance0, float Reflectance90)
{
    return SCHLICK_REFLECTION(VdotH, Reflectance0, Reflectance90);
}
float3 SchlickReflection(float VdotH, float3 Reflectance0, float3 Reflectance90)
{
    return SCHLICK_REFLECTION(VdotH, Reflectance0, Reflectance90);
}

// Visibility = G(v,l,a) / (4 * (n,v) * (n,l))
// see https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/geometricshadowing(specularg)
float SmithGGXVisibilityCorrelated(float NdotL, float NdotV, float AlphaRoughness)
{
    float a2 = AlphaRoughness * AlphaRoughness;

    float GGXV = NdotL * sqrt(max(NdotV * NdotV * (1.0 - a2) + a2, 1e-7));
    float GGXL = NdotV * sqrt(max(NdotL * NdotL * (1.0 - a2) + a2, 1e-7));

    return 0.5 / (GGXV + GGXL);
}

// Smith GGX shadow-masking function G(v,l,a)
float SmithGGXShadowMasking(float NdotL, float NdotV, float AlphaRoughness)
{
    return 4.0 * NdotL * NdotV * SmithGGXVisibilityCorrelated(NdotL, NdotV, AlphaRoughness);
}

// Smith GGX masking function G1
// [1] "Sampling the GGX Distribution of Visible Normals" (2018) by Eric Heitz - eq. (2)
// https://jcgt.org/published/0007/04/01/
float SmithGGXMasking(float NdotV, float AlphaRoughness)
{
    float a2 = AlphaRoughness * AlphaRoughness;

    // In [1], eq. (2) is defined for the tangent-space view direction V:
    //
    //                                        1
    //      G1(V) = -----------------------------------------------------------
    //                                    {      (ax*V.x)^2 + (ay*V.y)^2)  }
    //               1 + 0.5 * ( -1 + sqrt{ 1 + -------------------------- } )
    //                                    {              V.z^2             }
    // 
    // Note that [1] uses notation N for the micronormal, but in our case N is the macronormal,
    // while micronormal is H (aka the halfway vector).
    // 
    // After multiplying both nominator and denominator by 2*V.z and given that in our
    // case ax = ay = a, we get:
    // 
    //                                2 * V.z                                        2 * V.z
    //      G1(V) = ------------------------------------------- =  ----------------------------------------
    //               V.z + sqrt{ V.z^2 + a2 * (V.x^2 + V.y^2) }     V.z + sqrt{ V.z^2 + a2 * (1 - V.z^2) }
    //
    // Since V.z = NdotV, we finally get:

    float Denom = NdotV + sqrt(a2 + (1.0 - a2) * NdotV * NdotV);
    return 2.0 * max(NdotV, 0.0) / max(Denom, 1e-6);
}


// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float NormalDistribution_GGX(float NdotH, float AlphaRoughness)
{
    // [1] "Sampling the GGX Distribution of Visible Normals" (2018) by Eric Heitz - eq. (1)
    // https://jcgt.org/published/0007/04/01/

    float a2 = AlphaRoughness * AlphaRoughness;
    float f = NdotH * NdotH * (a2 - 1.0)  + 1.0;
    return a2 / (PI * f * f);
}


// Samples a normal from Visible Normal Distribution as described in
// "Sampling the GGX Distribution of Visible Normals" (2018) by Eric Heitz
//
// Notes:
//      - View direction must be pointing away from the surface.
//      - Returned normal is in tangent space with Z up.
//      - Returned normal should be used to reflect the view direction and obtain
//        the sampling direction.
//
// https://jcgt.org/published/0007/04/01/
// https://github.com/TheRealMJP/DXRPathTracer/blob/master/DXRPathTracer/RayTrace.hlsl
float3 SmithGGXSampleVisibleNormal(float3 View, // View direction in tangent space
                                   float  ax,   // X roughness
                                   float  ay,   // Y roughness
                                   float  u1,   // Uniform random variable in [0, 1]
                                   float  u2    // Uniform random variable in [0, 1]
                                   )
{
    // Stretch the view vector so we are sampling as if roughness==1
    float3 V = normalize(float3(View.x * ax, View.y * ay, View.z));

    // Build an orthonormal basis with V, T, and B
    float3 T = (V.z < 0.999) ? normalize(cross(V, float3(0, 0, 1))) : float3(1, 0, 0);
    float3 B = cross(T, V);

    // Choose a point on a disk with each half of the disk weighted
    // proportionally to its projection onto direction V
    float a = 1.0 / (1.0 + V.z);
    float r = sqrt(u1);
    float phi = (u2 < a) ? (u2 / a) * PI : PI + (u2 - a) / (1.0 - a) * PI;
    float p1 = r * cos(phi);
    float p2 = r * sin(phi) * ((u2 < a) ? 1.0 : V.z);

    // Calculate the normal in this stretched tangent space
    float3 n = p1 * T + p2 * B + sqrt(max(0.0, 1.0 - p1 * p1 - p2 * p2)) * V;

    // Unstretch and normalize the normal
    return normalize(float3(ax * n.x, ay * n.y, max(0.0, n.z)));
}


// Returns the probability of sampling direction L for the view direction V and normal N
// using the visible normals distribution.
// [1] "Sampling the GGX Distribution of Visible Normals" (2018) by Eric Heitz
// https://jcgt.org/published/0007/04/01/
float SmithGGXSampleDirectionPDF(float3 V, float3 N, float3 L, float AlphaRoughness)
{
    // Micronormal is the halfway vector
    float3 H = normalize(V + L);

    float NdotH = dot(H, N);
    float NdotV = dot(N, V);
    float NdotL = dot(N, L);
    //float VdotH = dot(V, H);
    if (NdotH <= 0.0 || NdotV <= 0.0 || NdotL <= 0.0)
        return 0.0;

    // Note that [1] uses notation N for the micronormal, but in our case N is the macronormal,
    // while micronormal is H (aka the halfway vector).
    float NDF = NormalDistribution_GGX(NdotH, AlphaRoughness); // (1) - D(N)
    float G1  = SmithGGXMasking(NdotV, AlphaRoughness);        // (2) - G1(V)

    float VNDF = G1 /* * VdotH */ * NDF / NdotV; // (3) - Dv(N)
    return  VNDF / (4.0 /* * VdotH */); // (17) - VdotH cancels out
}

struct AngularInfo
{
    float NdotL;   // cos angle between normal and light direction
    float NdotV;   // cos angle between normal and view direction
    float NdotH;   // cos angle between normal and half vector
    float LdotH;   // cos angle between light direction and half vector
    float VdotH;   // cos angle between view direction and half vector
};

AngularInfo GetAngularInfo(float3 PointToLight, float3 Normal, float3 View)
{
    float3 n = normalize(Normal);       // Outward direction of surface point
    float3 v = normalize(View);         // Direction from surface point to camera
    float3 l = normalize(PointToLight); // Direction from surface point to light
    float3 h = normalize(l + v);        // Direction of the vector between l and v

    AngularInfo info;
    info.NdotL = clamp(dot(n, l), 0.0, 1.0);
    info.NdotV = clamp(dot(n, v), 0.0, 1.0);
    info.NdotH = clamp(dot(n, h), 0.0, 1.0);
    info.LdotH = clamp(dot(l, h), 0.0, 1.0);
    info.VdotH = clamp(dot(v, h), 0.0, 1.0);

    return info;
}

struct SurfaceReflectanceInfo
{
    float  PerceptualRoughness;
    float3 Reflectance0;
    float3 Reflectance90;
    float3 DiffuseColor;
};

void BRDF(in float3                 PointToLight, 
          in float3                 Normal,
          in float3                 View,
          in SurfaceReflectanceInfo SrfInfo,
          out float3                DiffuseContrib,
          out float3                SpecContrib,
          out float                 NdotL)
{
    AngularInfo angularInfo = GetAngularInfo(PointToLight, Normal, View);

    DiffuseContrib = float3(0.0, 0.0, 0.0);
    SpecContrib    = float3(0.0, 0.0, 0.0);
    NdotL          = angularInfo.NdotL;
    // If one of the dot products is larger than zero, no division by zero can happen. Avoids black borders.
    if (angularInfo.NdotL > 0.0 || angularInfo.NdotV > 0.0)
    {
        //           D(h,a) * G(v,l,a) * F(v,h,f0)
        // f(v,l) = -------------------------------- = D(h,a) * Vis(v,l,a) * F(v,h,f0)
        //               4 * (n,v) * (n,l)
        // where
        //
        // Vis(v,l,a) = G(v,l,a) / (4 * (n,v) * (n,l))

        // It is not a mistake that AlphaRoughness = PerceptualRoughness ^ 2 and that
        // SmithGGXVisibilityCorrelated and NormalDistribution_GGX then use a2 = AlphaRoughness ^ 2.
        // See eq. 3 in https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
        float AlphaRoughness = SrfInfo.PerceptualRoughness * SrfInfo.PerceptualRoughness;
        float  D   = NormalDistribution_GGX(angularInfo.NdotH, AlphaRoughness);
        float  Vis = SmithGGXVisibilityCorrelated(angularInfo.NdotL, angularInfo.NdotV, AlphaRoughness);
        float3 F   = SchlickReflection(angularInfo.VdotH, SrfInfo.Reflectance0, SrfInfo.Reflectance90);

        DiffuseContrib = (1.0 - F) * LambertianDiffuse(SrfInfo.DiffuseColor);
        SpecContrib    = F * Vis * D;
    }
}

#endif // _PBR_COMMON_FXH_