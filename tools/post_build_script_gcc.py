#!/usr/bin/env python
#
# DAPLink Interface Firmware
# Copyright (c) 2009-2020, ARM Limited, All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import os
import shutil
from post_build_script import post_build_script

project_dir = 'build'
# project_dir = os.path.join(os.getcwd(), 'build')
project_name = os.path.basename(os.getcwd())
infile = os.path.join(project_dir, project_name + '.hex')
outbase = os.path.join(project_dir, project_name + '_crc')

if not os.path.exists(infile):
    print("File '%s' is missing" % infile)
    exit(-1)

run = False
if not os.path.exists(outbase + '.bin'):
    run = True
elif os.path.getmtime(infile) >= os.path.getmtime(outbase + '.bin'):
    run = True
# else:
#     print("%s already up-to-date" % outbase)

if run:
    # print("%s -> %s" % (infile, outbase))
    post_build_script(infile, outbase)

# Copy output files to projectfiles/(toolchain)/
def copy_build_outputs():
    """Copy hex and bin files to projectfiles/make_gcc_arm/"""
    # Find project root (walk up from current directory)
    current = os.getcwd()
    project_root = None
    while current != os.path.dirname(current):
        if os.path.exists(os.path.join(current, 'projectfiles')):
            project_root = current
            break
        if os.path.basename(os.path.dirname(current)) == 'projectfiles':
            project_root = os.path.dirname(os.path.dirname(current))
            break
        current = os.path.dirname(current)
    
    if project_root is None:
        project_root = os.path.join(os.getcwd(), '..', '..', '..')
    
    dest_dir = os.path.join(project_root, 'projectfiles', 'make_gcc_arm')
    os.makedirs(dest_dir, exist_ok=True)
    
    # Copy _crc files first, then regular files as fallback
    for ext in ['.hex', '.bin']:
        for suffix in ['_crc', '']:
            src = os.path.join(project_dir, project_name + suffix + ext)
            if os.path.exists(src):
                dest = os.path.join(dest_dir, os.path.basename(src))
                try:
                    shutil.copy2(src, dest)
                    # print("Copied: %s -> %s" % (src, dest))
                except Exception as e:
                    print("Error copying %s: %s" % (src, e))
                break

copy_build_outputs()
