// based on https://github.com/aras-p/hlsl2glslfork/blob/master/tests/hlsl2glsltest/hlsl2glsltest.cpp
#include <iostream>
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <string>
#include <vector>
#include <time.h>
#include <assert.h>

#include <dirent.h>
#include <unistd.h>

#include "../hlsl2glslfork/include/hlsl2glsl.h"
#include "../glsl-optimizer/src/glsl/glsl_optimizer.h"

static void replace_string (std::string& target, const std::string& search, const std::string& replace, size_t startPos);

static bool ReadStringFromFile (const char* pathName, std::string& output)
{
    FILE* file = fopen( pathName, "rb" );
    if (file == NULL)
        return false;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (length < 0)
    {
        fclose( file );
        return false;
    }

    output.resize(length);
    size_t readLength = fread(&*output.begin(), 1, length, file);

    fclose(file);

    if (readLength != length)
    {
        output.clear();
        return false;
    }

    replace_string(output, "\r\n", "\n", 0);

    return true;
}


static void replace_string (std::string& target, const std::string& search, const std::string& replace, size_t startPos)
{
    if (search.empty())
        return;

    std::string::size_type p = startPos;
    while ((p = target.find (search, p)) != std::string::npos)
    {
        target.replace (p, search.size (), replace);
        p += replace.size ();
    }
}


static std::string GetCompiledShaderText(ShHandle parser)
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


static std::string TestString (std::string shaderStr,
                               const char* entryPoint,
                               ETargetVersion version,
                               unsigned options)
{
    assert(version != ETargetVersionCount);

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
                    printf ("  contains non-ascii '%c' (0x%02X)\n", c, c);
                    text = "";
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
            printf ("  translate error: %s\n", infoLog);
            text = "";
        }
    }
    else
    {
        printf ("  parse error: %s\n", infoLog);
        text = "";
    }

    Hlsl2Glsl_DestructCompiler (parser);

    return text;
}

int main (int argc, const char** argv)
{

    std::string inputStr;
    if (argc < 2)
    {
        inputStr = "";
        std::string lineInput;
        while (getline(std::cin,lineInput)) {
            inputStr += lineInput + "\n";
        }
    }
    else {
        std::string fragShaderFile = argv[1];

        std::string input;
        if (!ReadStringFromFile (fragShaderFile.c_str(), inputStr))
        {
            printf ("  failed to read input file\n");
            return 1;
        }

    }

    bool logging = false;

    clock_t time0 = clock();

    Hlsl2Glsl_Initialize ();

    size_t tests = 1;
    size_t errors = 0;
    if(logging) printf ("TESTING %s...\n", "fragment");
    const ETargetVersion version1 = ETargetGLSL_ES_300;

    std::string text = TestString(inputStr, "main", version1, ETranslateOpNone);

    if (text.empty())
        ++errors;
    else {
        if(logging) printf ("FRAG SHADER: \n %s", text.c_str());

        glslopt_ctx* ctx = glslopt_initialize(kGlslTargetOpenGLES30);

        glslopt_shader* shader = glslopt_optimize (ctx, kGlslOptShaderFragment, text.c_str(), 0);
        const char* newSource;
        const char* errorLog;
        if (glslopt_get_status (shader)) {
            newSource = glslopt_get_output (shader);

            if(logging) printf ("OPTIMIZED FRAG SHADER: \n");
            printf ("%s", newSource);
        } else {
            errorLog = glslopt_get_log (shader);

            if(logging) printf ("ERROR LOG: \n %s", errorLog);
        }
        glslopt_shader_delete (shader);
        glslopt_cleanup (ctx);

    }
    clock_t time1 = clock();
    float t = float(time1-time0) / float(CLOCKS_PER_SEC);
    if (errors != 0) {
        if(logging) printf ("%i tests, %i FAILED, %.2fs\n", (int)tests, (int)errors, t);\
    }
    else {
        if(logging) printf ("%i tests succeeded, %.2fs\n", (int)tests, t);
    }
    Hlsl2Glsl_Shutdown();

    return errors ? 1 : 0;
}
