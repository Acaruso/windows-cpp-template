#pragma once

#include "../lib/readerwriterqueue.h"

int audioMain(moodycamel::ReaderWriterQueue<std::string>* queue);
