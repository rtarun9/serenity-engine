function update_transform(scale, rotation, translation, delta_time, frame_count)

	frequency = 0.1

	rotation.z = rotation.z + delta_time * frequency
	rotation.y = rotation.y - delta_time * frequency
	return scale, rotation, translation
end