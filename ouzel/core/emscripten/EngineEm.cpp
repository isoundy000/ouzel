// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <cstdlib>
#include <emscripten.h>
#include "EngineEm.hpp"
#include "audio/AudioDevice.hpp"
#include "graphics/RenderDevice.hpp"
#include "input/emscripten/InputEm.hpp"
#include "utils/Utils.hpp"

static void loop(void* arg)
{
    if (!reinterpret_cast<ouzel::EngineEm*>(arg)->step())
    {
        emscripten_cancel_main_loop();
    }
}

namespace ouzel
{
    EngineEm::EngineEm(int aArgc, char* aArgv[])
    {
        for (int i = 0; i < aArgc; ++i)
        {
            args.push_back(aArgv[i]);
        }
    }

    int EngineEm::run()
    {
        if (!init())
        {
            return EXIT_FAILURE;
        }

        start();

        emscripten_set_main_loop_arg(loop, this, 0, 1);

        return EXIT_SUCCESS;
    }

    bool EngineEm::step()
    {
        update();
        audio->update();

        if (!active ||
            !renderer->getDevice()->process())
        {
            return false;
        }

        // TODO: check for result of the AudioDevice::process
        audio->getDevice()->process();

        input::InputEm* inputEm = static_cast<input::InputEm*>(input.get());
        inputEm->update();

        return active;
    }

    void EngineEm::executeOnMainThread(const std::function<void(void)>& func)
    {
        if (func) func();
    }

    bool EngineEm::openURL(const std::string& url)
    {
        EM_ASM_ARGS({window.open(Pointer_stringify($0));}, url.c_str());

        return true;
    }
}
