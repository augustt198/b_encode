require "b_encode"
require "benchmark"

TESTS = 1_000_000

Benchmark.bm(30) do |bm|
  bm.report("b_encode: encoding integers") do
    TESTS.times { |x| x.bencode }
  end
  bm.report("b_encode: encoding strings") do
    TESTS.times { |x| x.to_s.bencode }
  end
  bm.report("b_encode: encoding lists") do
    TESTS.times { |x| [x, x.to_s, [], {}].bencode }
  end
  bm.report("b_encode: encoding dicts") do
    TESTS.times { |x| {x.to_s => x}.bencode }
  end
  bm.report("b_encode: decoding integers") do
    TESTS.times { |x| BEncode.decode "i#{x}e" }
  end
  bm.report("b_encode: decoding strings") do
    TESTS.times do |x|
      s = x.to_s
      BEncode.decode "#{s.length}:#{s}"
    end
  end
  bm.report("b_encode: decoding lists") do
    TESTS.times do |x|
      BEncode.decode "li123e3:fooli10eed1:a1:bee"
    end
  end
  bm.report("b_encode: decoding hashes") do
    TESTS.times do |x|
      BEncode.decode "d3:fooi123e1:ali123ed3:abc3:defeee"
    end
  end 
end

# suppress warnings
$VERBOSE = nil

require "bencode"

Benchmark.bm(30) do |bm|
  bm.report("bencode: encoding integers") do
    TESTS.times { |x| x.bencode }
  end
  bm.report("bencode: encoding strings") do
    TESTS.times { |x| x.to_s.bencode }
  end
  bm.report("bencode: encoding lists") do
    TESTS.times { |x| [x, x.to_s, [], {}].bencode }
  end
  bm.report("bencode: encoding dicts") do
    TESTS.times { |x| {x.to_s => x}.bencode }
  end
  bm.report("bencode: decoding integers") do
    TESTS.times { |x| BEncode.load "i#{x}e" }
  end
  bm.report("bencode: decoding strings") do
    TESTS.times do |x|
      s = x.to_s
      BEncode.load "#{s.length}:#{s}"
    end
  end
  bm.report("bencode: decoding lists") do
    TESTS.times do |x|
      BEncode.load "li123e3:fooli10eed1:a1:bee"
    end
  end
  bm.report("bencode: decoding hashes") do
    TESTS.times do |x|
      BEncode.load "d3:fooi123e1:ali123ed3:abc3:defeee"
    end
  end 
end
