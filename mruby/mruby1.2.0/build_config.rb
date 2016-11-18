MRuby::Build.new do |conf|
  # load specific toolchain settings

  # Gets set by the VS command prompts.
  if ENV['VisualStudioVersion'] || ENV['VSINSTALLDIR']
    toolchain :visualcpp
  else
    toolchain :gcc
  end

  enable_debug

  # Use mrbgems
  # conf.gem 'examples/mrbgems/ruby_extension_example'
  # conf.gem 'examples/mrbgems/c_extension_example' do |g|
  #   g.cc.flags << '-g' # append cflags in this gem
  # end
  # conf.gem 'examples/mrbgems/c_and_ruby_extension_example'
  # conf.gem :github => 'masuidrive/mrbgems-example', :checksum_hash => '76518e8aecd131d047378448ac8055fa29d974a9'
  # conf.gem :git => 'git@github.com:masuidrive/mrbgems-example.git', :branch => 'master', :options => '-v'

  # include the default GEMs
  conf.gembox 'default'
  # C compiler settings
  # conf.cc do |cc|
  #   cc.command = ENV['CC'] || 'gcc'
  #   cc.flags = [ENV['CFLAGS'] || %w()]
  #   cc.include_paths = ["#{root}/include"]
  #   cc.defines = %w(DISABLE_GEMS)
  #   cc.option_include_path = '-I%s'
  #   cc.option_define = '-D%s'
  #   cc.compile_options = "%{flags} -MMD -o %{outfile} -c %{infile}"
  # end

  # mrbc settings
  # conf.mrbc do |mrbc|
  #   mrbc.compile_options = "-g -B%{funcname} -o-" # The -g option is required for line numbers
  # end

  # Linker settings
  # conf.linker do |linker|
  #   linker.command = ENV['LD'] || 'gcc'
  #   linker.flags = [ENV['LDFLAGS'] || []]
  #   linker.flags_before_libraries = []
  #   linker.libraries = %w()
  #   linker.flags_after_libraries = []
  #   linker.library_paths = []
  #   linker.option_library = '-l%s'
  #   linker.option_library_path = '-L%s'
  #   linker.link_options = "%{flags} -o %{outfile} %{objs} %{libs}"
  # end

  # Archiver settings
  # conf.archiver do |archiver|
  #   archiver.command = ENV['AR'] || 'ar'
  #   archiver.archive_options = 'rs %{outfile} %{objs}'
  # end

  # Parser generator settings
  # conf.yacc do |yacc|
  #   yacc.command = ENV['YACC'] || 'bison'
  #   yacc.compile_options = '-o %{outfile} %{infile}'
  # end

  # gperf settings
  # conf.gperf do |gperf|
  #   gperf.command = 'gperf'
  #   gperf.compile_options = '-L ANSI-C -C -p -j1 -i 1 -g -o -t -N mrb_reserved_word -k"1,3,$" %{infile} > %{outfile}'
  # end

  # file extensions
  # conf.exts do |exts|
  #   exts.object = '.o'
  #   exts.executable = '' # '.exe' if Windows
  #   exts.library = '.a'
  # end

  # file separetor
  # conf.file_separator = '/'

  # bintest
  # conf.enable_bintest
end

MRuby::Build.new('host-debug') do |conf|
  # load specific toolchain settings

  # Gets set by the VS command prompts.
  if ENV['VisualStudioVersion'] || ENV['VSINSTALLDIR']
    toolchain :visualcpp
  else
    toolchain :gcc
  end

  enable_debug

  # include the default GEMs
  conf.gembox 'default'

  # C compiler settings
  conf.cc.defines = %w(MRB_ENABLE_DEBUG_HOOK)

  # Generate mruby debugger command (require mruby-eval)
  conf.gem :core => "mruby-bin-debugger"

  # bintest
  # conf.enable_bintest
end

MRuby::Build.new('test') do |conf|
  # Gets set by the VS command prompts.
  if ENV['VisualStudioVersion'] || ENV['VSINSTALLDIR']
    toolchain :visualcpp
  else
    toolchain :gcc
  end

  enable_debug
  conf.enable_bintest
  conf.enable_test

  conf.gembox 'default'
end

MRuby::Build.new('bench') do |conf|
  # Gets set by the VS command prompts.
  if ENV['VisualStudioVersion'] || ENV['VSINSTALLDIR']
    toolchain :visualcpp
  else
    toolchain :gcc
    conf.cc.flags << '-O3'
  end

  conf.gembox 'default'
end

# Define cross build settings
# MRuby::CrossBuild.new('32bit') do |conf|
#   toolchain :gcc
#
#   conf.cc.flags << "-m32"
#   conf.linker.flags << "-m32"
#
#   conf.build_mrbtest_lib_only
#
#   conf.gem 'examples/mrbgems/c_and_ruby_extension_example'
#
#   conf.test_runner.command = 'env'
#
# end

# Cross build for Renesas RX630
MRuby::CrossBuild.new('rx630') do |conf|
   toolchain :rx630

#   conf.cc.flags << "-m32"
#   conf.linker.flags << "-m32"
#
#   conf.build_mrbtest_lib_only
#
   #conf.gem "#{root}/mrbgems/mruby-array-ext"
   ##conf.gem "#{root}/mrbgems/mruby-bin-debugger"
   ##conf.gem "#{root}/mrbgems/mruby-bin-mirb"
   ##conf.gem "#{root}/mrbgems/mruby-bin-mrbc"
   ##conf.gem "#{root}/mrbgems/mruby-bin-mruby"
   ##conf.gem "#{root}/mrbgems/mruby-bin-mruby-config"
   #conf.gem "#{root}/mrbgems/mruby-bin-strip"
   #conf.gem "#{root}/mrbgems/mruby-class-ext"
   #conf.gem "#{root}/mrbgems/mruby-compiler"
   #conf.gem "#{root}/mrbgems/mruby-enumerator"
   #conf.gem "#{root}/mrbgems/mruby-enum-ext"
   #conf.gem "#{root}/mrbgems/mruby-enum-lazy"
   #conf.gem "#{root}/mrbgems/mruby-error"
   #conf.gem "#{root}/mrbgems/mruby-eval"
   #conf.gem "#{root}/mrbgems/mruby-exit"
   #conf.gem "#{root}/mrbgems/mruby-fiber"
   #conf.gem "#{root}/mrbgems/mruby-hash-ext"
   #conf.gem "#{root}/mrbgems/mruby-inline-struct"
   #conf.gem "#{root}/mrbgems/mruby-kernel-ext"
   conf.gem "#{root}/mrbgems/mruby-math"
   conf.gem "#{root}/mrbgems/mruby-numeric-ext"  #10.chr
   #conf.gem "#{root}/mrbgems/mruby-object-ext"
   #conf.gem "#{root}/mrbgems/mruby-objectspace"
   #conf.gem "#{root}/mrbgems/mruby-print"
   #conf.gem "#{root}/mrbgems/mruby-proc-ext"
   #conf.gem "#{root}/mrbgems/mruby-random"
   #conf.gem "#{root}/mrbgems/mruby-range-ext"
   #conf.gem "#{root}/mrbgems/mruby-sprintf"
   #conf.gem "#{root}/mrbgems/mruby-string-ext"	#str.split(",")
   #conf.gem "#{root}/mrbgems/mruby-struct"
   #conf.gem "#{root}/mrbgems/mruby-symbol-ext"
   #conf.gem "#{root}/mrbgems/mruby-test"
   #conf.gem "#{root}/mrbgems/mruby-time"
   #conf.gem "#{root}/mrbgems/mruby-toplevel-ext"
#
end
