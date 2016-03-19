MRuby::Toolchain.new(:rx63n) do |conf|
  toolchain :gcc

  TOOL_PATH = "/usr/local/tkdn-20110720/rx-elf/bin"

  conf.cc do |cc|
    cc.command="#{TOOL_PATH}/rx-elf-gcc"
    cc.flags << "-Wall -g -O2"
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

