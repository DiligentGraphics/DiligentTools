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

import subprocess
import os
import sys
from argparse import ArgumentParser
from cxx_parser import generate_file, generate_common, generate_filename
from cxx_config import *


def main():
    parser = ArgumentParser("Generate sources files")
    parser.add_argument("--dir",
                        required=True,
                        help="Output directory")

    parser.add_argument("--files",
                        required=True,
                        nargs='+',
                        help="Path to file")

    args = parser.parse_args()

    for file in args.files:
        generated_filename = generate_filename(file)
        print(f"Generate CXX -> {generated_filename}")
        generate_file(file, os.path.join(args.dir, generated_filename))

    generated_filename = generate_filename("Common")
    generate_common(os.path.join(args.dir, generated_filename))
    print(f"Generate CXX -> {generated_filename}")


if __name__ == "__main__":
    main()
