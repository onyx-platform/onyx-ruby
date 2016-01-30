require 'rubygems'
require 'sinatra'
require 'JSON'

def inc(n)
  return n + 1
end

post '/ruby-shim' do
  request.body.rewind
  b = request.body.read
  stuff = JSON.parse(b)
  fn = stuff[":onyx/fn"] 
  JSON.generate({ "result" => (send fn, 1) })
end
