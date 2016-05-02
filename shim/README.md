# Ruby Shim

Basic POC to show messaging with Ruby Gem via an Aeron client through Ruby FFI

## Usage

Clone Aeron from [here](https://github.com/real-logic/Aeron/tree/0.9.2). This uses version 0.9.2 of the Aeron client. (Upgrading would involve swapping out files in the `ext` directory.) Build, then run the media driver

```
$ ./gradlew
java -cp aeron-samples/build/libs/samples.jar uk.co.real_logic.aeron.driver.MediaDriver
```

Build the Aeron subscriber library to be called by the shim script and the BasicPublisher and BasicSubscriber clients for testing. The BasicPublisher publishes a message on stream 10, which is subscribed to in the Ruby shim through an Aeron subscriber library. The BasicSubscriber subscribes to stream 11, to which the Ruby shim publishes the results of running the Ruby function with an Aeron publisher library. 

Run each in its own terminal so you can see the output of each. Then run the Ruby script. 

 
```
$ cd ext
$ make
$ ./BasicPublisher 
```

```
$ cd ext
$ ./BasicSubscriber -s 11
```

```
$ bundle install
$ ruby lib/app.rb
```

The Ruby script will receive a message with an :onyx/fn and :onyx/params, then call that function and return a result. The function has to be defined in `app.rb`. The example uses the function `inc`.

