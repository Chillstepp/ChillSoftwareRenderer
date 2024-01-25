# Chill Software Renderer

![Snipaste_2024-01-15_14-23-30](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/Snipaste_2024-01-15_14-23-30.png)

## Tech

- [x] Back-Face Culling
- [x] Perspective-Correct Interpolation
- [x] Camera
- [x] Shading:
    - [x] Phong
    - [x] Blinn–Phong
    - [x] Gouraud
    - [x] Flat
- [x] Tangent Space Normal Mapping
- [ ] Shadow
    - [x] PCF
    - [x] PCSS
    - [ ] CSM
    - [ ] SDF Soft Shadows
    - [ ] VSM/VSSM
    - [ ] ESM
- [x] AO
    - [x] SSAO
    - [x] HBAO
- [ ] Anti-Aliasing
    - [ ] MSAA
    - [ ] FXAA
    - [ ] TAA
- [ ] SSR
- [ ] PBR
- [ ] IBL
- [ ] Cubemap

## ShowCase

<img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20240125220750615.png" alt="image-20240125220750615" style="zoom: 67%;" />

## Tech Roadmap

### Camera

- 规定$CenterLocation - CameraLocation$ 作为向量规定为**摄像机坐标系下z轴**的**正方向**，即**离摄像机越远的z值越大。**
- **右手坐标系**
- ![image-20231121144853967](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231121144853967.png)

#### Perspective-Correct Interpolation

- 图形学 - 关于透视矫正插值那些事 - Shawoxo的文章 - 知乎
  https://zhuanlan.zhihu.com/p/403259571
- 计算机图形学六：透视矫正插值和图形渲染管线总结 - 孙小磊的文章 - 知乎
  https://zhuanlan.zhihu.com/p/144331875

#### Culling

- 视锥剔除
- 视口剔除
- 背面剔除(BackFaceCulling)

BackFaceCulling

```c++
for(int i=0;i<model->nfaces();i++)
{
    const std::vector<int>& face = model->getface(i);
    Vec3f WorldCoords[3];
    for(int j=0;j<3;j++)
    {
        WorldCoords[j] = model->getvert(i, j);
    }
    Vec3f ScreenCoords[3];
    for(int j=0;j<3;j++)
    {
        auto Mat4x1_Vertex = Shader->vertex(i, j);
        ScreenCoords[j] = {Mat4x1_Vertex.raw[0][0], Mat4x1_Vertex.raw[1][0], Mat4x1_Vertex.raw[2][0]};
    }
    Vec3f tri_normal = (WorldCoords[2]-WorldCoords[0])^(WorldCoords[1]-WorldCoords[0]);
    if(tri_normal.normlize() * (WorldCoords[0] - Eye) >= 0) //back face culling
    {
        triangle(model,ScreenCoords, image2,ZBuffer,Shader);
    }
}
```

#### Clipping

- 如何写一个软渲染(4)-Clipping - 拳四郎的文章 - 知乎 https://zhuanlan.zhihu.com/p/43586784

### Tangent Space Normal Mapping

存储在模型顶点的**切线空间(tangent space)**中，优势：

1. 切线空间存储的是相对法线信息，因此换个网格（或者网格变换deforming）应用该纹理，也能得到合理的结果。
2. 可以进行uv动画，通过移动该纹理的uv坐标实现凹凸移动的效果，这种UV动画在水或者火山熔岩这种类型的物体会会用到。
3. 重用法线纹理节省内存，比如,一个砖块,我们仅使用一张法线纹理就可以用到所有的6个面，无需对每个面有一个发现贴图，这样大大减少了法线贴图的大小。
4. 可以压缩。因为切线空间的法线z方向总是正方向，因此可以仅存储xy方向，从而推导z方向（存储的法线向量是单位向量，用勾股定理由xy得出z，取z为正的一个即可）。

项目wiki上的理解/公式推导: [Summary of TagentSpaceNormal, TBN Matrix, TBN Coordinate](https://github.com/Chillstepp/ChillSoftwareRenderer/wiki/Summary-of-TagentSpaceNormal,-TBN-Matrix,--TBN-Coordinate)

### HDR/AECS ToneMapping

- HDR: https://learnopengl-cn.github.io/05%20Advanced%20Lighting/06%20HDR/#_1

- [Tone mapping进化论](https://zhuanlan.zhihu.com/p/21983679 )
- [ACES Filmic Tone Mapping Curve](https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/)

A very elegant fitting.

```c++
float3 ACESToneMapping(float3 color, float adapted_lum)
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	color *= adapted_lum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}
```

| With ACES ToneMapping                                        | Without ACES ToneMapping                                     |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231128150403640.png" alt="image-20231128150403640" style="zoom:67%;" /> | <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231128150312255.png" alt="image-20231128150312255" style="zoom:67%;" /> |

### Shadow Mapping

#### The Most Basic Method Of Shadow Mapping

- Z-fighting https://en.wikipedia.org/wiki/Z-fighting

- DirectX -
  改进阴影深度映射的常见技术 https://learn.microsoft.com/zh-cn/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps?redirectedfrom=MSDN

Here we use DepthShader to get Depth Buffer

| Depth Buffer（In Light View）                                |
| ------------------------------------------------------------ |
| <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231129161027212.png" alt="image-20231129161027212" style="zoom: 25%;" /> |

 For a point, We transform it form FrameBuffer screen coordinates to DepthBuffer screen coordinates.If a point's depth
is deeper than DepthBuffer, then this point should be a shadow point. We let the `color*ShadowFactor`, and ShadowFactor
is between 0 to 1 to make this point's color dark.

| With Shadow                                                  | Without Shadow                                               |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231129173100355.png" alt="image-20231129173100355" style="zoom:67%;" /> | <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231128150403640.png" alt="image-20231128150403640" style="zoom:67%;" /> |

思路不错但会出现几个问题：

- **阴影失真(Shadow Acne)**
- **阴影悬浮(Shadow Peter-panning)**
- **阴影锯齿(Shadow Aliasing)**

- 大场景中Shadow Map找不到对应的点

##### Shadow Acne

![image-20231224202229441](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231224202229441.png)

上图出现了条状的阴影:

![image-20231224202314452](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231224202314452.png)

 以方向光源为例，一般认为方向光是平行光，在光源处渲染时使用正交投影。因为Shadow Map的分辨率有限，Shadow Map上面的**一个片段**对应场景中的**一块区域**
，又因为很多情况下光源与物体存在夹角，因此记录的深度通常与物体的实际深度存在偏差。

 上图中蓝色片段即为Shadow
Map中记录的深度。在纹理中像素为最小单位，一个像素只能记录一个值，图中每个像素记录的是箭头处的深度。这就导致了明明本该整块被照亮的地板，会出现明暗相间的条纹：黑线处的地板由于在光源视角中深度小于记录的值，因此不在阴影中。红线处的地板深度大于记录的值，没有通过阴影测试。

 解决办法有些trick，我们可以用一个叫做**阴影偏移**（shadow bias）的技巧来解决这个问题，我们简单的对表面的深度（或深度贴图）应用一个偏移量，这样片元就不会被错误地认为在表面之下了。

![image-20231224202630631](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231224202630631.png)

```c++
float shadowFactor = 1.0f;
if((int)CorrespondingPoint.x < DepthBuffer.size() && (int)CorrespondingPoint.y < DepthBuffer[(int)CorrespondingPoint.x].size())
{
    shadowFactor = 0.3f + 0.7f*(DepthBuffer[(int)CorrespondingPoint.x][(int)CorrespondingPoint.y] > CorrespondingPoint.z - 1);//1 is shadow bias
}
```

我们这里简单的用了1作为bias， 阴影基本正常：

![image-20231224202923283](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231224202923283.png)

因为阴影失真的问题是由于光线和物体表面的夹角导致的，光线越垂直于物体，失真的影响就越小，因此通常会将bias与光线和物体表面法线的夹角挂钩：

$\text { bias }=k \cdot(1.0-\operatorname{dot}(\text { normal, lightDir }))$

```c++
    //Shadow
Vec3f p(0, 0, 0);
for (int i=0;i<3;i++)//坐标插值
{
p = p + Varying_tri[i] * bar.raw[i];
}
Matrix<4, 1, float>CorrespondingPointInShadowBuffer = Uniform_MShadow * Matrix<4, 1, float>::Embed(p);
CorrespondingPointInShadowBuffer /= CorrespondingPointInShadowBuffer.raw[3][0];
Vec3f CorrespondingPoint{ CorrespondingPointInShadowBuffer.raw[0][0], CorrespondingPointInShadowBuffer.raw[1][0], CorrespondingPointInShadowBuffer.raw[2][0] };
float shadowFactor = 1.0f;
constexpr float shadowK = 1.0f;
float shadowBias = shadowK*(1.0f - n*l);
if ((int)CorrespondingPoint.x < DepthBuffer.size() && (int)CorrespondingPoint.y < DepthBuffer[(int)CorrespondingPoint.x].size())
{
shadowFactor = 0.3f + 0.7f*(DepthBuffer[(int)CorrespondingPoint.x][(int)CorrespondingPoint.y] > CorrespondingPoint.z - shadowBias);//1 is shadow bias
}
```

Reference:

- https://learnopengl-cn.readthedocs.io/zh/latest/05%20Advanced%20Lighting/03%20Shadows/01%20Shadow%20Mapping/
- 实时阴影(一) ShadowMap, PCF与Face Culling - 陈陈的文章 - 知乎
  https://zhuanlan.zhihu.com/p/477330771

##### Shadow Peter-panning

这其实是由于ShdowBias导致的：

![image-20231224204404293](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231224204404293.png)

本来不能通过阴影测试，bias设置过大，导致本该被遮挡的物体，因为减去了bias导致深度值变小，通过了阴影测试。

解决方法：

1.其实bias设置合理一点即可，不要太大。

2.也可以当渲染深度贴图时候使用正面剔除（front face culling），我们只利用背面，这样阴影失真也解决了，不再需要bias的辅助。使用这个方法可以免去反复调整bias，并且也不会导致悬浮的问题。但使用该方法的前提是**
场景中的物体必须是一个体（有正面和反面）而非原先的一个面**。

##### Shadow Aliasing

我们上述的阴影有的锯齿：

![image-20231224211837915](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231224211837915.png)

- 锯齿本质是采样频率不够，最直接的解决方法就是 提高采样频率，但也会带来时间/空间的开销
- 使用PCF/PCSS等各种效果更好的阴影算法

##### 大场景中ShadowMap找不到对应的点

当一个像素点在Shadow Map中找不到对应的深度值是很常见的问题，比如你的光源没有把整个角色照到。

以下是几种处理方法：

1. 固定阴影：将超出Shadow Map范围的像素点视为在阴影中，这样就可以给予物体一个全局的阴影效果。这种方法适用于开放世界游戏中的遥远物体或无法包含在Shadow Map中的大场景。
2. 边界处理：在将Shadow Map的深度值与对应像素点的深度值进行比较时，如果像素点处于阴影区域的边界上（即在Shadow Map范围内但未找到对应深度值），可以根据相邻像素点的深度值进行插值，以消除锯齿或产生更平滑的阴影过渡效果。
3. PSSM（Parallel Split Shadow Maps）：PSSM是一种将阴影贴图分成多个级别的技术，每个级别具有不同的观察矩阵和投影矩阵。如果一个像素点不在当前级别的Shadow Map中，可以尝试从下一个级别的Shadow
   Map中查找对应深度值。这种方法在遥远物体的阴影渲染中特别有效。
4. 使用近似算法：有时可以使用近似算法来估计不在Shadow Map中的像素点的阴影值。例如，可以使用屏幕空间的法线、深度等信息进行计算，以生成一个近似的阴影效果。

#### PCF(Percentage Closer Filtering)

PCF全称是Percentage Closer Filtering，优化阴影测试时的采样方式，对一个shadowbuffer上的点的周围3*3采样，相当于模糊处理，以节约阴影锯齿的问题。

| Without PCF                                                  | With PCF                                                     |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| ![image-20231226201932225](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231226201932225.png) | ![image-20231226202027397](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231226202027397.png) |

```c++
    //PCF
for (int i = 1; i < width-1; i++)
{
for (int j = 1; j< height-1; j++)
{
int SampleShadowNumber = 0;
for (int dx = -1; dx <= 1; dx++)
{
for (int dy = -1; dy <= 1; dy++)
{
SampleShadowNumber += ShadowBuffer[i + dx][j + dy];
}
}
float SampleShadowRate = SampleShadowNumber/(3.0f*3.0f);
float PCFShadowFactor = 1.0f - 0.7f*SampleShadowRate;
image2.set(i, j, image2.get(i, j) * PCFShadowFactor);
}
}

```

#### PCSS(Percentage-Closer Soft Shadows)

PCF虽然“软化”了阴影，但是我们阴影往往是由硬阴影+软阴影组成，PCF这种全软化的方式算不上很好的效果。

现实中的阴影往往是如下图，沿着红线越来越“软”

![image-20231228171039621](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231228171039621.png)

PCSS就是动态适应的选择PCF模糊的size：

![image-20231228171344785](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231228171344785.png)

如上图，只需要遮挡处到光源处的竖直距离$d_{Blocker}$, 投射阴影处到光源处的竖直距离$d_{Receiver}$，还需要一个光源大小$W_{Light}$,

![image-20240102214451107](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20240102214451107.png)

我们法线过度好像好生硬，这是因为$d_{Blocker}$出现了突变，即我们计算average $d_{Blocker}$的时候选择的采样半径太小，导致变化不够平滑，我们调整计算平均$d_{Blocker}$的采样大小为40，$W_
{light}$大小为50：过度平滑了很多

![image-20240102215517557](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20240102215517557.png)

我们使用上述的两种软阴影方法PCF/PCSS中，不难发现因为做高斯模糊导致在脚趾处有一定的漏光，这是因为对于脚趾边缘的阴影点在计算时高斯模糊导致了边缘很软，解决漏光的方法也很直接，**使用带权的高斯模糊**
，而不是权重相同，权重可以以两个点之间的世界空间距离的倒数作为权重，即两个点差距很大(一点在地板的阴影上，另一个点在脚趾上)，这个点的权重贡献会很小。

Reference:

- [高质量实时渲染：实时软阴影](https://yangwc.com/2021/04/14/PCSS/)
- Unity SRP 实战（三）PCSS 软阴影与性能优化 - AKG4e3的文章 - 知乎https://zhuanlan.zhihu.com/p/462371147
- 高效的泊松圆盘采样：[Robert Bridson](http://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph07-poissondisk.pdf)

#### CSM(Cascaded Shadow Mapping)

 一个常见的疑问是为什么有的叫它CSM，有的叫他PSSM（Parallel-Split Shadow Map）。实际上原论文是把它叫做PSSM，工业界实现的时候选择了个更广泛的名字CSM。从名字本身来说，只要是一系列层级关系的shadow
map，就能叫CSM，而只有平行切分视锥的才叫做PSSM。换句话说，CSM是个种类，PSSM是具体方法。

​    ![image-20231224214449007](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231224214449007.png)

CSGO游戏中就是用了CSM，保证了近处阴影的细节，远处阴影的基本轮廓。

Reference:

https://learnopengl.com/Guest-Articles/2021/CSM

#### SDF Soft Shadows

#### VSM/VSSM

- 实时阴影(三) VSM与VSSM - 陈陈的文章 - 知乎 https://zhuanlan.zhihu.com/p/483674565

#### ESM

### AO(Ambient Occlusion)

#### SSAO(Screen-Space Ambient Occlusion)

- AO的概念理解 https://mentalraytips.blogspot.com/2008/11/joy-of-little-ambience.html

- https://learnopengl-cn.readthedocs.io/zh/latest/05%20Advanced%20Lighting/09%20SSAO/
- http://john-chapman-graphics.blogspot.com/2013/01/ssao-tutorial.html
- 球体均匀采样https://mathworld.wolfram.com/SpherePointPicking.html

- 游戏后期特效第四发 -- 屏幕空间环境光遮蔽(SSAO) - 音速键盘猫的文章 - 知乎
  https://zhuanlan.zhihu.com/p/25038820

实现的几个细节：

- 物体相差远的地方, 会在深度图上误认为有AO,实际上不应该有AO，所以要做RangeCheck

  <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231205011832980.png" alt="image-20231205011832980" style="zoom: 67%;" />

- 降噪: 双边滤波

- 降采样提升运行效率

- 法向半球采样 + 加速插值函数

工作流：

![image-20240115142822822](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20240115142822822.png)

最后效果:

![image-20240117205040340](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20240117205040340.png)

#### HBAO(Image-space Horizon-based Ambient Occlusion)

| With HBAO                                                    | Without HBAO                                                 |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| ![image-20231204200320537](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231204200320537.png) | <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231204201046444.png" alt="image-20231204201046444" style="zoom:85%;" /> |

工作流是:

- 降低分辨率
- 对每个随机方向进行RayMatching找最大水平角
- 做Blur

HBAO(屏幕空间的环境光遮蔽) - YiQiuuu的文章 - 知乎
https://zhuanlan.zhihu.com/p/103683536

### Anti-Aliasing

#### TAA

#### MSAA

#### FXAA

### SSR(Screen Space Reflection)

- [图形学基础|屏幕空间反射(SSR)](https://blog.csdn.net/qjh5606/article/details/120102582#SSR_0)

### PBR

### IBL

### CubeMap

### Scene Management

Multi Objects Render Pipeline is supported~ 
