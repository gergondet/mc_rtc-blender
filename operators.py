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

import os.path

# -------------------------------------------------------------------

class BlenderInterface(imgui.Interface3D):
    def __init__(self):
        super().__init__()
        if 'mc_rtc' in bpy.data.collections:
            bpy.data.collections.remove(bpy.data.collections['mc_rtc'])
        self._collection = bpy.data.collections.new('mc_rtc')
        bpy.context.scene.collection.children.link(self._collection)

    def __del__(self):
        super().__del__()
        bpy.data.collections.remove(self._collection)

    def _get_collection(self, name):
        return self._collection.children[name]

    def _new_object_name(self, name):
        if not name in bpy.data.objects:
            return name
        i = 1
        while "{}.{:03d}".format(name, i) in bpy.data.objects:
            i += 1
        return "{}.{:03d}".format(name, i)

    def add_collection(self, category, name):
        ncol = bpy.data.collections.new('/'.join(category + [name]))
        self._collection.children.link(ncol)
        return ncol.name

    def hide_collection(self, name, hide):
        self._get_collection(name).hide_viewport = hide

    def remove_collection(self, name):
        bpy.data.collections.remove(self._get_collection(name))

    def load_mesh(self, collection, meshPath, meshName):
        ext = os.path.splitext(meshPath)[1]
        if ext.lower() == '.dae':
            bpy.ops.wm.collada_import(filepath = meshPath, import_units = True)
        elif ext.lower() == '.stl':
            bpy.ops.import_mesh.stl(filepath = meshPath, use_scene_unit = True)
        else:
            print("Requested loading of {} that I cannot handle (yet)".format(meshPath))
            return ""
        [ bpy.data.objects.remove(o) for o in bpy.context.selected_objects if o.type != 'MESH' ]
        if len(bpy.context.selected_objects) == 0:
            return ""
        bpy.context.view_layer.objects.active = bpy.context.selected_objects[0]
        bpy.ops.object.join()
        bpy.ops.object.transform_apply()
        mesh = bpy.context.selected_objects[0]
        bpy.ops.collection.objects_remove_all()
        self._get_collection(collection).objects.link(mesh)
        mesh.name = self._new_object_name(meshName)
        return mesh.name

    def set_mesh_position(self, meshName, pose):
        if meshName == "":
            return
        obj = bpy.data.objects[meshName]
        t = pose.translation
        obj.location = t
        r = pose.rotation.inverse()
        obj.rotation_mode = 'QUATERNION'
        obj.rotation_quaternion[0] = r.w()
        obj.rotation_quaternion[1] = r.x()
        obj.rotation_quaternion[2] = r.y()
        obj.rotation_quaternion[3] = r.z()

    def remove_mesh(self, meshName):
        if meshName == "":
            return
        pass


class McRtcGUI(Operator,ImguiBasedOperator):
    """mc_rtc GUI inside Blender"""
    bl_idname = "object.mc_rtc_gui"
    bl_label = "mc_rtc GUI"

    def __init__(self):
        super().__init__()
        self.timer = bpy.context.window_manager.event_timer_add(1.0 / 60.0, window = bpy.context.window)
        self._iface = BlenderInterface()
        self._client = imgui.Client(self._iface)
        self._client.connect("ipc:///tmp/mc_rtc_pub.ipc", "ipc:///tmp/mc_rtc_rep.ipc")
        self._client.timeout(1.0)

    def __del__(self):
        bpy.context.window_manager.event_timer_remove(self.timer)
        super().__del__()

    def draw(self, context):
        self._client.draw2D()
        self._client.draw3D()

    def invoke(self, context, event):
        # Call init_imgui() at the beginning
        self.init_imgui(context)
        context.window_manager.modal_handler_add(self)
        return {'RUNNING_MODAL'}

    def modal(self, context, event):
        context.area.tag_redraw()

        self._client.update()

        # Handle the event as you wish here, as in any modal operator
        if event.type in {'ESC'}:
            # Call shutdown_imgui() any time you'll return {'CANCELLED'} or {'FINISHED'}
            self.shutdown_imgui()
            return {'CANCELLED'}

        # Don't forget to call parent's modal:
        busy = self.modal_imgui(context, event)
        if busy:
            return {'RUNNING_MODAL'}
        else:
            return {'PASS_THROUGH'}

# -------------------------------------------------------------------

classes = (
    McRtcGUI,
)

register, unregister = bpy.utils.register_classes_factory(classes)
