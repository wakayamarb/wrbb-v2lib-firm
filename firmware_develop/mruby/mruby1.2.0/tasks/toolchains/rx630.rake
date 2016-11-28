MRuby::Toolchain.new(:rx630) do |conf, _params|
  toolchain :gcc

  TOOL_PATH = "/cygdrive/d/Renesas/GNURXv14.03-ELF/rx-elf/rx-elf/bin"
	
  conf.cc do |cc|
    cc.command="#{TOOL_PATH}/rx-elf-gcc"
    cc.flags << "-Wall -g -O2 -flto -mcpu=rx600 -m64bit-doubles"
		cc.defines << "__CYGWIN32__"
    cc.compile_options = "%{flags} -o %{outfile} -c %{infile}"

    conf.linker do |linker|
      linker.command="#{TOOL_PATH}/rx-elf-ld"
    end
    conf.archiver do |archiver|
      archiver.command = "#{TOOL_PATH}/rx-elf-ar"
      archiver.archive_options = 'rcs %{outfile} %{objs}'
    end
  end

  conf.bins = []
end

