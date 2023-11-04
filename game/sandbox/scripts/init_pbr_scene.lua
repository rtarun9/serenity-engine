-- Setup the scene game objects and params that will be parsed and used to setup the scene on the C++ side.

-- The key in table (i.e lua's map) is the name of gameobject, and the value is of the form:
-- File Path, Scale, Rotation, Translation, Script (a table that can optionally have name and path (either both or none)).
game_objects = {
	cube = {
		model_path= "data/Cube/glTF/Cube.gltf",
		scale = {x = 1.0, y = 1.0, z = 1.0},
		rotation = {x = 0.0, y = 0.0, z = 0.0},
		translation = {x = 1.0, y = 1.0, z = 1.0},
		script = {
			name = "circle",
			path = "game/sandbox/scripts/test.lua"
		}
	},
	
	cube2 = {
		model_path= "data/Cube/glTF/Cube.gltf",
		scale = {x = 1.0, y = 1.0, z = 1.0},
		rotation = {x = 0.0, y = 0.0, z = 0.0},
		translation = {x = 1.0, y = 1.0, z = 1.0},
		script = {
			name = "scale_and_circle",
			path = "game/sandbox/scripts/test2.lua"
		}
	},
	
	pbr_spheres= {
		model_path= "data/sketchfab_pbr_material_reference_chart/scene.gltf",
		scale = {x = 1.0, y = 1.0, z = 1.0},
		rotation = {x = 0.0, y = 0.0, z = 0.0},
		translation = {x = 1.0, y = 0.0, z = -3.0},
		script = {
		}
	},

	flying_world= {
		model_path= "data/flying_world/scene.gltf",
		scale = {x = 0.1, y = 0.1, z = 0.1},
		rotation = {x = 0.0, y = 90.0, z = 0.0},
		translation = {x = 0.0, y = 0.0, z = 0.0},
		script = {
		}
	},

}