#include <cstdint>
#include <cstdio>
#include <signal.h>
#include <util/CommandOptionParser.h>
#include <thread>
#include "Configuration.h"
#include <Aeron.h>
#include "ShimPublisher.h"
#include <array>

using namespace aeron::util;
using namespace aeron;

std::atomic<bool> running (true);

struct Settings
{
    std::string dirPrefix = "";
    int lingerTimeoutMs = 0;
};

typedef std::array<std::uint8_t, 256> buffer_t;

int publish ( char* channel, int streamId, char* message )
{
    Settings settings;

    try
    {
        std::cout << "Publishing to channel " << channel << " on Stream ID " << streamId << std::endl;

        aeron::Context context;

        if (settings.dirPrefix != "")
        {
            context.aeronDir(settings.dirPrefix);
        }

        context.newPublicationHandler(
            [](const std::string& channel, std::int32_t streamId, std::int32_t sessionId, std::int64_t correlationId)
            {
                std::cout << "Publication: " << channel << " " << correlationId << ":" << streamId << ":" << sessionId << std::endl;
            });

        std::shared_ptr<Aeron> aeron = Aeron::connect(context);

        // add the publication to start the process
        std::int64_t id = aeron->addPublication(channel, streamId);

        std::shared_ptr<Publication> publication = aeron->findPublication(id);
        // wait for the publication to be valid
        while (!publication)
        {
            std::this_thread::yield();
            publication = aeron->findPublication(id);
        }

        AERON_DECL_ALIGNED(buffer_t buffer, 16);
        concurrent::AtomicBuffer srcBuffer(&buffer[0], buffer.size());

        int sent = 0;
        int attempts = 0;

        while (0 == sent)
        {
            int messageLen = strlen(message);
            srcBuffer.putBytes(0, reinterpret_cast<std::uint8_t *>(message), messageLen);

            const std::int64_t result = publication->offer(srcBuffer, 0, messageLen);

            if (result < 0)
            {
                if (NOT_CONNECTED == result)
                {
                    std::cout << " not connected yet." << std::endl;
                }
                else if (BACK_PRESSURED == result)
                {
                    std::cout << " back pressured." << std::endl;
                }
                else
                {
                    std::cout << " ah?! unknown " << result << std::endl;
                }
            }
            else
            {
                std::cout << " yay! sent:" << message << std::endl;
                sent = 1;
            }

            attempts++;
            
            if (attempts > 5) 
            {
              break;
            } 

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "Done sending." << std::endl;

        if (settings.lingerTimeoutMs > 0)
        {
            std::cout << "Lingering for " << settings.lingerTimeoutMs << " milliseconds." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(settings.lingerTimeoutMs));
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
