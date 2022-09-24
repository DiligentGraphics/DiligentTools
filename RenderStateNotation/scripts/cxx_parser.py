# ----------------------------------------------------------------------------
# Copyright 2019-2022 Diligent Graphics LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# In no event and under no legal theory, whether in tort (including negligence),
# contract, or otherwise, unless required by applicable law (such as deliberate
# and grossly negligent acts) or agreed to in writing, shall any Contributor be
# liable for any damages, including any direct, indirect, special, incidental,
# or consequential damages of any character arising as a result of this License or
# out of the use or inability to use the software (including but not limited to damages
# for loss of goodwill, work stoppage, computer failure or malfunction, or any and
# all other commercial damages or losses), even if such Contributor has been advised
# of the possibility of such damages.
# ----------------------------------------------------------------------------

import sys
import typing
import os
import subprocess
import re

from argparse import ArgumentParser
from clang.cindex import Index, Type, TypeKind, Cursor, CursorKind, AccessSpecifier, TranslationUnit
from difflib import get_close_matches
from pathlib import Path
from cxx_config import *
from cxx_builder import CXXBuilder
from cxx_template import *

CXX_INCLUDE_FILE = '#include "{}"'
CXX_PRAGMA_ONCE = "#pragma once"
CXX_NAMESPACE = "namespace {}"

CXX_PATTERN_INTERFACE = r"(Diligent::I[A-Z].*[a-z])|(struct I[A-Z].*[a-z])"
CXX_PATTERN_STRING = r"const(\s)+(char|Diligent::Char)(\s)+\*$"

def filter_by_file(nodes: typing.Iterable[Cursor], file_name: str) -> typing.Iterable[Cursor]:
    return [node for node in nodes if node.location.file.name == file_name] 

def filter_by_node_kind(nodes: typing.Iterable[Cursor], kinds: list) -> typing.Iterable[Cursor]:
    return [node for node in nodes if node.kind in kinds]

def filter_namespace(nodes: typing.Iterable[Cursor], namespace_name: str) -> typing.Iterable[Cursor]:
    return [node for node in nodes if node.kind in [CursorKind.NAMESPACE] and node.spelling == namespace_name ]

def filter_bitwise_enum(translation_unit):
    #TODO Ugly Code 
    bitwise_enum = set()
    for namespace in filter_namespace(translation_unit.cursor.get_children(), 'Diligent'):
        for unexposed in filter_by_node_kind(namespace.get_children(), [CursorKind.UNEXPOSED_DECL]):  
            for function in filter_by_node_kind(unexposed.get_children(), [CursorKind.FUNCTION_DECL]):
                for param in filter_by_node_kind(function.get_children(), [CursorKind.PARM_DECL]):  
                    name_type = param.type.spelling.split("::")[1]
                    if name_type in CXX_REGISTERED_ENUM:
                        bitwise_enum.add(name_type)
    return list(bitwise_enum)

def find_all_fields(cursor: Cursor, bitwise_enum, base_structs) -> typing.Iterable[typing.Tuple[str, Type]]:
    #TODO Ugly Code 
    result = []
    field_declarations = filter_by_node_kind(cursor.get_children(), [CursorKind.FIELD_DECL, CursorKind.UNION_DECL])
    reference_types = [TypeKind.POINTER]

    for struct_name in base_structs:
        if struct_name in CXX_REGISTERD_BASE_STRUCT:
           result.append(CXX_REGISTERD_BASE_STRUCT[struct_name]) 
        else:
            raise Exception(f'Unexpected base struct: {struct_name}')

    for node in field_declarations:
        expression = replace_raw_type(node.type.spelling)
        if (re.match(CXX_PATTERN_STRING, node.type.spelling)) is not None:
            result.append({ 'name': node.displayname, 'type': node.type.spelling, 'meta': 'string' })
        elif (node.type.kind == TypeKind.POINTER) and (re.match(CXX_PATTERN_INTERFACE, node.type.spelling) is not None):
            pass
        elif (node.type.get_declaration().is_anonymous()):
            result.extend([{ 'name': union.displayname, 'type': union.type.spelling, 'meta': 'union' } for union in node.get_children() ])
        elif (node.type.kind == TypeKind.CONSTANTARRAY):
            result.append({ 'name': node.displayname, 'type': node.type.spelling, 'meta': 'const_array' })
        elif (node.type.kind == TypeKind.ENUM) and expression in bitwise_enum:
            result.append({ 'name': node.displayname, 'type': node.type.spelling, 'meta': 'bitwise' })
        elif (node.type.kind in reference_types):
            result.append({ 'name': node.displayname, 'type': node.type.spelling, 'meta': 'pointer' })
        elif expression in CXX_REGISTERED_STRUCT:
            result.append({ 'name': node.displayname, 'type': node.type.spelling, 'meta': 'struct' }) 
        else:
            result.append({ 'name': node.displayname, 'type': node.type.spelling, 'meta': '' }) 

    return result

def find_all_base_structs(cursor: Cursor):
    return [node.referenced.spelling for node in cursor.get_children() if node.kind == CursorKind.CXX_BASE_SPECIFIER]

def find_all_xitems(cursor: Cursor) -> typing.Iterable[str]:
    xitems = [node.displayname for node in filter_by_node_kind(cursor.get_children(), [CursorKind.ENUM_CONSTANT_DECL])]
    return [{ 'value': x, 'name': y} for x, y in zip(xitems, replace_enum_string(xitems))]

def compute_all_enums(enums):
    return {node.spelling: find_all_xitems(node) for node in enums if node.spelling in CXX_REGISTERED_ENUM }

def compute_all_structs(structs, bitwise_enum):
    return { struct.spelling : { 'fields': find_all_fields(struct, bitwise_enum, find_all_base_structs(struct)) } for struct in structs if struct.spelling in CXX_REGISTERED_STRUCT }

def compute_all_fields_size(struct_field_map):
    #TODO Ugly Code 
    size_fields_map = {}
    size_fields_map_inv = {}
    condition = lambda x: x['meta'] == 'pointer'

    for struct, node_info in struct_field_map.items():
        all_fields = struct_field_map[struct]['fields']
        pod_fields = [field['name'] for field in all_fields if not condition(field)]
        result_0 = {}
        result_1 = {}
        for field in all_fields:
            if condition(field):
                match_str = get_close_matches(field['name'], pod_fields)
                if match_str:
                    result_0[field['name']] = match_str[0]
                    result_1[match_str[0]] = field['name']
        if result_0 and result_1:
            size_fields_map[struct] = result_0
            size_fields_map_inv[struct] = result_1

    return (size_fields_map, size_fields_map_inv)

def replace_enum_string(strings: typing.Iterable[str]) -> typing.Iterable[str]:
    prefix = os.path.commonprefix(strings)
    return [ string.replace(prefix, '') for string in strings ]

def replace_raw_type(string):
    string = string.replace('Diligent', '')
    string = string.replace('::', '')
    string = string.replace('struct', '')
    string = string.replace('enum', '')
    string = string.replace('*', '')
    string = string.replace(' ', '')
    return string

def generate_file(input_filename, output_filename):
    index = Index.create()
    translation_unit = index.parse(input_filename, args=['-x', 'c++', '-std=c++14'])
    source = filter_by_file(translation_unit.cursor.get_children(), translation_unit.spelling)
    bitwise_enum = filter_bitwise_enum(translation_unit)

    cpp = CXXBuilder()
    cpp.write_line(CXX_LICENCE)
    cpp.write_line(CXX_PRAGMA_ONCE)
    cpp.write_line()
    cpp.write_line(CXX_INCLUDE_FILE.format(os.path.basename(input_filename)))
    cpp.write_line()

    for namespace in filter_namespace(source, 'Diligent'):
        enums = filter_by_node_kind(namespace.get_children(), [CursorKind.ENUM_DECL])
        structs = filter_by_node_kind(namespace.get_children(), [CursorKind.STRUCT_DECL])

        emum_xitems_map = compute_all_enums(enums)
        struct_field_map = compute_all_structs(structs, bitwise_enum)
        field_size_map = compute_all_fields_size(struct_field_map)

        with cpp.block(CXX_NAMESPACE.format(namespace.spelling)):
            cpp.write(CXX_ENUM_SERIALIZE_TEMPLATE.render(enums=emum_xitems_map.items()))        
            cpp.write(CXX_STRUCT_SERIALIZE_TEMPLATE.render(structs=struct_field_map.items(), field_size=field_size_map[0], field_size_inv=field_size_map[1]))
            cpp.write_line()

    cpp.save(output_filename)

def generate_common(output_filename):
     cpp = CXXBuilder()
     cpp.write_line(CXX_LICENCE)
     cpp.write_line(CXX_PRAGMA_ONCE)
     cpp.write_line()

     with cpp.block(CXX_NAMESPACE.format("Diligent")):
         cpp.write(CXX_COMMON_SERIALIZE_TEMPLATE.render())       
         cpp.write_line()
     cpp.save(output_filename)

def generate_filename(input_filename):
    base_name = os.path.splitext(os.path.basename(input_filename))[0]
    return f"{base_name}{CXX_SUFFIX_FILE}.{CXX_EXTENSION_FILE}"

def main():
    parser = ArgumentParser()
    parser.add_argument("--dir",
                        required=True,
                        help="Output directory")

    parser.add_argument("--file",
                        required=True,
                        help="The path to the file")

    args = parser.parse_args()

    generate_file(args.file, os.path.join(args.dir, generate_filename(args.file)))

if __name__ == "__main__":
    main()
