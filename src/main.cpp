
#include <chrono>
#include <thread>

#include "eplayer.h"

#include "elog/elog.h"

int main(int argc, char const *argv[])
{

    EPlayer player;

    try
    {
        player.setUrl("[VCB-Studio] Hina Logi ~From Luck & "
                      "Logic~ [01][Ma10p_1080p][x265_flac_aac].mkv");
        player.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    {
        using namespace std;
        while (true)
        {
            /* code */

            this_thread::sleep_for(chrono::milliseconds(10));
        }
    }
    return 0;
}
