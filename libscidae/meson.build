pkgconf = import('pkgconfig')
gir_scanner = find_program('g-ir-scanner', required: get_option('introspection'))
do_introspection = gir_scanner.found() and (not meson.is_cross_build() or get_option('introspection').enabled())

subdir('data')

scidae_src = [
	'scidaecontext.c',
	'scidaecanvas.c',
	'scidaeutil.c',
	[
		'widget/scidaewidget.c',
		'widget/scidaecontainer.c',
		'widget/scidaetoplevel.c',
		'widget/scidaeparagraph.c',
		'widget/scidaetext.c'
	],
	[
		'pango/scidaepangocontext.c',
		'pango/scidaepangotext.c'
	]
]

inc_dirs = [ include_directories('.') ]

sciade_headers = [
	'scidae.h',
	'scidaecontext.h',
	'scidaecanvas.h',
	'scidaeutil.h',
	[
		'widget/scidaewidget.h',
		'widget/scidaewidget-alt.h',
		'widget/scidaecontainer.h',
		'widget/scidaetoplevel.h',
		'widget/scidaeparagraph.h',
		'widget/scidaetext.h'
	],
	[
		'pango/scidaepangocontext.h',
		'pango/scidaepangotext.h'
	]
]

pango2 = subproject('pango2')
inc_dirs += pango2.get_variable('root_inc')

scidae_public_deps = [
	dependency('glib-2.0'),
	dependency('gobject-2.0'),
	dependency('gio-2.0'),
	dependency('gtk4'),
	pango2.get_variable('libpango_dep'),
	scidae_data_dep
]

scidae_lib = library('scidae', scidae_src,
	dependencies: [
		scidae_public_deps
	],
	c_args: ['-DG_LOG_DOMAIN="Scidae"', '-Wno-pedantic'],
	include_directories: inc_dirs,
	#version: meson.project_version(),
	#soversion: scidae_sover,
	install: true
)

scidae_dep = declare_dependency(
	include_directories: inc_dirs,
	link_with: scidae_lib,
	dependencies: scidae_public_deps
)

install_headers(sciade_headers, subdir: 'libscidae')

pkgconf.generate(
	name: meson.project_name(),
	filebase: 'libscidae',
	description : 'Meson sample project.',
	subdirs: 'libscidae',
	libraries: scidae_lib,
	version: meson.project_version()
)

if do_introspection
	scidae_gir_t = gnome.generate_gir(scidae_lib,
		dependencies: scidae_public_deps,
		sources: [scidae_src, sciade_headers],
		namespace: 'Scidae',
		nsversion: '@0@'.format(scidae_sover),
		includes: ['GLib-2.0', 'GObject-2.0', 'Gio-2.0', 'Gtk-4.0', 'Pango2-1.0', scidae_data_gir_t[0]],
		include_directories: [pango2.get_variable('pango_inc')],
		header: 'scidae.h',
		install: true,
		symbol_prefix: 'scidae'
	)
endif

subdir('docs')