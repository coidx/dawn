//* Copyright 2020 The Dawn Authors
//*
//* Licensed under the Apache License, Version 2.0 (the "License");
//* you may not use this file except in compliance with the License.
//* You may obtain a copy of the License at
//*
//*     http://www.apache.org/licenses/LICENSE-2.0
//*
//* Unless required by applicable law or agreed to in writing, software
//* distributed under the License is distributed on an "AS IS" BASIS,
//* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//* See the License for the specific language governing permissions and
//* limitations under the License.
//*
//*
//* This template itself is part of the Dawn source and follows Dawn's license
//* but the generated file is used for "Web API native". The template comments
//* using //* at the top of the file are removed during generation such that
//* the resulting file starts with the BSD 3-Clause comment.
//*
//*
// BSD 3-Clause License
//
{% set year = metadata.copyright_year if metadata.copyright_year else 2019 %}
// Copyright (c) {{year}}, "{{metadata.api}} native" developers
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#ifndef {{metadata.api.upper()}}_H_
#define {{metadata.api.upper()}}_H_

{% set c_prefix = metadata.c_prefix %}
#if defined({{c_prefix}}_SHARED_LIBRARY)
#    if defined(_WIN32)
#        if defined({{c_prefix}}_IMPLEMENTATION)
#            define {{c_prefix}}_EXPORT __declspec(dllexport)
#        else
#            define {{c_prefix}}_EXPORT __declspec(dllimport)
#        endif
#    else  // defined(_WIN32)
#        if defined({{c_prefix}}_IMPLEMENTATION)
#            define {{c_prefix}}_EXPORT __attribute__((visibility("default")))
#        else
#            define {{c_prefix}}_EXPORT
#        endif
#    endif  // defined(_WIN32)
#else       // defined({{c_prefix}}_SHARED_LIBRARY)
#    define {{c_prefix}}_EXPORT
#endif  // defined({{c_prefix}}_SHARED_LIBRARY)

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

{% for constant in by_category["constant"] %}
    #define {{c_prefix}}_{{constant.name.SNAKE_CASE()}} {{constant.value}}
{% endfor %}

typedef uint32_t {{c_prefix}}Flags;

{% for type in by_category["object"] %}
    typedef struct {{as_cType(type.name)}}Impl* {{as_cType(type.name)}};
{% endfor %}

{% for type in by_category["enum"] + by_category["bitmask"] %}
    typedef enum {{as_cType(type.name)}} {
        {% for value in type.values %}
            {{as_cEnum(type.name, value.name)}} = 0x{{format(value.value, "08X")}},
        {% endfor %}
        {{as_cEnum(type.name, Name("force32"))}} = 0x7FFFFFFF
    } {{as_cType(type.name)}};
    {% if type.category == "bitmask" %}
        typedef {{c_prefix}}Flags {{as_cType(type.name)}}Flags;
    {% endif %}

{% endfor -%}

typedef struct {{c_prefix}}ChainedStruct {
    struct {{c_prefix}}ChainedStruct const * next;
    {{c_prefix}}SType sType;
} {{c_prefix}}ChainedStruct;

typedef struct {{c_prefix}}ChainedStructOut {
    struct {{c_prefix}}ChainedStructOut * next;
    {{c_prefix}}SType sType;
} {{c_prefix}}ChainedStructOut;

{% for type in by_category["structure"] %}
    typedef struct {{as_cType(type.name)}} {
        {% set Out = "Out" if type.output else "" %}
        {% set const = "const " if not type.output else "" %}
        {% if type.extensible %}
            {{c_prefix}}ChainedStruct{{Out}} {{const}}* nextInChain;
        {% endif %}
        {% if type.chained %}
            {{c_prefix}}ChainedStruct{{Out}} chain;
        {% endif %}
        {% for member in type.members %}
            {{as_annotated_cType(member)}};
        {% endfor %}
    } {{as_cType(type.name)}};

{% endfor %}
{% for typeDef in by_category["typedef"] %}
    // {{as_cType(typeDef.name)}} is deprecated.
    // Use {{as_cType(typeDef.type.name)}} instead.
    typedef {{as_cType(typeDef.type.name)}} {{as_cType(typeDef.name)}};

{% endfor %}
#ifdef __cplusplus
extern "C" {
#endif

{% for type in by_category["callback"] %}
    typedef void (*{{as_cType(type.name)}})(
        {%- for arg in type.arguments -%}
            {% if not loop.first %}, {% endif %}{{as_annotated_cType(arg)}}
        {%- endfor -%}
    );
{% endfor %}

typedef void (*{{c_prefix}}Proc)(void);

#if !defined({{c_prefix}}_SKIP_PROCS)

typedef WGPUInstance (*WGPUProcCreateInstance)(WGPUInstanceDescriptor const * descriptor);
typedef WGPUProc (*WGPUProcGetProcAddress)(WGPUDevice device, char const * procName);

{% for type in by_category["object"] if len(c_methods(type)) > 0 %}
    // Procs of {{type.name.CamelCase()}}
    {% for method in c_methods(type) %}
        typedef {{as_cType(method.return_type.name)}} (*{{as_cProc(type.name, method.name)}})(
            {{-as_cType(type.name)}} {{as_varName(type.name)}}
            {%- for arg in method.arguments -%}
                , {{as_annotated_cType(arg)}}
            {%- endfor -%}
        );
    {% endfor %}

{% endfor %}
#endif  // !defined({{c_prefix}}_SKIP_PROCS)

#if !defined({{c_prefix}}_SKIP_DECLARATIONS)

WGPU_EXPORT WGPUInstance wgpuCreateInstance(WGPUInstanceDescriptor const * descriptor);
WGPU_EXPORT WGPUProc wgpuGetProcAddress(WGPUDevice device, char const * procName);

{% for type in by_category["object"] if len(c_methods(type)) > 0 %}
    // Methods of {{type.name.CamelCase()}}
    {% for method in c_methods(type) %}
        {{c_prefix}}_EXPORT {{as_cType(method.return_type.name)}} {{as_cMethod(type.name, method.name)}}(
            {{-as_cType(type.name)}} {{as_varName(type.name)}}
            {%- for arg in method.arguments -%}
                , {{as_annotated_cType(arg)}}
            {%- endfor -%}
        );
    {% endfor %}

{% endfor %}
#endif  // !defined({{c_prefix}}_SKIP_DECLARATIONS)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // {{metadata.api.upper()}}_H_