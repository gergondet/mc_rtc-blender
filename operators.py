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

def new_object_name(name):
    if not name in bpy.data.objects:
        return name
    i = 1
    while "{}.{:03d}".format(name, i) in bpy.data.objects:
        i += 1
    return "{}.{:03d}".format(name, i)

class InteractiveMarker(object):
    def __init__(self, collection, name, axis, callback):
        self._collection = collection
        self._callback = callback
        self._axis = axis
        self._ro = None
        self._position = imgui.PTransformd.Identity()

        bpy.ops.mesh.primitive_uv_sphere_add()
        self._sphere = bpy.context.selected_objects[0]
        self._sphere.name = new_object_name("{}_sphere".format(name))
        self._sphere.scale = [1e-6]*3

        bpy.ops.object.empty_add(type='ARROWS')
        self._empty = bpy.context.selected_objects[0]
        self._empty.name = new_object_name("{}_empty".format(name))

        self._sphere.select_set(True)
        self._empty.select_set(True)
        bpy.ops.collection.objects_remove_all()
        self._collection.objects.link(self._sphere)
        self._collection.objects.link(self._empty)

        bpy.context.view_layer.objects.active = self._sphere
        self._sphere.select_set(True)
        self._empty.select_set(True)
        bpy.ops.object.parent_set()
    def name(self):
        return self._sphere.name
    def _updateConstraints(self, ro):
        self._ro = ro
    def update(self, ro, pos):
        if self._ro != ro:
            self._updateConstraints(ro)
        self._position = pos
        self._sphere.location = pos.translation
        r = pos.rotation.inverse()
        self._sphere.rotation_mode = 'QUATERNION'
        self._sphere.rotation_quaternion[0] = r.w()
        self._sphere.rotation_quaternion[1] = r.x()
        self._sphere.rotation_quaternion[2] = r.y()
        self._sphere.rotation_quaternion[3] = r.z()

class BlenderInterface(imgui.Interface3D):
    def __init__(self):
        super().__init__()
        if 'mc_rtc' in bpy.data.collections:
            bpy.data.collections.remove(bpy.data.collections['mc_rtc'])
        self._collection = bpy.data.collections.new('mc_rtc')
        bpy.context.scene.collection.children.link(self._collection)
        self._markers = {}

    def __del__(self):
        super().__del__()
        bpy.data.collections.remove(self._collection)

    def _get_collection(self, name):
        return self._collection.children[name]

    def _fix_material(self, mesh, color):
        if len(mesh.material_slots) == 0:
            mat = bpy.data.materials.new(name = "{}_material".format(mesh.name))
            mat.diffuse_color = color
            bpy.context.view_layer.objects.active = mesh
            bpy.ops.object.material_slot_add()
            mesh.material_slots[0].material = mat
        else:
            for mat in [m.material for m in mesh.material_slots]:
                for node in mat.node_tree.nodes:
                    for i in node.inputs:
                        if i.name == "Alpha" and i.default_value == 0.0:
                            i.default_value = 1.0

    def add_collection(self, category, name):
        ncol = bpy.data.collections.new('/'.join(category + [name]))
        self._collection.children.link(ncol)
        return ncol.name

    def hide_collection(self, name, hide):
        self._get_collection(name).hide_viewport = hide

    def remove_collection(self, name):
        bpy.data.collections.remove(self._get_collection(name))

    def load_mesh(self, collection, meshPath, meshName, defaultColor):
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
        mesh.name = new_object_name(meshName)
        self._fix_material(mesh, defaultColor)
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

    def add_interactive_marker(self, category, name, axis, callback):
        collection = self.add_collection(category, name)
        marker = InteractiveMarker(self._get_collection(collection), name, axis, callback)
        self._markers[marker.name()] = marker
        return marker.name()

    def update_interactive_marker(self, name, ro, pos):
        if name not in self._markers:
            return
        self._markers[name].update(ro, pos)

    def remove_interactive_marker(self, name):
        if name not in self._markers:
            return
        bpy.data.collections.remove(self._markers[name]._collection)
        del self._markers[name]


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
