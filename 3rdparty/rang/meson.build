project('rang', 'cpp', version : '3.2.0',
		default_options : ['cpp_std=c++11'])

inc = include_directories('include')

rang_dep = declare_dependency(include_directories:inc)

doctest = dependency('doctest', required: false)
if doctest.found()
  subdir('test')
endif

cppcheck = find_program('cppcheck', required: false)
if cppcheck.found()
  run_target('cppcheck', command : ['cppcheck', '--project=' +
			 join_paths(meson.build_root(), 'compile_commands.json')])
endif
