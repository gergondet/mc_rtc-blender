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

from .blender_imgui import ImguiBasedOperator, imgui

from math import cos, sin, pi

# -------------------------------------------------------------------

class ImguiExample(Operator,ImguiBasedOperator):
    """Example of modal operator using ImGui"""
    bl_idname = "object.imgui_example"
    bl_label = "Imgui Example"

    def __init__(self):
        super().__init__()
        self.calls = []
        self.cubes = []
        self.t = 0.0
        self.dt = 0.05
        bpy.app.timers.register(self.animate_cubes)
        self.timer = bpy.context.window_manager.event_timer_add(0.1, window = bpy.context.window)

    def __del__(self):
        bpy.app.timers.unregister(self.animate_cubes)
        bpy.context.window_manager.event_timer_remove(self.timer)
        super().__del__()

    def draw(self, context):
        # This is where you can use any code from pyimgui's doc
        # see https://pyimgui.readthedocs.io/en/latest/
        imgui.show_demo_window()
        imgui.begin("mc_rtc addon", True)
        #changed, self.color = imgui.color_edit3("Pick a color: Color", *self.color)
        #changed, self.message = imgui.input_text_multiline(
        #    'Message:',
        #    self.message,
        #    2056
        #)
        imgui.text(self.message)
        #imgui.text_colored(self.message, *self.color)
        #if imgui.button("POP CUBE"):
        #    self.calls.append(lambda: self.add_cube())
        imgui.end()

    def add_cube(self):
        bpy.ops.mesh.primitive_cube_add(size = 0.1)
        self.cubes.append(bpy.context.visible_objects[-1])

    def animate_cubes(self):
        r = 1.0
        theta = self.t * 2 * pi
        for c in self.cubes:
            c.location = [r*cos(theta), r*sin(theta), 0.0]
            r += 1.0
        self.t += self.dt
        return self.dt

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
        busy = self.modal_imgui(context, event)
        for c in self.calls:
            c()
        self.calls.clear()
        if busy:
            return {'RUNNING_MODAL'}
        else:
            return {'PASS_THROUGH'}

# -------------------------------------------------------------------

classes = (
    ImguiExample,
)

register, unregister = bpy.utils.register_classes_factory(classes)
