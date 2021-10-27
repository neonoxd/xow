/*
 * Copyright (C) 2019 Medusalix
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "utils/log.h"
#include "utils/reader.h"
#include "utils/sock.h"
#include "dongle/usb.h"
#include "dongle/dongle.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstring>
#include <csignal>
#include <sys/signalfd.h>

char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

int main(int argc, char * argv[])
{
    Log::init();
    Log::info("xow %s ©Severin v. W.", VERSION);
    if (argc > 1) 
    {
        char * customPortChr = getCmdOption(argv, argv + argc, "-p");
        Log::info("launched with args: %d", argc);
        Log::info("found param: [%s] polybar ipc enabled", argv[1]);
        if (customPortChr) {
            std::stringstream strValue;
            strValue << customPortChr;
            strValue >> Socks::custom_port;
            Log::info("with value %d", Socks::custom_port);
        }
        Socks::sockMode = true;
        Socks::createConnection();
        if (Socks::act_sock < 0)
            Log::error("Couldn't connect to socket. retrying with the next message.");
    }

    sigset_t signalMask;

    sigemptyset(&signalMask);
    sigaddset(&signalMask, SIGINT);
    sigaddset(&signalMask, SIGTERM);
    sigaddset(&signalMask, SIGUSR1);

    // Block signals for all USB threads
    if (pthread_sigmask(SIG_BLOCK, &signalMask, nullptr) < 0)
    {
        Log::error("Error blocking signals: %s", strerror(errno));

        return EXIT_FAILURE;
    }

    UsbDeviceManager manager;

    // Unblock signals for current thread to allow interruption
    if (pthread_sigmask(SIG_UNBLOCK, &signalMask, nullptr) < 0)
    {
        Log::error("Error unblocking signals: %s", strerror(errno));

        return EXIT_FAILURE;
    }

    // Bind USB device termination to signal reader interruption
    InterruptibleReader signalReader;
    UsbDevice::Terminate terminate = std::bind(
        &InterruptibleReader::interrupt,
        &signalReader
    );
    std::unique_ptr<UsbDevice> device = manager.getDevice({
        { DONGLE_VID, DONGLE_PID_OLD },
        { DONGLE_VID, DONGLE_PID_NEW },
        { DONGLE_VID, DONGLE_PID_SURFACE }
    }, terminate);

    // Block signals and pass them to the signalfd
    if (pthread_sigmask(SIG_BLOCK, &signalMask, nullptr) < 0)
    {
        Log::error("Error blocking signals: %s", strerror(errno));

        return EXIT_FAILURE;
    }

    int file = signalfd(-1, &signalMask, 0);

    if (file < 0)
    {
        Log::error("Error creating signal file: %s", strerror(errno));

        return EXIT_FAILURE;
    }

    signalReader.prepare(file);

    Dongle dongle(std::move(device));
    signalfd_siginfo info = {};

    while (signalReader.read(&info, sizeof(info)))
    {
        uint32_t type = info.ssi_signo;

        if (type == SIGINT || type == SIGTERM)
        {
            break;
        }

        if (type == SIGUSR1)
        {
            Log::debug("User signal received");
            std::string msgc = Socks::concat_string("PS|", std::to_string(1));
            Socks::sendMessage(msgc);
            dongle.setPairingStatus(true);
        }
    }

    Log::info("Shutting down...");

    return EXIT_SUCCESS;
}
