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

import mathutils

from .blender_imgui import ImguiBasedOperator, imgui
from .interactive_markers import InteractiveMarkers

from math import cos, sin, pi

import hashlib
import os.path

# -------------------------------------------------------------------

def file_hash(filename):
    sha256 = hashlib.sha256()
    with open(filename, "rb") as f:
        for block in iter(lambda: f.read(4096), b""):
            sha256.update(block)
    return sha256.hexdigest()

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
        self._sphere.name = new_object_name("{}_marker".format(name))
        self._sphere.scale = [1e-6]*3

        self._sphere.select_set(True)
        bpy.ops.collection.objects_remove_all()
        self._collection.objects.link(self._sphere)

        self._gizmo = InteractiveMarkers.add_marker(self._sphere, self._axis)
    def name(self):
        return self._sphere.name
    def _updateConstraints(self, ro):
        self._ro = ro
        self._gizmo.read_only(self._ro)
    def update(self, ro, pos):
        if self._ro != ro:
            self._updateConstraints(ro)
        if self._gizmo.busy():
            npos = self._gizmo.release_pose()
            if npos is None:
                return
            pos = npos
            self._callback(pos)
        self._position = pos
        self._sphere.location = pos.translation
        r = pos.rotation.inverse()
        self._sphere.rotation_mode = 'QUATERNION'
        self._sphere.rotation_quaternion[0] = r.w()
        self._sphere.rotation_quaternion[1] = r.x()
        self._sphere.rotation_quaternion[2] = r.y()
        self._sphere.rotation_quaternion[3] = r.z()
        self._gizmo.update(self._sphere.matrix_world.normalized())
    def remove(self):
        InteractiveMarkers.remove_marker(self._sphere)
        bpy.data.collections.remove(self._collection)
    def hidden(self, hidden):
        self._gizmo.hidden(hidden)

class Arrow(object):
    def __init__(self, collection):
        self._collection = collection

        bpy.ops.mesh.primitive_cylinder_add(radius = 1, depth = 1)
        self._shaft = bpy.context.selected_objects[0]
        self._shaft.name = new_object_name("arrow_shaft")

        bpy.ops.mesh.primitive_cone_add(radius1 = 1, radius2 = 0, depth = 1)
        self._head = bpy.context.selected_objects[0]
        self._head.name = new_object_name("arrow_head")

        self._material = bpy.data.materials.new(name = "{}_material".format(self._shaft.name))
        bpy.context.view_layer.objects.active = self._shaft
        bpy.ops.object.material_slot_add()
        self._shaft.material_slots[0].material = self._material
        bpy.context.view_layer.objects.active = self._head
        bpy.ops.object.material_slot_add()
        self._head.material_slots[0].material = self._material

        self._shaft.select_set(True)
        self._head.select_set(True)
        bpy.ops.collection.objects_remove_all()
        self._collection.objects.link(self._shaft)
        self._collection.objects.link(self._head)
        self._shaft.select_set(False)
        self._head.select_set(False)
    def __del__(self):
        bpy.data.collections.remove(self._collection)
    def name(self):
        return self._shaft.name
    def update(self, startIn, endIn, shaft_diam, head_diam, head_len, color):
        start = mathutils.Vector(startIn)
        end = mathutils.Vector(endIn)
        normal = end - start
        height = normal.length
        if height != 0:
            normal = normal / height;
        if head_len >= height:
            head_len = height
        shaft_len = height - head_len
        theta = normal.angle([0, 0, 1])
        axis = normal.cross([0, 0, 1])
        if axis.length < 1e-6:
            axis = mathutils.Vector([1, 0, 0])
        axis.normalize()
        if shaft_len != 0 and shaft_diam != 0:
            self._shaft.hide_set(False)
            r = shaft_diam / 2
            self._shaft.matrix_world = mathutils.Matrix.Translation(start + 0.5 * shaft_len * normal) @ \
                                       mathutils.Matrix.Rotation(-theta, 4, axis) @ \
                                       mathutils.Matrix.Diagonal([r, r, shaft_len, 1.0])
        else:
            self._shaft.hide_set(True)
        if head_len != 0 and head_diam != 0:
            self._head.hide_set(False)
            r = head_diam / 2
            self._head.matrix_world = mathutils.Matrix.Translation(start + (shaft_len + 0.5 * head_len) * normal) @ \
                                      mathutils.Matrix.Rotation(-theta, 4, axis) @ \
                                      mathutils.Matrix.Diagonal([r, r, head_len, 1.0])
        else:
            self._head.hide_set(True)
        self._material.diffuse_color = color

class BlenderInterface(imgui.Interface3D):
    def __init__(self):
        super().__init__()
        if 'mc_rtc' in bpy.data.collections:
            bpy.data.collections.remove(bpy.data.collections['mc_rtc'])
        self._collection = bpy.data.collections.new('mc_rtc')
        bpy.context.scene.collection.children.link(self._collection)
        self._meshes = {}
        self._meshes_hash = {}
        self._markers = {}
        self._arrows = {}
        # Set some saner default for visualization
        bpy.context.space_data.shading.color_type = 'TEXTURE'
        if 'Cube' in bpy.data.objects:
            bpy.data.objects.remove(bpy.data.objects['Cube'])

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

    def _copy_mesh(self, collection, meshPath, meshName):
        mesh = self._meshes[meshPath].copy()
        self._get_collection(collection).objects.link(mesh)
        mesh.name = new_object_name(meshName)
        return mesh.name

    def load_mesh(self, collection, meshPath, meshName, defaultColor):
        if not os.path.exists(meshPath):
            return ""
        mesh_hash = file_hash(meshPath)
        if meshPath in self._meshes and mesh_hash == self._meshes_hash[meshPath]:
            return self._copy_mesh(collection, meshPath, meshName)
        self._meshes_hash[meshPath] = mesh_hash
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
        self._meshes[meshPath] = mesh
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

    def set_marker_hidden(self, name, hidden):
        if name not in self._markers:
            return
        self._markers[name].hidden(hidden)

    def remove_interactive_marker(self, name):
        if name not in self._markers:
            return
        self._markers[name].remove()
        del self._markers[name]

    def add_arrow(self, category, name):
        collection = self.add_collection(category, name)
        arrow = Arrow(self._get_collection(collection))
        self._arrows[arrow.name()] = arrow
        return arrow.name()

    def update_arrow(self, name, start, end, shaft_diam, head_diam, head_len, color):
        if name not in self._arrows:
            return
        self._arrows[name].update(start, end, shaft_diam, head_diam, head_len, color)

    def remove_arrow(self, name):
        if name not in self._arrows:
            return
        del self._arrows[name]


class McRtcGUI(Operator,ImguiBasedOperator):
    """mc_rtc GUI inside Blender"""
    bl_idname = "object.mc_rtc_gui"
    bl_label = "mc_rtc GUI"

    def __init__(self):
        super().__init__()
        self._timer = None
        self._iface = BlenderInterface()
        self._client = imgui.Client(self._iface)
        self._client.connect("ipc:///tmp/mc_rtc_pub.ipc", "ipc:///tmp/mc_rtc_rep.ipc")
        self._client.timeout(1.0)

    def __del__(self):
        super().__del__()

    def draw(self, context):
        self._client.draw2D()
        self._client.draw3D()

    def invoke(self, context, event):
        # Call init_imgui() at the beginning
        self.init_imgui(context)
        context.window_manager.modal_handler_add(self)
        self._timer = context.window_manager.event_timer_add(1.0 / 60.0, window = context.window)
        return {'RUNNING_MODAL'}

    def cancel(self, context):
        if self._timer:
            wm = context.window_manager
            wm.event_timer_remove(self._timer)

    def modal(self, context, event):
        # We need to do this because of https://developer.blender.org/T77419
        area = context.area
        if context.area is None:
            for w in context.window_manager.windows:
                for a in w.screen.areas:
                    if a.type == 'VIEW_3D':
                        area = a
        if area is None:
            return {'PASS_THROUGH'}
        area.tag_redraw()

        self._client.update()
        InteractiveMarkers.update()

        # Handle the event as you wish here, as in any modal operator
        if False:
            # Call shutdown_imgui() any time you'll return {'CANCELLED'} or {'FINISHED'}
            self.shutdown_imgui()
            return {'CANCELLED'}

        # Don't forget to call parent's modal:
        busy = self.modal_imgui(area.regions[-1], event)
        if busy:
            return {'RUNNING_MODAL'}
        else:
            return {'PASS_THROUGH'}

# -------------------------------------------------------------------

classes = (
    InteractiveMarkers,
    McRtcGUI
)

register, unregister = bpy.utils.register_classes_factory(classes)
