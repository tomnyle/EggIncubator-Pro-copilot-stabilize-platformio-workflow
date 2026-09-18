#include <cassert>
#include <fstream>
#include <iterator>
#include <string>

int main()
{
    std::ifstream configFile(std::string(PROJECT_DIR) + "/platformio.ini");
    assert(configFile && "platformio.ini must be readable from the project root");

    const std::string text((std::istreambuf_iterator<char>(configFile)),
                           std::istreambuf_iterator<char>());

    assert(text.find("[env:esp32dev]") != std::string::npos);
    assert(text.find("[env:esp32dev_debug]") != std::string::npos);
    assert(text.find("platform = espressif32@7.0.1") != std::string::npos);
    assert(text.find("board = esp32dev") != std::string::npos);
    assert(text.find("framework = arduino") != std::string::npos);
    assert(text.find("monitor_speed = 115200") != std::string::npos);
    assert(text.find("upload_protocol = esptool") != std::string::npos);
    assert(text.find("upload_speed = 460800") != std::string::npos);

    const std::size_t releaseStart = text.find("[env:esp32dev]");
    const std::size_t debugStart = text.find("[env:esp32dev_debug]");
    const std::size_t nativeStart = text.find("[env:native]");

    assert(releaseStart != std::string::npos);
    assert(debugStart != std::string::npos);
    assert(nativeStart != std::string::npos);
    assert(releaseStart < debugStart);
    assert(debugStart < nativeStart);

    const std::string releaseBlock = text.substr(releaseStart, debugStart - releaseStart);
    const std::string debugBlock = text.substr(debugStart, nativeStart - debugStart);

    assert(releaseBlock.find("-DDEBUG") == std::string::npos);
    assert(debugBlock.find("-DDEBUG") == std::string::npos);

    assert(releaseBlock.find("monitor_filters =") != std::string::npos);
    assert(releaseBlock.find("time") != std::string::npos);
    assert(releaseBlock.find("esp32_exception_decoder") == std::string::npos);

    assert(debugBlock.find("monitor_filters =") != std::string::npos);
    assert(debugBlock.find("time") != std::string::npos);
    assert(debugBlock.find("esp32_exception_decoder") != std::string::npos);

    return 0;
}
