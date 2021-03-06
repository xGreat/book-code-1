#include "pipe.h"

namespace cactus
{
pipe::pipe(std::wstring pipeName, std::wstring serveur) : hPipe(INVALID_HANDLE_VALUE), pipePath(L"\\\\")
{
    pipePath.append(serveur);
    pipePath.append(L"\\pipe\\");
    pipePath.append(pipeName);
}

pipe::~pipe(void)
{
    closePipe();
}

bool pipe::closePipe()
{
    bool reussite = false;

    if (hPipe != INVALID_HANDLE_VALUE && hPipe) {
        FlushFileBuffers(hPipe);
        DisconnectNamedPipe(hPipe);
        reussite = CloseHandle(hPipe) == TRUE;
    }

    return reussite;
}

bool pipe::readFromPipe(std::wstring &laReponse)
{
    bool reussite = false;
    wchar_t monBuffer[128];
    bool fSuccess;
    DWORD longueurReponse;
    laReponse.clear();

    do {
        fSuccess = ReadFile(hPipe, monBuffer, sizeof(monBuffer), &longueurReponse, NULL) ? true : false;

        if (reussite = (fSuccess || GetLastError() == ERROR_MORE_DATA)/* && longueurReponse != 0 */) {
            laReponse.append(monBuffer, longueurReponse / sizeof(wchar_t));
        } else {
            break;
        }
    } while (!fSuccess);

    return reussite;
}

bool pipe::writeToPipe(const std::wstring &leMessage)
{
    bool reussite = false;
    DWORD longueurMessage;
    DWORD longueurOctetsEcris;
    longueurMessage = (static_cast<DWORD>(leMessage.size())) * sizeof(wchar_t);

    if (WriteFile(hPipe, leMessage.c_str(), longueurMessage, &longueurOctetsEcris, NULL) && longueurMessage == longueurOctetsEcris) {
        reussite = FlushFileBuffers(hPipe) != 0;
    }

    return reussite;
}


bool pipe::createServer()
{
    bool reussite = false;

    if (!hPipe || hPipe == INVALID_HANDLE_VALUE) {
        hPipe = CreateNamedPipe(pipePath.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 0, 0, 30000, NULL);

        if (hPipe && hPipe != INVALID_HANDLE_VALUE) {
            reussite = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        } else {
            closePipe();
        }
    }

    return reussite;
}

bool pipe::createClient()
{
    bool reussite = false;

    if (!hPipe || hPipe == INVALID_HANDLE_VALUE) {
        if (WaitNamedPipe(pipePath.c_str(), NMPWAIT_USE_DEFAULT_WAIT)) {
            hPipe = CreateFile(pipePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

            if (hPipe != INVALID_HANDLE_VALUE) {
                DWORD dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;

                if (!(reussite = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL) != 0)) {
                    closePipe();
                }
            }
        }
    }

    return reussite;
}

bool pipe::isConnected()
{
    return (hPipe && hPipe != INVALID_HANDLE_VALUE);
}
}