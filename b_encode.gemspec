# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'b_encode/version'

Gem::Specification.new do |spec|
  spec.name          = "b_encode"
  spec.version       = BEncode::VERSION
  spec.authors       = ["August"]
  spec.email         = ["augustt198@gmail.com"]
  spec.extensions    = ["ext/b_encode/extconf.rb"]
  spec.summary       = %q{Bencode encoder/decoder}
  spec.homepage      = "http://github.com/augustt198/b_encode"
  spec.license       = "MIT"

  spec.files         = `git ls-files -z`.split("\x0")
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", "~> 1.7"
  spec.add_development_dependency "rake", "~> 10.0"
  spec.add_development_dependency "rake-compiler"
end
