#include "audio_main.hpp"

#include <comdef.h>
#include <string>

#include "audio_service.hpp"
#include "wasapi_client.hpp"

int audioMain(moodycamel::ReaderWriterQueue<std::string>* queue) {
    // initialize COM
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    try {
        WasapiClient wasapiClient;
        AudioService audioService{wasapiClient, queue};
        audioService.run();
    } catch(std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
