if get_option('documentation').enabled() and not do_introspection
	error('Documentation can\'t be enabled if introspection won\'t be generated.')
endif

docgen = find_program('gi-docgen', required: get_option('documentation'))
if docgen.found()
	documentation_conf = configuration_data()
	documentation_conf.set('VERSION', meson.project_version())

	scidae_documentation_toml = configure_file(
		input: 'libscidae.toml.in',
		output: 'libscidae.toml',
		configuration: documentation_conf
	)

	custom_target('documentation',
		input: [scidae_documentation_toml, scidae_gir_t[0]],
		output: 'libscidae-@0@'.format(scidae_sover),
		command: [
			docgen,
			'generate',
			'--quiet',
			'--config=@INPUT0@',
			'--output-dir=@OUTPUT@',
			'--no-namespace-dir',
			'--content-dir=@0@'.format(meson.current_source_dir()),
			'--add-include-path=subprojects/pango2/pango2/',
			'@INPUT1@',
		],
		depend_files: [ 'scidae_urlmap.js' ],
		build_by_default: true,
		install: true,
		install_dir: get_option('datadir') / 'doc',
	)
endif