scidae_data_src = [
	'scidaedataid.c',
	'scidaewordnode.c',
	'scidaerevision.c',
	'scidaerevisioninsert.c',
	'scidaerevisiondelete.c'
]

scidae_data_headers = [
	'scidaedata.h',
	'scidaedataid.h',
	'scidaewordnode.h',
	'scidaerevision.h',
	'scidaerevisioninsert.h',
	'scidaerevisiondelete.h'
]

scidae_data_inc_dirs = [ include_directories('.') ]

scidae_data_public_deps = [
	dependency('glib-2.0'),
	dependency('gobject-2.0')
]

scidae_data_lib = library('scidaedata', scidae_data_src,
	dependencies: [
		scidae_data_public_deps
	],
	c_args: ['-DG_LOG_DOMAIN="ScidaeData"', '-Wno-pedantic'],
	include_directories: scidae_data_inc_dirs,
	#version: meson.project_version(),
	#soversion: scidae_sover,
	install: true
)

scidae_data_dep = declare_dependency(
	include_directories: scidae_data_inc_dirs,
	link_with: scidae_data_lib,
	dependencies: scidae_data_public_deps
)

install_headers(scidae_data_headers, subdir: 'libscidae/data')

pkgconf.generate(
	name: meson.project_name(),
	filebase: 'libscidae-data',
	description : 'Meson sample project.',
	subdirs: 'libscidae/data',
	libraries: scidae_data_lib,
	version: meson.project_version()
)

if do_introspection
	scidae_data_gir_t = gnome.generate_gir(scidae_data_lib,
		dependencies: scidae_data_public_deps,
		sources: [scidae_data_src, scidae_data_headers],
		namespace: 'ScidaeData',
		identifier_prefix: 'Scidae',
		nsversion: '@0@'.format(scidae_sover),
		includes: ['GLib-2.0', 'GObject-2.0'],
		header: 'scidaedata.h',
		install: true,
		symbol_prefix: 'scidae'
	)
endif