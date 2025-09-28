@tool
extends EditorPlugin

func _enter_tree():
	print("Lymo FFmpeg plugin loaded")

func _exit_tree():
	print("Lymo FFmpeg plugin unloaded")