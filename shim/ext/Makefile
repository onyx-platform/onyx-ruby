CXX_FLAGS = -Wall -std=c++11 -fexceptions -g -m64 -Wsign-compare -O3 -DNDEBUG -Ofast -DDISABLE_BOUNDS_CHECKS -fPIC -c 

default: install

install: shared-subscriber shared-publisher BasicPublisher BasicSubscriber
	mv libsubscriber.so ../lib
	mv libpublisher.so ../lib

ShimSubscriber.o: ShimSubscriber.cpp libaeron_client.a
	g++ $(CXX_FLAGS) ShimSubscriber.cpp -I.

shared-subscriber: ShimSubscriber.o
	g++ -Wall -std=c++11 -fexceptions -g -m64 -Wsign-compare -O3 -DNDEBUG -Ofast -DDISABLE_BOUNDS_CHECKS -shared -Wl,-headerpad_max_install_names  -o libsubscriber.so ShimSubscriber.o libaeron_client.a

ShimPublisher.o: ShimPublisher.cpp libaeron_client.a
	g++ $(CXX_FLAGS) ShimPublisher.cpp -I.

shared-publisher: ShimPublisher.o
	g++ -Wall -std=c++11 -fexceptions -g -m64 -Wsign-compare -O3 -DNDEBUG -Ofast -DDISABLE_BOUNDS_CHECKS -shared -Wl,-headerpad_max_install_names  -o libpublisher.so ShimPublisher.o libaeron_client.a

BasicPublisher.o: BasicPublisher.cpp libaeron_client.a
	g++ $(CXX_FLAGS) BasicPublisher.cpp -I.

BasicPublisher: BasicPublisher.o
	g++ -Wall -std=c++11 -fexceptions -g -m64 -O3 -DNDEBUG -Ofast -DDISABLE_BOUNDS_CHECKS BasicPublisher.o -o BasicPublisher -rdynamic libaeron_client.a -lpthread	

BasicSubscriber.o: BasicSubscriber.cpp libaeron_client.a
	g++ $(CXX_FLAGS) BasicSubscriber.cpp -I.

BasicSubscriber: BasicSubscriber.o
	g++ -Wall -std=c++11 -fexceptions -g -m64 -O3 -DNDEBUG -Ofast -DDISABLE_BOUNDS_CHECKS BasicSubscriber.o -o BasicSubscriber -rdynamic libaeron_client.a -lpthread	

Aeron.o: Aeron.cpp
	g++ $(CXX_FLAGS) Aeron.cpp -I.

LogBuffers.o: LogBuffers.cpp MemoryMappedFile.o
	g++ $(CXX_FLAGS) LogBuffers.cpp -I.

ClientConductor.o: ClientConductor.cpp LogBuffers.o
	g++ $(CXX_FLAGS) ClientConductor.cpp -I. 

Publication.o: Publication.cpp ClientConductor.o
	g++ $(CXX_FLAGS) Publication.cpp -I. 

Subscription.o: Subscription.cpp
	g++ $(CXX_FLAGS) Subscription.cpp -I.

MemoryMappedFile.o: util/MemoryMappedFile.cpp
	g++ $(CXX_FLAGS) util/MemoryMappedFile.cpp 

CommandOption.o: util/CommandOption.cpp
	g++ $(CXX_FLAGS) util/CommandOption.cpp -I.

CommandOptionParser.o: util/CommandOptionParser.cpp
	g++ $(CXX_FLAGS) util/CommandOptionParser.cpp -I.

StringUtil.o: util/StringUtil.cpp
	g++ $(CXX_FLAGS) util/StringUtil.cpp -I.

libaeron_client.a: Publication.o Subscription.o ClientConductor.o Aeron.o LogBuffers.o MemoryMappedFile.o CommandOption.o CommandOptionParser.o StringUtil.o
	ar rcs libaeron_client.a Publication.o Subscription.o ClientConductor.o Aeron.o LogBuffers.o MemoryMappedFile.o CommandOption.o CommandOptionParser.o StringUtil.o

clean:
	rm *.o ../lib/*.so *.a BasicPublisher BasicSubscriber
