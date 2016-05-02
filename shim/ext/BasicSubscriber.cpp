/*
 * Copyright 2014 - 2015 Real Logic Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdint>
#include <cstdio>
#include <string.h>
#include <signal.h>
#include <util/CommandOptionParser.h>
#include <thread>
#include <queue>
#include "Configuration.h"
#include "BasicSubscriber.h"
#include <Aeron.h>

using namespace aeron::util;
using namespace aeron;

std::atomic<bool> running (true);
std::queue<std::string> messagequeue;

static const std::chrono::duration<long, std::milli> IDLE_SLEEP_MS(1);
static const std::chrono::duration<long, std::milli> IDLE_SLEEP_MS_2(1000);
static const int FRAGMENTS_LIMIT = 10;

struct Settings
{
    std::string dirPrefix = "";
};

fragment_handler_t saveMessage()
{
    return [&](AtomicBuffer& buffer, util::index_t offset, util::index_t length, Header& header)
    {
        messagequeue.push(std::string((char *)buffer.buffer() + offset, (unsigned long)length));
    };
}

extern "C"
{
void unsubscribe()
{
  running = false;
}

int subscribe( char* channel, int streamId )
{

    try
    {
        Settings settings;

        std::cout << "Subscribing to channel " << channel << " on Stream ID " << streamId << std::endl;

        aeron::Context context;

        if (settings.dirPrefix != "")
        {
            context.aeronDir(settings.dirPrefix);
        }

        context.newSubscriptionHandler(
            [](const std::string& channel, std::int32_t streamId, std::int64_t correlationId)
            {
                std::cout << "Subscription: " << channel << " " << correlationId << ":" << streamId << std::endl;
            });

        context.availableImageHandler([](Image &image)
            {
                std::cout << "Available image correlationId=" << image.correlationId() << " sessionId=" << image.sessionId();
                std::cout << " at position=" << image.position() << " from " << image.sourceIdentity() << std::endl;
            });

        context.unavailableImageHandler([](Image &image)
            {
                std::cout << "Unavailable image on correlationId=" << image.correlationId() << " sessionId=" << image.sessionId();
                std::cout << " at position=" << image.position() << " from " << image.sourceIdentity() << std::endl;
            });

        std::shared_ptr<Aeron> aeron = Aeron::connect(context);

        // add the subscription to start the process
        std::int64_t id = aeron->addSubscription(channel, streamId);

        std::shared_ptr<Subscription> subscription = aeron->findSubscription(id);
        // wait for the subscription to be valid
        while (!subscription)
        {
            std::this_thread::yield();
            subscription = aeron->findSubscription(id);
        }

        fragment_handler_t handler = saveMessage();
        SleepingIdleStrategy idleStrategy(IDLE_SLEEP_MS);

        while (running)
        {
            const int fragmentsRead = subscription->poll(handler, FRAGMENTS_LIMIT);

            idleStrategy.idle(fragmentsRead);
            
            if (fragmentsRead > 0) { std::this_thread::sleep_for(IDLE_SLEEP_MS_2);}
        }

    }
    catch (CommandOptionException& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        return -1;
    }
    catch (SourcedException& e)
    {
        std::cerr << "FAILED: " << e.what() << " : " << e.where() << std::endl;
        return -1;
    }
    catch (std::exception& e)
    {
        std::cerr << "FAILED: " << e.what() << " : " << std::endl;
        return -1;
    }

    return 0;
}

int poll(char* message, int n)
{
  if (!messagequeue.empty())
  {
    std::string messageStr = messagequeue.front();
    messagequeue.pop();
    
    strncpy(message, messageStr.c_str(), n - 1);
    message[n - 1] = '\0';

    return 1;
  }
  
  return 0;
}
}
