# BEncode

A bencode encoder/decoder.

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'b_encode'
```

Or install it yourself:

```sh
$ gem install b_encode
```

## Usage

#### Encoding

```ruby
"foo".bencode # => "3:foo"
123.bencode   # => "i123e"

["foo", 123].bencode   # => "l3:fooi123ee"
{"foo" => 123}.bencode # => "d3:fooi123ee"
```
