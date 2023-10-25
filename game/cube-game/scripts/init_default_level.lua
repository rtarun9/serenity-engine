-- Setup the scene game objects and params that will be parsed and used to setup the scene on the C++ side.

-- The key in table (i.e lua's map) is the name of gameobject, and the value is of the form:
-- File Path, Scale, Rotation, Translation, Script (a table that can optionally have name and path (either both or none)).
game_objects = {
	player = {
		model_path= "data/Cube/glTF/Cube.gltf",
		scale = {x = 1.0, y = 1.0, z = 1.0},
		rotation = {x = 0.0, y = 0.0, z = 0.0},
		translation = {x = 1.0, y = 10.0, z = -60.0},
		script = {
		}
	},

	obstacle_1 = {
		model_path= "data/Cube/glTF/Cube.gltf",
		scale = {x = 1.0, y = 1.0, z = 1.0},
		rotation = {x = 0.0, y = 0.0, z = 0.0},
		translation = {x = 0.0, y = 1.0, z = 10.0},
		script = {
			name = "horizontal_movement",
			path="game/cube-game/scripts/obstacle_horizontal_movement.lua"
		}
	},

	obstacle_2 = {
		model_path= "data/Cube/glTF/Cube.gltf",
		scale = {x = 1.0, y = 1.0, z = 1.0},
		rotation = {x = 0.0, y = 0.0, z = 0.0},
		translation = {x = 0.0, y = 1.0, z = 60.0},
		script = {
			name = "horizontal_movement_fast",
			path="game/cube-game/scripts/obstacle_horizontal_movement_fast.lua"
		}
	},

	obstacle_3 = {
		model_path= "data/Cube/glTF/Cube.gltf",
		scale = {x = 1.0, y = 1.0, z = 1.0},
		rotation = {x = 0.0, y = 0.0, z = 0.0},
		translation = {x = 0.0, y = 1.0, z = 120.0},
		script = {
			name = "circular_movement",
			path="game/cube-game/scripts/obstacle_circular_movement.lua"
		}
	},

	obstacle_4 = {
		model_path= "data/Cube/glTF/Cube.gltf",
		scale = {x = 1.0, y = 1.0, z = 1.0},
		rotation = {x = 0.0, y = 0.0, z = 0.0},
		translation = {x = 0.0, y = 1.0, z = 150.0},
		script = {
			name = "horizontal_scale",
			path="game/cube-game/scripts/obstacle_horizontal_scale.lua"
		}
	},

	obstacle_5 = {
		model_path= "data/Cube/glTF/Cube.gltf",
		scale = {x = 6.0, y = 2.0, z = 7.0},
		rotation = {x = 0.0, y = 0.0, z = 0.0},
		translation = {x = 0.0, y = 1.0, z = 220.0},
		script = {
			name = "rotator",
			path="game/cube-game/scripts/obstacle_rotator.lua"
		}
	},

	floor = {
		model_path= "data/Cube/glTF/Cube.gltf",
		scale = {x = 10.0, y = 1.0, z = 200.0},
		rotation = {x = 0.0, y = 0.0, z = 0.0},
		translation = {x = 1.0, y = -2.0, z = 170.0},
		script = {
		}
	},
}