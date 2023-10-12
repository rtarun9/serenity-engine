function update_transform(scale, rotation, translation, delta_time, frame_count)

	scale.x = math.sin(frame_count / 180.0) * 5.0
	scale.y = math.cos(frame_count / 180.0) * 5.0
	scale.z = 2

	radius = 10
	frequency = 0.06

	translation.x = math.cos(frame_count * frequency) * radius
	translation.y = math.sin(frame_count * frequency) * radius

	return scale, rotation, translation
end