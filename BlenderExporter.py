import json
import os
import bpy
from mathutils import Vector

bl_info = {
    "name": "Tempest Exporter",
    "blender": (2, 80, 0),
    "category": "Object",
}   
    
    
def get_base_mesh_name(name):
    sep = '.'
    return name.split(sep, 1)[0]

def export_mesh_to_gltf(obj, name):
    basedir = bpy.path.abspath('//')
    outputPath = os.path.join(basedir + 'Meshes', name)
    if not os.path.exists(outputPath + '.glb'):
        obj.select_set(True)
        old_location = obj.location
        obj.location = [0, 0, 0]

        bpy.ops.export_scene.gltf(
                    filepath=outputPath,
                    export_materials=False,
                    export_selected=True,
                    export_yup=True)
        obj.location = old_location
        obj.select_set(False)
    

    meshEntry = {}
    meshEntry["Path"] = 'Meshes/' + name + '.glb'
    if obj.rigid_body.enabled:
        meshEntry["Dynamism"] = "Dynamic"
    else:
        meshEntry["Dynamism"] = "Static"
    return meshEntry

def create_mesh_collider(obj):
    rigidBody = obj.rigid_body
    
    collider = {}
    if rigidBody.collision_shape == 'BOX':
        collider['GEOMETRY'] = 'Box'
    elif rigidBody.collision_shape == 'CAPSULE':
        collider['GEOMETRY'] = 'Capsule'
    elif rigidBody.collision_shape == 'SPHERE':
        collider['GEOMETRY'] = 'Sphere'
    elif rigidBody.collision_shape == 'MESH':
        collider['GEOMETRY'] = 'Mesh'
        
    collider['Mass'] = rigidBody.mass

    if rigidBody.enabled:
        collider["Type"] = "Dynamic"
    else:
        collider["Type"] = "Static"
        
    return collider

def create_mesh_instance(obj, assetName):
    instanceEntry = {}
    instanceEntry["Asset"] = assetName
    instanceEntry["Scale"] = [obj.scale[0], obj.scale[2], obj.scale[1]]
    instanceEntry["Rotation"] = [obj.rotation_quaternion[1], obj.rotation_quaternion[3], obj.rotation_quaternion[2], obj.rotation_quaternion[0]]
    instanceEntry["Position"] = [obj.location[0], obj.location[2], obj.location[1]]
    
    matSlot = obj.material_slots[0]
    instanceEntry["Material"] = matSlot.name
    
    instanceEntry["Collider"] = create_mesh_collider(obj)
    
    scriptsEntry = {}
    if "GamePlay" in bpy.data.objects[obj.name]:
        scriptsEntry["GamePlay"] = bpy.data.objects[obj.name]["GamePlay"]
        instanceEntry["Scripts"] = scriptsEntry       
    
    return instanceEntry
    
def create_camera_entry(obj):
    cameraEntry = {}
    
    camera = obj.data
    
    cameraEntry["Position"] = [obj.location[0], obj.location[2], obj.location[1]]
    
    worldMatrix = obj.matrix_world
    cam_direction = worldMatrix.to_quaternion() @ Vector((0.0, 0.0, -1.0))
    cameraEntry['Direction'] = [cam_direction[0], cam_direction[2], cam_direction[1]]
    
    if camera.type == 'PERSP':
        cameraEntry['Mode'] = 'InfinitePerspective'
    else:
        cameraEntry['Mode'] = 'Orthographic'
        cameraEntry["OrthoSize"] = [camera.ortho_scale, camera.ortho_scale]
    
    cameraEntry['FOV'] = (camera.angle * 180) / 3.1415
        
    cameraEntry["NearPlane"] = camera.clip_start
    cameraEntry["FarPlane"] = camera.clip_end
    
    return cameraEntry
    
def create_light_enry(obj):
    lightEntry = {}
    light = obj.data
    
    lightEntry["Scale"] = [obj.scale[0], obj.scale[2], obj.scale[1]]
    lightEntry["Rotation"] = [obj.rotation_quaternion[1], obj.rotation_quaternion[3], obj.rotation_quaternion[2], obj.rotation_quaternion[0]]
    lightEntry["Position"] = [obj.location[0], obj.location[2], obj.location[1]]
    
    if light.type == 'POINT':
        lightEntry["Type"] = "Point"
    elif light.type == 'SPOT':
        lightEntry["Type"] = "Spot"
    elif light.type == 'AREA':
        lightEntry["Type"] = "Area"
        
    lightEntry["Colour"] = [light.color[0], light.color[1], light.color[2]]
    lightEntry["FallOff"] = light.cutoff_distance;
    lightEntry["Intensity"] = light.specular_factor
    
    return lightEntry

def create_material_entry(obj):
    matSlot = obj.material_slots[0]
    materialName = matSlot.name

    materialEntry = {}
    textures = []
    textures.extend([x for x in matSlot.material.node_tree.nodes if x.type=='TEX_IMAGE'])
                    
    for t in textures:
        texturePath = t.image.filepath
        texturePath = texturePath.strip('\\')
        texturePath = texturePath.strip('/')
        if '_col' in texturePath:
            materialEntry["Albedo"] = texturePath
        elif '_mtl' in texturePath:
            materialEntry["Metalness"] = texturePath
        elif '_nrm' in texturePath:
            materialEntry["Normal"] = texturePath
        elif '_rgh' in texturePath:
            materialEntry["Roughness"] = texturePath
        elif '_occ' in texturePath:
            materialEntry["Occlusion"] = texturePath
        elif '_em' in texturePath:
            materialEntry["Emissive"] = texturePath

    return materialName, materialEntry

def create_export_dirs(baseDir):
    if not os.path.exists(baseDir + '/Meshes'):
        os.makedirs(baseDir + '/Meshes')

class ExportTemptestScene(bpy.types.Operator):
    """Tempest export script"""      # Use this as a tooltip for menu items and buttons.
    bl_idname = "tempest.tempest_export"        # Unique identifier for buttons and menu items to reference.
    bl_label = "Export to tempest scene file"         # Display name in the interface.
    bl_options = {'REGISTER'}  # Enable undo for the operator.

    def execute(self, context):        # execute() is called when running the operator.

        baseDir = bpy.path.abspath('//')
        sceneFilePath = baseDir + 'scene.json'

        globals = {}
        meshes = {}
        materials = {}
        instances = {}
        cameras = {}
        lights = {}
        scripts = {}

        if os.path.exists(sceneFilePath):
            with open(sceneFilePath) as sceneFile:
                sceneData = json.load(sceneFile)
                if 'GLOBALS' in sceneData:
                    globals = sceneData['GLOBALS']
        
        create_export_dirs(baseDir)

        scene = context.scene
        for obj in scene.objects:
            if obj.type == 'MESH':
                baseName = get_base_mesh_name(obj.name)
                if not(baseName in meshes):
                    meshes[baseName] = export_mesh_to_gltf(obj, baseName)
                instances[obj.name] = create_mesh_instance(obj, baseName)
                
                materialName, material = create_material_entry(obj)
                materials[materialName] = material
                
                if "GamePlay" in bpy.data.objects[obj.name]:
                    scripts[bpy.data.objects[obj.name]["GamePlay"]] = 'Scripts/' + bpy.data.objects[obj.name]["GamePlay"] + '.lua'  
                
            if obj.type == 'CAMERA':
                cameras[obj.name] = create_camera_entry(obj)
                
            if obj.type == 'LIGHT':
                lights[obj.name] = create_light_enry(obj)
                

        root = {}
        root['GLOBALS'] = globals
        root['MESH'] = meshes
        root['INSTANCE'] = instances
        root['CAMERA'] = cameras
        root['LIGHT'] = lights
        root['SCRIPTS'] = scripts
        root['MATERIALS'] = materials
        
        with open(sceneFilePath, 'w+') as outfile:
            json.dump(root, outfile, indent=4)
        

        return {'FINISHED'}    
    
    
    
def register():
    bpy.utils.register_class(ExportTemptestScene)


def unregister():
    bpy.utils.unregister_class(ExportTemptestScene)
