# BSD 2-Clause License
#
# Copyright (c) 2021, CNRS-UM LIRMM, CNRS-AIST JRL
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import bpy
from bpy.types import Operator

from .blender_imgui import ImguiBasedOperator
import imgui

# -------------------------------------------------------------------

class ImguiExample(Operator,ImguiBasedOperator):
    """Example of modal operator using ImGui"""
    bl_idname = "object.imgui_example"
    bl_label = "Imgui Example"

    def draw(self, context):
        # This is where you can use any code from pyimgui's doc
        # see https://pyimgui.readthedocs.io/en/latest/
        imgui.begin("Your first window!", True)
        imgui.text("Hello world!")
        imgui.text("Another line!")
        imgui.text("And yet another")
        changed, self.color = imgui.color_edit3("Pick a color: Color", *self.color)
        changed, self.message = imgui.input_text_multiline(
            'Message:',
            self.message,
            2056
        )
        imgui.text_colored(self.message, *self.color)
        imgui.end()

    def invoke(self, context, event):
        self.color = (1.,.5,0.)
        self.message = "Type something here!"
        # Call init_imgui() at the beginning
        self.init_imgui(context)
        context.window_manager.modal_handler_add(self)
        return {'RUNNING_MODAL'}

    def modal(self, context, event):
        context.area.tag_redraw()

        # Handle the event as you wish here, as in any modal operator
        if event.type in {'ESC'}:
            # Call shutdown_imgui() any time you'll return {'CANCELLED'} or {'FINISHED'}
            self.shutdown_imgui()
            return {'CANCELLED'}

        # Don't forget to call parent's modal:
        if self.modal_imgui(context, event):
            return {'RUNNING_MODAL'}
        else:
            return {'PASS_THROUGH'}

# -------------------------------------------------------------------

classes = (
    ImguiExample,
)

register, unregister = bpy.utils.register_classes_factory(classes)
