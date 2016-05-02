require 'rubygems'
require 'json'
require 'ffi'


module AeronSubscriber 
  extend FFI::Library
  ffi_lib File.join(File.expand_path(__FILE__).split("/").tap do |path|
    path.pop
    path << "libsubscriber.so"
  end)
  attach_function :subscribe, [ :string, :int ], :int, :blocking => true
  attach_function :unsubscribe, [ ], :void
  attach_function :poll, [ :strptr, :int ], :int
end

module AeronPublisher
  extend FFI::Library
  ffi_lib File.join(File.expand_path(__FILE__).split("/").tap do |path|
    path.pop
    path << "libpublisher.so"
  end)
  attach_function :publish, [ :string, :int, :string ], :int, :blocking => true
end

def inc(n)
  n + 1
end

def handle_message(message)
  # puts "Message: #{message}" 
  msg_parsed = JSON.parse(message)
  fn = msg_parsed[":onyx/fn"] 
  param = msg_parsed[":onyx/params"][0]
  JSON.generate({ "result" => (send fn, param) })
end

threads = []


Thread.abort_on_exception = true

threads << Thread.new { AeronSubscriber.subscribe("aeron:udp?remote=localhost:40123", 10) }

threads << Thread.new do
  loop do 
    # The FFI::MemoryPointer class allocates native memory with automatic garbage collection as a sweetener. 
    # When a MemoryPointer goes out of scope, the memory is freed up as part of the garbage collection process.
    messagePtr = FFI::MemoryPointer.new(:char, 1000) 
    ret = AeronSubscriber.poll(messagePtr, 1000)
    if ret > 0 then 
      message = messagePtr.get_string(0, 1000)
      result = handle_message(message) 
      ret = AeronPublisher.publish("aeron:udp?remote=localhost:40123", 11, result)
      puts result
    end
  end
end

trap("INT") { 
  puts "Exiting"
  AeronSubscriber.unsubscribe
  threads.each{|t|
    puts "killing"
    Thread.kill t
  }
}

threads.each { |t| t.join }
