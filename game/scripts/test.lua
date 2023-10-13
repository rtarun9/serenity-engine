function update_transform(scale, rotation, translation, delta_time, frame_count)

	radius = 10
	frequency = 0.03

	scale.x = 25

	-- translation.x = math.cos(frame_count * frequency) * radius
	-- translation.y = math.sin(frame_count * frequency) * radius

	return scale, rotation, translation
end