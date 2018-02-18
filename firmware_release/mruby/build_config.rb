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
  # conf.gem :core => 'mruby-eval'
  # conf.gem :mgem => 'mruby-io'
  # conf.gem :github => 'iij/mruby-io'
  # conf.gem :git => 'git@github.com:iij/mruby-io.git', :branch => 'master', :options => '-v'

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

#MRuby::Build.new('bench') do |conf|
#  # Gets set by the VS command prompts.
#  if ENV['VisualStudioVersion'] || ENV['VSINSTALLDIR']
#    toolchain :visualcpp
#  else
#    toolchain :gcc
#    conf.cc.flags << '-O3'
#  end
#
#  conf.gembox 'default'
#end

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
# end

# Cross Compiling configuration for RX630
# http://gadget.renesas.com/
#
# Requires gnurx_v14.03
MRuby::CrossBuild.new("RX630") do |conf|
  toolchain :gcc

  # Windows
  BIN_PATH = "/cygdrive/c/Renesas/GNURXv14.03-ELF/rx-elf/rx-elf/bin"
  LIB_PATH = "/cygdrive/c/Renesas/GNURXv14.03-ELF/rx-elf/rx-elf/rx-elf/lib"
  # Linux
  # BIN_PATH = "/usr/share/gnurx_v14.03_elf-1/bin"
  # macOS
  # BIN_PATH = "/Applications/IDE4GR.app/Contents/Java/hardware/tools/gcc-rx/rx-elf/rx-elf/bin/"

  conf.cc do |cc|
    cc.command = "#{BIN_PATH}/rx-elf-gcc"
    cc.flags = "-Wall -g -O2 -flto -mcpu=rx600 -m64bit-doubles -L#{LIB_PATH}/"
    cc.compile_options = "%{flags} -o %{outfile} -c %{infile}"

    #configuration for low memory environment
    cc.defines << %w(MRB_USE_FLOAT)           # add -DMRB_USE_FLOAT to use float instead of double for floating point numbers
    cc.defines << %w(MRB_FUNCALL_ARGC_MAX=6)  # argv max size in mrb_funcall
    cc.defines << %w(MRB_HEAP_PAGE_SIZE=24)   # number of object per heap page
    cc.defines << %w(MRB_USE_IV_SEGLIST)      # use segmented list for IV table
    cc.defines << %w(MRB_IVHASH_INIT_SIZE=3)  # initial size for IV khash; ignored when MRB_USE_IV_SEGLIST is set
    cc.defines << %w(KHASH_DEFAULT_SIZE=2)    # default size of khash table bucket
    cc.defines << %w(POOL_PAGE_SIZE=256)      # effective only for use with mruby-eval
    cc.defines << %w(MRB_BYTECODE_DECODE_OPTION)  # hooks for bytecode decoder
  end

  conf.cxx do |cxx|
    cxx.command = conf.cc.command.dup
    cxx.include_paths = conf.cc.include_paths.dup
    cxx.flags = conf.cc.flags.dup
    cxx.defines = conf.cc.defines.dup
    cxx.compile_options = conf.cc.compile_options.dup
  end

  conf.linker do |linker|
    linker.command="#{BIN_PATH}/rx-elf-ld"
  end

  conf.archiver do |archiver|
    archiver.command = "#{BIN_PATH}/rx-elf-ar"
    archiver.archive_options = 'rcs %{outfile} %{objs}'
  end

  #no executables
  conf.bins = []

  #do not build executable test
  conf.build_mrbtest_lib_only

  #disable C++ exception
  conf.disable_cxx_exception

  #gems from core
  #conf.gem :core => "mruby-array-ext"
  #conf.gem :core => "mruby-bin-debugger"
  #conf.gem :core => "mruby-bin-mirb"
  #conf.gem :core => "mruby-bin-mrbc"
  #conf.gem :core => "mruby-bin-mruby"
  #conf.gem :core => "mruby-bin-mruby-config"
  #conf.gem :core => "mruby-bin-strip"
  #conf.gem :core => "mruby-class-ext"
  #conf.gem :core => "mruby-compar-ext"
  #conf.gem :core => "mruby-compiler"
  #conf.gem :core => "mruby-enumerator"
  #conf.gem :core => "mruby-enum-ext"
  #conf.gem :core => "mruby-enum-lazy"
  #conf.gem :core => "mruby-error"
  #conf.gem :core => "mruby-eval"
  #conf.gem :core => "mruby-exit"
  #conf.gem :core => "mruby-fiber"
  #conf.gem :core => "mruby-hash-ext"
  #conf.gem :core => "mruby-inline-struct"
  #conf.gem :core => "mruby-io"
  #conf.gem :core => "mruby-kernel-ext"
  conf.gem :core => "mruby-math"
  #conf.gem :core => "mruby-method"
  conf.gem :core => "mruby-numeric-ext"
  #conf.gem :core => "mruby-object-ext"
  #conf.gem :core => "mruby-objectspace"
  #conf.gem :core => "mruby-print"
  #conf.gem :core => "mruby-proc-ext"
  #conf.gem :core => "mruby-random"
  #conf.gem :core => "mruby-range-ext"
  #conf.gem :core => "mruby-socket"
  #conf.gem :core => "mruby-sprintf"
  #conf.gem :core => "mruby-string-ext"
  #conf.gem :core => "mruby-struct"
  #conf.gem :core => "mruby-symbol-ext"
  #conf.gem :core => "mruby-test"
  #conf.gem :core => "mruby-time"
  #conf.gem :core => "mruby-toplevel-ext"

  #light-weight regular expression
  #conf.gem :github => "masamitsu-murase/mruby-hs-regexp", :branch => "master"
end
