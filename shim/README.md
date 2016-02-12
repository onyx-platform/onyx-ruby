# Ruby Shim

Basic POC to show messaging with Ruby Gem via an Aeron client through Ruby FFI

## Usage

Clone Aeron from [here](https://github.com/real-logic/Aeron). Build, then run the media driver

```
$ ./gradlew
java -cp aeron-samples/build/libs/samples.jar uk.co.real_logic.aeron.driver.MediaDriver
```

Build the Aeron subscriber library to be called by the shim script and the 
BasicPublisher client for testing. Then run the Ruby script. 
 
```
$ cd ext
$ make
$ ./BasicPublisher

$ cd ..
$ ruby lib/app.rb
```

The Ruby script will receive a message with an :onyx/fn and :onyx/params, then call that 
function and return a result.
