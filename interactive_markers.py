import bpy
import math
import mathutils
from bpy.types import (
    GizmoGroup,
    Gizmo
)

from .mc_rtc_blender import PTransformd, Quaterniond, ControlAxis

class InteractiveMarker:
    def __init__(self, group, object, axis):
        self._object = object
        self._gizmos = []
        self._group = group
        if axis & ControlAxis.RX:
            self.add_rotate_gizmo([1, 0, 0], mathutils.Matrix.Rotation(math.radians(90.0), 4, 'Y'))
        if axis & ControlAxis.RY:
            self.add_rotate_gizmo([0, 1, 0], mathutils.Matrix.Rotation(math.radians(-90.0), 4, 'X'))
        if axis & ControlAxis.RZ:
            self.add_rotate_gizmo([0, 0, 1])
        if axis & ControlAxis.TX:
            self.add_translate_gizmo([1, 0, 0], mathutils.Matrix.Rotation(math.radians(90.0), 4, 'Y'))
        if axis & ControlAxis.TY:
            self.add_translate_gizmo([0, 1, 0], mathutils.Matrix.Rotation(math.radians(-90.0), 4, 'X'))
        if axis & ControlAxis.TZ:
            self.add_translate_gizmo([0, 0, 1])
        self._previous_pose = self._object.matrix_world.normalized()
        self._busy = False
        self._release_pose = None

    def __del__(self):
        for gz in self._gizmos:
            self._group.gizmos.remove(gz)

    def _add_gizmo(self, gztype, operator, axis, offset):
        gz = self._group.gizmos.new(gztype)
        props = gz.target_set_operator(operator)
        props.constraint_axis = [bool(a) for a in axis]
        props.orient_type = 'LOCAL'
        props.release_confirm = True
        gz.matrix_basis = self._object.matrix_world.normalized()
        gz.line_width = 3
        gz.color = ([ 0.8 * x for x in axis])
        gz.alpha = 0.5
        gz.color_highlight = axis
        gz.alpha_highlight = 1.0
        gz.matrix_offset = offset
        gz.use_grab_cursor = True
        gz.use_draw_modal = True
        gz.scale_basis = 0.125
        self._gizmos.append(gz)

    def read_only(self, ro):
        for gz in self._gizmos:
            gz.hide_select = ro

    def add_translate_gizmo(self, axis, offset = mathutils.Matrix.Identity(4)):
        self._add_gizmo("GIZMO_GT_arrow_3d", "transform.translate", axis, offset)

    def add_rotate_gizmo(self, axis, offset = mathutils.Matrix.Identity(4)):
        self._add_gizmo("GIZMO_GT_dial_3d", "transform.rotate", axis, offset)
        self._gizmos[-1].draw_options = {'CLIP'}

    def name(self):
        return self._object.name

    def busy(self):
        if self._busy:
            return True
        self._busy = any([gz.is_modal for gz in self._gizmos])
        return self._busy

    def release_pose(self):
        if self._release_pose:
            pos = self._release_pose
            self._release_pose = None
            self._busy = False
            return pos
        return self._release_pose

    def update(self, matrix):
        self._previous_pose = matrix
        for gz in self._gizmos:
            gz.matrix_basis = matrix

    def refresh(self):
        pos = self._object.matrix_world.normalized()
        if self._previous_pose != pos:
            self._release_pose = PTransformd.Identity()
            self._release_pose.translation = pos.to_translation()
            q = pos.to_quaternion().inverted()
            self._release_pose.rotation = Quaterniond(q.w, q.x, q.y, q.z)
        else:
            self._busy = False
        self.update(pos)

    def is_highlight(self):
        return any([gz.is_highlight for gz in self._gizmos])

class InteractiveMarkers(GizmoGroup):
    bl_idname = "OBJECT_GGT_interactive_markers"
    bl_label = "Display persistent interactive markers in the scene"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'WINDOW'
    bl_options = {'3D', 'SCALE', 'PERSISTENT'}
    all_gizmos = {}
    active_object = None
    instance = None

    @classmethod
    def poll(cls, context):
        cls.active_object = None
        for gz in cls.all_gizmos.values():
            if gz.is_highlight():
                cls.active_object = gz._object
        return True

    @classmethod
    def update(cls):
        marker = cls.active_object
        if marker and bpy.context.view_layer.objects.active != marker:
            for obj in bpy.context.selected_objects:
                obj.select_set(False)
            marker.select_set(True)
            bpy.context.view_layer.objects.active = marker

    @classmethod
    def add_marker(cls, obj, axis):
        if not obj in cls.all_gizmos:
            cls.all_gizmos[obj] = InteractiveMarker(cls.instance, obj, axis)
        return cls.all_gizmos[obj]

    @classmethod
    def remove_marker(cls, obj):
        if obj in cls.all_gizmos:
            del cls.all_gizmos[obj]

    def setup(self, context):
        InteractiveMarkers.instance = self
        #for obj in bpy.data.objects:
        #    if obj.type == 'MESH' and not obj in InteractiveMarkers.all_gizmos:
        #        InteractiveMarkers.all_gizmos[obj] = InteractiveMarker(self, obj)

    def refresh(self, context):
        ob = context.object
        if ob and ob in InteractiveMarkers.all_gizmos:
            InteractiveMarkers.all_gizmos[ob].refresh()
