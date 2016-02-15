require 'rubygems'
require 'JSON'
require 'ffi'


module AeronSubscriber 
  extend FFI::Library
  ffi_lib File.join(File.expand_path(__FILE__).split("/").tap do |path|
    path.pop
    path << "libsubscriber.so"
  end)
  attach_function :subscribe, [ ], :int, :blocking => true
  attach_function :unsubscribe, [ ], :void
  attach_function :poll, [ :strptr, :int ], :int
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

threads << Thread.new{AeronSubscriber.subscribe}

threads << Thread.new do
  loop do 
    messagePtr = FFI::MemoryPointer.new(:char, 1000) # need to free this memory
    ret = AeronSubscriber.poll(messagePtr, 1000)
    if ret > 0 then 
      message = messagePtr.get_string(0, 1000)
      result = handle_message(message) 
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
