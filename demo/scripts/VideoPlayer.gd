extends Control

# Demo script showing how to use Lymo FFmpeg GDExtension
# for projection mapping applications

@onready var video_player: VideoStreamPlayer = $VideoStreamPlayer
@onready var file_dialog: FileDialog = $FileDialog
@onready var controls: VBoxContainer = $Controls
@onready var info_label: Label = $Controls/InfoLabel
@onready var play_button: Button = $Controls/PlayButton
@onready var pause_button: Button = $Controls/PauseButton
@onready var stop_button: Button = $Controls/StopButton
@onready var load_button: Button = $Controls/LoadButton
@onready var hw_accel_checkbox: CheckBox = $Controls/HardwareAcceleration
@onready var position_slider: HSlider = $Controls/PositionSlider

var ffmpeg_stream: FFmpegVideoStream
var ffmpeg_decoder: FFmpegDecoder
var is_seeking: bool = false

func _ready():
	# Create FFmpeg components
	ffmpeg_decoder = FFmpegDecoder.new()
	ffmpeg_stream = FFmpegVideoStream.new()
	
	# Connect signals
	play_button.pressed.connect(_on_play_pressed)
	pause_button.pressed.connect(_on_pause_pressed)
	stop_button.pressed.connect(_on_stop_pressed)
	load_button.pressed.connect(_on_load_pressed)
	hw_accel_checkbox.toggled.connect(_on_hw_accel_toggled)
	position_slider.value_changed.connect(_on_position_changed)
	file_dialog.file_selected.connect(_on_file_selected)
	
	# Setup file dialog
	file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	file_dialog.add_filter("*.mp4", "MP4 Video")
	file_dialog.add_filter("*.avi", "AVI Video")
	file_dialog.add_filter("*.mkv", "MKV Video")
	file_dialog.add_filter("*.mov", "MOV Video")
	file_dialog.add_filter("*.webm", "WebM Video")
	
	# Initialize hardware acceleration checkbox
	hw_accel_checkbox.button_pressed = ffmpeg_decoder.get_use_hardware_acceleration()
	
	# Update info display
	_update_info_display()

func _process(_delta):
	if video_player.is_playing() and not is_seeking:
		var position = video_player.get_stream_position()
		var duration = video_player.get_stream_length()
		
		if duration > 0:
			position_slider.value = (position / duration) * 100.0
		
		_update_info_display()

func _on_play_pressed():
	if ffmpeg_stream:
		video_player.stream = ffmpeg_stream
		video_player.play()

func _on_pause_pressed():
	video_player.paused = !video_player.paused

func _on_stop_pressed():
	video_player.stop()

func _on_load_pressed():
	file_dialog.popup_centered(Vector2i(800, 600))

func _on_hw_accel_toggled(enabled: bool):
	if ffmpeg_decoder:
		ffmpeg_decoder.set_use_hardware_acceleration(enabled)

func _on_position_changed(value: float):
	if not video_player.is_playing():
		return
		
	is_seeking = true
	var duration = video_player.get_stream_length()
	if duration > 0:
		var seek_time = (value / 100.0) * duration
		video_player.stream_position = seek_time
	is_seeking = false

func _on_file_selected(path: String):
	print("Loading video file: ", path)
	
	# Close previous file
	if ffmpeg_decoder.is_file_open():
		ffmpeg_decoder.close()
	
	# Open new file
	if ffmpeg_decoder.open_file(path):
		ffmpeg_stream.set_file(path)
		_update_info_display()
		print("Video loaded successfully")
		
		# Show available hardware decoders
		var hw_decoders = ffmpeg_decoder.get_available_hw_decoders()
		if hw_decoders.size() > 0:
			print("Available hardware decoders: ", hw_decoders)
		print("Current decoder: ", ffmpeg_decoder.get_current_hw_decoder())
	else:
		print("Failed to load video file")

func _update_info_display():
	if not ffmpeg_decoder or not ffmpeg_decoder.is_file_open():
		info_label.text = "No video loaded"
		return
	
	var info_text = ""
	info_text += "Resolution: %dx%d\n" % [ffmpeg_decoder.get_width(), ffmpeg_decoder.get_height()]
	info_text += "Frame Rate: %.2f fps\n" % ffmpeg_decoder.get_frame_rate()
	info_text += "Duration: %.2f seconds\n" % ffmpeg_decoder.get_duration()
	info_text += "Pixel Format: %s\n" % ffmpeg_decoder.get_pixel_format_name()
	info_text += "Has Alpha: %s\n" % ("Yes" if ffmpeg_decoder.get_has_alpha() else "No")
	info_text += "Hardware Decoder: %s\n" % ffmpeg_decoder.get_current_hw_decoder()
	
	if video_player.is_playing():
		info_text += "Status: Playing"
		if video_player.paused:
			info_text += " (Paused)"
	else:
		info_text += "Status: Stopped"
	
	info_label.text = info_text