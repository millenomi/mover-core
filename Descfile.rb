
# This file describes declaratively which pieces are there to build Mover Core. For now, it's written in a Ruby DSL whose API has not been coded yet, but it will someday.
# A 'part' is what it says — it's NOT an entire target, just a piece of a compilation job. A 'cc'-style part is a part that describes how to build something using a C/C++-style compiler such as GCC or Clang.

mover_core_4 = CCPart.part "net.infinite-labs.Mover.Core.4" do |part|
    part.sources FileList['*.cpp']
    part.preprocessor_definitions {
        Option.new("ILPlatformCoreSupportEntireUnicodeRange", { true => "1", false => "0" }, true)
    }
end

CCPart.part "net.infinite-labs.Mover.Core.4.Tests" do |part|
    part.inherits mover_core_4
    part.sources FileList['Tests/*.cpp']
end
