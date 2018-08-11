const MilkdropShaderConverter = require('../build/Release/MilkdropShaderConverter.node');

describe('MilkdropShaderConverter', () => {
  const HLSLShader = `float4 xlat_main (float2 uv : TEXCOORD0) : COLOR0 {
      float3 ret = float3(0.1, 0.2, 0.3);
      return float4(ret, 1.0);
    };`;

  const GLSLShader =
`#version 300 es
out vec4 _glesFragData[4];
void main ()
{
  highp vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = vec3(0.1, 0.2, 0.3);
  _glesFragData[0] = tmpvar_1;
}`;

  it('converts HLSL to GLSL', () => {
    expect(MilkdropShaderConverter.convertHLSLString(HLSLShader).toString().trim()).toEqual(GLSLShader.trim());
  });

  it('returnes undefined if shader cannot be converted', () => {
    expect(MilkdropShaderConverter.convertHLSLString("")).toBeUndefined;
  });
});
