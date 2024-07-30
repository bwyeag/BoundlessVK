#ifndef _BOUNDLESS_RENDER_FILE_
#define _BOUNDLESS_RENDER_FILE_
#include <cstdint>
#include "init.hpp"
#include "types.hpp"
namespace BL {
const uint32_t DEFAULT_MAX_FLIGHT_COUNT = 3;
struct RenderContext {
    commandPool cmdPool;
    uint32_t maxFlightCount;
    uint32_t curFrame;
    std::vector<commandBuffer> cmdBufs;
    std::vector<fence> fences;
    std::vector<semaphore> semsImageAvaliable;
    std::vector<semaphore> semsRenderFinish;

    RenderContext(uint32_t c);
    ~RenderContext() = default;
};
void initRenderLoop();
void terminateRenderLoop();
void renderLoop();

class RenderPassPack {
public:
    renderPass renderPass;
    std::vector<framebuffer> framebuffers;
    int callback_c_id,callback_d_id;
    RenderPassPack();
    ~RenderPassPack();
};
}  // namespace BL
#endif  //!_BOUNDLESS_RENDER_FILE_