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

  const GLSLShaderUnoptimized =
`#version 300 es
#define lowp
#define mediump
#define highp
#define gl_Vertex _glesVertex
#define gl_Normal _glesNormal
#define gl_Color _glesColor
#define gl_MultiTexCoord0 _glesMultiTexCoord0
#define gl_MultiTexCoord1 _glesMultiTexCoord1
#define gl_MultiTexCoord2 _glesMultiTexCoord2
#define gl_MultiTexCoord3 _glesMultiTexCoord3
in highp vec4 _glesVertex;
in highp vec3 _glesNormal;
in lowp vec4 _glesColor;
in highp vec4 _glesMultiTexCoord0;
in highp vec4 _glesMultiTexCoord1;
in highp vec4 _glesMultiTexCoord2;
in highp vec4 _glesMultiTexCoord3;
#define gl_FragData _glesFragData
out lowp vec4 _glesFragData[4];

#line 1
highp vec4 xlat_main( in highp vec2 uv ) {
    highp vec3 ret = vec3( 0.1, 0.2, 0.3);
    #line 3
    return vec4( ret, 1.0);
}
in highp vec2 xlv_TEXCOORD0;
void main() {
    highp vec4 xl_retval;
    xl_retval = xlat_main( vec2(xlv_TEXCOORD0));
    gl_FragData[0] = vec4(xl_retval);
}`;

  it('converts HLSL to GLSL', () => {
    expect(MilkdropShaderConverter.convertHLSLString(HLSLShader).toString().trim()).toEqual(GLSLShader.trim());
  });

  it('converts HLSL to GLSL without optimizing', () => {
    expect(MilkdropShaderConverter.convertHLSLString(HLSLShader, false).toString().trim()).toEqual(GLSLShaderUnoptimized.trim());
  });

  it('throws error if shader cannot be converted', () => {
    expect(() => MilkdropShaderConverter.convertHLSLString("")).toThrow(new Error("HLSL parse error:(1): ERROR: '' : syntax error syntax error\n"));
  });
});
