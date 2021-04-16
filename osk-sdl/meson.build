project(
	'osk-sdl',
	'cpp',
	version : '0.63',
	default_options : ['warning_level=3', 'cpp_std=c++2a'],
	meson_version : '>=0.53.0',
)

add_project_arguments('-DVERSION="@0@"'.format(meson.project_version()), language : ['cpp'])

src = [
	'src/config.cpp',
	'src/draw_helpers.cpp',
	'src/keyboard.cpp',
	'src/main.cpp',
	'src/tooltip.cpp',
	'src/util.cpp',
]

sh = find_program('sh', native : true)

osk_sdl_exe = executable(
	'osk-sdl',
	src,
	dependencies : [
		dependency('SDL2'),
		dependency('SDL2_ttf')
	],
	install : true
)