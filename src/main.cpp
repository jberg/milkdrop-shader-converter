// based on https://github.com/aras-p/hlsl2glslfork/blob/master/tests/hlsl2glsltest/hlsl2glsltest.cpp
#include <nan.h>
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <vector>
#include <stdexcept>

#include <dirent.h>
#include <unistd.h>

#include "hlsl2glslfork/include/hlsl2glsl.h"
#include "glsl-optimizer/src/glsl/glsl_optimizer.h"

using v8::FunctionTemplate;
using v8::String;

static std::string GetCompiledShaderText (ShHandle parser)
{
    std::string txt = Hlsl2Glsl_GetShader (parser);

    int count = Hlsl2Glsl_GetUniformCount (parser);
    if (count > 0)
    {
        const ShUniformInfo* uni = Hlsl2Glsl_GetUniformInfo(parser);
        txt += "\n// uniforms:\n";
        for (int i = 0; i < count; ++i)
        {
            char buf[1000];
            snprintf(buf,1000,"// %s:%s type %d arrsize %d\n", uni[i].name, uni[i].semantic?uni[i].semantic:"<none>", uni[i].type, uni[i].arraySize);
            txt += buf;
        }
    }

    return txt;
}

static std::string ConvertString (std::string shaderStr,
                                  const char* entryPoint,
                                  ETargetVersion version,
                                  unsigned options)
{
    const char* sourceStr = shaderStr.c_str();

    ShHandle parser = Hlsl2Glsl_ConstructCompiler (EShLangFragment);

    std::string text = "";

    int parseOk = Hlsl2Glsl_Parse (parser, sourceStr, version, NULL, options);
    const char* infoLog = Hlsl2Glsl_GetInfoLog( parser );


    if (parseOk)
    {
        static EAttribSemantic kAttribSemantic[] = {
            EAttrSemTangent,
        };
        static const char* kAttribString[] = {
            "TANGENT",
        };
        Hlsl2Glsl_SetUserAttributeNames (parser, kAttribSemantic, kAttribString, 1);

        int translateOk = Hlsl2Glsl_Translate (parser, entryPoint, version, options);
        const char* infoLog = Hlsl2Glsl_GetInfoLog( parser );
        if (translateOk)
        {
            text = GetCompiledShaderText(parser);

            for (size_t i = 0, n = text.size(); i != n; ++i)
            {
                char c = text[i];

                if (!isascii(c))
                {
                    throw std::runtime_error(std::string("Contains non-ascii character: ") + c);
                }
            }

            std::string newSrc;
            newSrc.reserve(text.size());

            newSrc += "#version 300 es\n";
            newSrc += "#define lowp\n";
            newSrc += "#define mediump\n";
            newSrc += "#define highp\n";
            newSrc += "#define gl_Vertex _glesVertex\n";
            newSrc += "#define gl_Normal _glesNormal\n";
            newSrc += "#define gl_Color _glesColor\n";
            newSrc += "#define gl_MultiTexCoord0 _glesMultiTexCoord0\n";
            newSrc += "#define gl_MultiTexCoord1 _glesMultiTexCoord1\n";
            newSrc += "#define gl_MultiTexCoord2 _glesMultiTexCoord2\n";
            newSrc += "#define gl_MultiTexCoord3 _glesMultiTexCoord3\n";
            newSrc += "in highp vec4 _glesVertex;\n";
            newSrc += "in highp vec3 _glesNormal;\n";
            newSrc += "in lowp vec4 _glesColor;\n";
            newSrc += "in highp vec4 _glesMultiTexCoord0;\n";
            newSrc += "in highp vec4 _glesMultiTexCoord1;\n";
            newSrc += "in highp vec4 _glesMultiTexCoord2;\n";
            newSrc += "in highp vec4 _glesMultiTexCoord3;\n";
            newSrc += "#define gl_FragData _glesFragData\n";
            newSrc += "out lowp vec4 _glesFragData[4];\n";
            newSrc += text;

            text = newSrc;
        }
        else
        {
            throw std::runtime_error(std::string("HLSL translation error:") + infoLog);
        }
    }
    else
    {
        throw std::runtime_error(std::string("HLSL parse error:") + infoLog);
    }

    Hlsl2Glsl_DestructCompiler (parser);

    return text;
}

NAN_METHOD(ConvertHLSLString) {
  if (!info[0]->IsString()) return Nan::ThrowError("Must pass a string");
  Nan::Utf8String inputShader(info[0]);

  bool optimize = true;
  if (info.Length() > 1 && info[1]->IsBoolean()) {
    optimize = info[1]->BooleanValue();
  }

  try {
      Hlsl2Glsl_Initialize ();

      bool converted = false;
      // bool logging = false;
      size_t errors = 0;
      // if(logging) printf ("TESTING %s...\n", "fragment");
      const ETargetVersion version1 = ETargetGLSL_ES_300;

      std::string text = ConvertString(*inputShader, "main", version1, ETranslateOpNone);

      if (optimize) {
        const char* shaderOutput;
        if (!text.empty()) {
            // if(logging) printf ("FRAG SHADER: \n %s", text.c_str());

            glslopt_ctx* ctx = glslopt_initialize(kGlslTargetOpenGLES30);
            glslopt_shader* shader = glslopt_optimize (ctx, kGlslOptShaderFragment, text.c_str(), 0);

            const char* errorLog;
            if (glslopt_get_status (shader)) {
                shaderOutput = glslopt_get_output (shader);

                converted = true;
                info.GetReturnValue().Set(Nan::CopyBuffer(shaderOutput, strlen(shaderOutput)).ToLocalChecked());
            }
            // else if(logging) {
            //     printf ("ERROR LOG: \n %s", glslopt_get_log (shader));
            // }
            glslopt_shader_delete (shader);
            glslopt_cleanup (ctx);
        }

        Hlsl2Glsl_Shutdown();

        if (!converted) {
          info.GetReturnValue().Set(Nan::Undefined());
        }
      } else {
        info.GetReturnValue().Set(Nan::CopyBuffer(text.c_str(), strlen(text.c_str())).ToLocalChecked());
      }
  }
  catch (const std::runtime_error& ex) {
    return Nan::ThrowError(ex.what());
  }
  catch(...) {
    return Nan::ThrowError("An error occurred translating HLSL");
  }
}

NAN_MODULE_INIT(InitModule) {
  Nan::Set(target, Nan::New<String>("convertHLSLString").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(ConvertHLSLString)).ToLocalChecked());
}

NODE_MODULE(MilkdropShaderConverter, InitModule)
